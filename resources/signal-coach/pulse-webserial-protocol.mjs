// Protocol parser shared by the Signal Coach transport and deterministic tests.
export const PSWS_PREFIX = 'PSWS';
export const PSWS_VERSIONS = Object.freeze([1, 2]);

function integer(value, name, minimum, maximum) {
  if (!Number.isInteger(value) || value < minimum || value > maximum) {
    throw new Error(`invalid ${name}`);
  }
  return value;
}

export function parsePulseLine(line) {
  const normalized = line.trim();
  if (!normalized) return null;
  const fields = normalized.split(',');

  if (fields[0] === PSWS_PREFIX) {
    const version = integer(Number(fields[1]), 'protocol version', 1, 255);
    if (!PSWS_VERSIONS.includes(version)) throw new Error(`unsupported PSWS version ${version}`);
    const expectedFields = version === 1 ? 11 : 18;
    if (fields.length !== expectedFields) throw new Error('invalid PSWS field count');
    const mode = fields[2];
    if (mode !== 'LIVE' && mode !== 'SIM') throw new Error('invalid PSWS mode');
    const state = fields[10];
    if (!/^[A-Z0-9_]+$/.test(state)) throw new Error('invalid PSWS state');
    const frame = {
      source: 'sticks3',
      version,
      mode,
      sequence: integer(Number(fields[3]), 'sequence', 0, 0x7fffffff),
      timestampMs: integer(Number(fields[4]), 'timestamp', 0, 0x3fffffff),
      signal: integer(Number(fields[5]), 'signal', 0, 4095),
      bpm: integer(Number(fields[6]), 'BPM', 0, 240),
      ibi: integer(Number(fields[7]), 'IBI', 0, 10000),
      beat: integer(Number(fields[8]), 'beat', 0, 1),
      quality: integer(Number(fields[9]), 'quality', 0, 12),
      state,
    };
    if (version === 2) {
      Object.assign(frame, {
        threshold: integer(Number(fields[11]), 'threshold', -1024, 2047),
        signalMin: integer(Number(fields[12]), 'signal minimum', -1024, 2047),
        signalMax: integer(Number(fields[13]), 'signal maximum', -1024, 2047),
        battery: integer(Number(fields[14]), 'battery', -1, 100),
        charging: Boolean(integer(Number(fields[15]), 'charging', 0, 1)),
        linked: Boolean(integer(Number(fields[16]), 'linked', 0, 1)),
        locked: Boolean(integer(Number(fields[17]), 'locked', 0, 1)),
      });
    }
    return frame;
  }

  // Backward compatibility for the published Arduino/ESP32 tutorial.
  if (fields.length !== 4) return null;
  const numbers = fields.map(Number);
  if (!numbers.every(Number.isFinite)) return null;
  return {
    source: 'playground',
    version: 0,
    mode: 'LIVE',
    sequence: null,
    timestampMs: null,
    signal: integer(numbers[0], 'signal', 0, 4095),
    bpm: integer(numbers[1], 'BPM', 0, 240),
    ibi: integer(numbers[2], 'IBI', 0, 10000),
    beat: integer(numbers[3], 'beat', 0, 1),
    quality: null,
    state: numbers[1] > 0 ? 'BEAT_TRACKING' : 'ACQUIRING',
  };
}
