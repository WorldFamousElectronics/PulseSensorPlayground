// Pulse-wave transport adapters. Signal Coach itself runs in Chrome; sender
// BPM, IBI, quality, and state values are retained only as optional diagnostics.
export const PSWS_PREFIX = 'PSWS';
export const PSWS_VERSIONS = Object.freeze([1, 2]);

function integer(value, name, minimum, maximum) {
  if (!Number.isInteger(value) || value < minimum || value > maximum) {
    throw new Error(`invalid ${name}`);
  }
  return value;
}

function optionalInteger(value, name, minimum, maximum) {
  if (value === undefined || value === null || value === '') return null;
  return integer(Number(value), name, minimum, maximum);
}

function pulseFrame({ source, format, signal, timestampMs = null, sequence = null,
  mode = 'LIVE', sourceBpm = null, sourceIbi = null, sourceBeat = null,
  sourceQuality = null, sourceState = null, version = 0, ...extra }) {
  return {
    source,
    format,
    version,
    mode,
    sequence,
    timestampMs,
    signal: integer(Number(signal), 'signal', 0, 65535),
    sourceBpm: optionalInteger(sourceBpm, 'source BPM', 0, 240),
    sourceIbi: optionalInteger(sourceIbi, 'source IBI', 0, 10000),
    sourceBeat: sourceBeat === null ? null : integer(Number(sourceBeat), 'source beat', 0, 1),
    sourceQuality: optionalInteger(sourceQuality, 'source quality', 0, 100),
    sourceState,
    ...extra,
  };
}

function parseJsonLine(normalized) {
  if (!normalized.startsWith('{')) return null;
  let value;
  try { value = JSON.parse(normalized); } catch { throw new Error('invalid JSON pulse frame'); }
  const signal = value.signal ?? value.sample ?? value.raw ?? value.value;
  if (signal === undefined) return null;
  return pulseFrame({
    source: 'json',
    format: 'JSON pulse sample',
    signal,
    timestampMs: optionalInteger(value.timestampMs ?? value.timestamp ?? value.time, 'timestamp', 0, Number.MAX_SAFE_INTEGER),
    sequence: optionalInteger(value.sequence ?? value.seq, 'sequence', 0, 0x7fffffff),
    sourceBpm: value.bpm,
    sourceIbi: value.ibi ?? value.ibiMs,
    sourceBeat: value.beat,
    sourceQuality: value.quality,
    sourceState: typeof value.state === 'string' ? value.state : null,
  });
}

export function parsePulseLine(line) {
  const normalized = line.trim();
  if (!normalized) return null;

  const json = parseJsonLine(normalized);
  if (json) return json;

  const fields = normalized.split(',').map((field) => field.trim());
  if (fields[0] === 'PTT1') {
    if (fields.length !== 4) throw new Error('invalid PTT1 field count');
    const timestampUs = integer(Number(fields[1]), 'timestamp', 0, 0xffffffff);
    const frame = pulseFrame({
      source: 'ptt1',
      format: 'PulseSensor dual-channel PTT1',
      signal: fields[2],
      timestampMs: timestampUs / 1000,
    });
    frame.timestampUs = timestampUs;
    frame.secondarySignal = integer(Number(fields[3]), 'secondary signal', 0, 65535);
    return frame;
  }
  if (fields[0] === PSWS_PREFIX) {
    const version = integer(Number(fields[1]), 'protocol version', 1, 255);
    if (!PSWS_VERSIONS.includes(version)) throw new Error(`unsupported PSWS version ${version}`);
    const expectedFields = version === 1 ? 11 : 18;
    if (fields.length !== expectedFields) throw new Error('invalid PSWS field count');
    const mode = fields[2];
    if (mode !== 'LIVE' && mode !== 'SIM') throw new Error('invalid PSWS mode');
    const sourceState = fields[10];
    if (!/^[A-Z0-9_]+$/.test(sourceState)) throw new Error('invalid PSWS state');
    const frame = pulseFrame({
      source: 'pulselink',
      format: `PulseLink PSWS v${version}`,
      version,
      mode,
      sequence: integer(Number(fields[3]), 'sequence', 0, 0x7fffffff),
      timestampMs: integer(Number(fields[4]), 'timestamp', 0, 0x3fffffff),
      signal: fields[5],
      sourceBpm: fields[6],
      sourceIbi: fields[7],
      sourceBeat: fields[8],
      sourceQuality: fields[9],
      sourceState,
    });
    if (version === 2) {
      Object.assign(frame, {
        sourceThreshold: integer(Number(fields[11]), 'source threshold', -1024, 2047),
        sourceSignalMin: integer(Number(fields[12]), 'source signal minimum', -1024, 2047),
        sourceSignalMax: integer(Number(fields[13]), 'source signal maximum', -1024, 2047),
        battery: integer(Number(fields[14]), 'battery', -1, 100),
        charging: Boolean(integer(Number(fields[15]), 'charging', 0, 1)),
        linked: Boolean(integer(Number(fields[16]), 'linked', 0, 1)),
        sourceLocked: Boolean(integer(Number(fields[17]), 'source locked', 0, 1)),
      });
    }
    return frame;
  }

  if (fields.length === 4 && fields.every((field) => field !== '' && Number.isFinite(Number(field)))) {
    return pulseFrame({
      source: 'arduino-csv',
      format: 'Arduino signal,bpm,ibi,beat CSV',
      signal: fields[0],
      sourceBpm: fields[1],
      sourceIbi: fields[2],
      sourceBeat: fields[3],
    });
  }

  if (fields.length === 2 && fields.every((field) => field !== '' && Number.isFinite(Number(field)))) {
    return pulseFrame({
      source: 'timestamped-csv',
      format: 'timestamp,signal CSV',
      timestampMs: integer(Number(fields[0]), 'timestamp', 0, Number.MAX_SAFE_INTEGER),
      signal: fields[1],
    });
  }

  if (/^[-+]?\d+$/.test(normalized)) {
    return pulseFrame({ source: 'raw-number', format: 'raw numeric samples', signal: normalized });
  }

  const prefixed = normalized.match(/^[Ss]\s*[:=]?\s*(\d+)$/);
  if (prefixed) {
    return pulseFrame({ source: 'pulse-serial', format: 'PulseSensor S-value samples', signal: prefixed[1] });
  }

  const labeled = normalized.match(/^(?:signal|sig|sample|raw)(?:\s*[:=]\s*|\s+)(\d+)$/i);
  if (labeled) {
    return pulseFrame({ source: 'labeled', format: 'labeled pulse samples', signal: labeled[1] });
  }

  // PulseSensor Processing streams can interleave B (BPM) and Q (IBI) lines.
  // Chrome computes both itself, so non-signal records are intentionally ignored.
  if (/^[BbQq]\s*[:=]?\s*\d+$/.test(normalized)) return null;
  return null;
}
