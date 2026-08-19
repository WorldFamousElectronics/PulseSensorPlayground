function integer(value, name, minimum, maximum) {
  if (!Number.isInteger(value) || value < minimum || value > maximum) {
    throw new Error(`invalid ${name}`);
  }
  return value;
}

function unsignedDecimal(field, name, maximum) {
  if (!/^(?:0|[1-9]\d*)$/.test(field)) throw new Error(`invalid ${name}`);
  return integer(Number(field), name, 0, maximum);
}

export function parsePulseTransitTimeLine(line) {
  const fields = line.trim().split(',');
  if (fields[0] !== 'PTT1') return null;
  if (fields.length !== 4) throw new Error('invalid PTT1 field count');
  const timestampUs = unsignedDecimal(fields[1], 'timestamp', 0xffffffff);
  return {
    source: 'pulse-transit-time',
    format: 'Pulse Transit Time PTT1',
    mode: 'LIVE',
    timestampUs,
    timestampMs: timestampUs / 1000,
    proximalSignal: unsignedDecimal(fields[2], 'proximal signal', 65535),
    distalSignal: unsignedDecimal(fields[3], 'distal signal', 65535),
  };
}
