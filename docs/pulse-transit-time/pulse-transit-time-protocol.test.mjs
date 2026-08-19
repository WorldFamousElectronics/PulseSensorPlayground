import assert from 'node:assert/strict';
import { parsePulseTransitTimeLine } from './pulse-transit-time-protocol.mjs';

assert.deepEqual(parsePulseTransitTimeLine('PTT1,123456,520,618'), {
  source: 'pulse-transit-time',
  format: 'Pulse Transit Time PTT1',
  mode: 'LIVE',
  timestampUs: 123456,
  timestampMs: 123.456,
  proximalSignal: 520,
  distalSignal: 618,
});
assert.equal(parsePulseTransitTimeLine('512'), null, 'one-sensor streams belong to Signal Coach');
assert.equal(parsePulseTransitTimeLine('S512'), null, 'Signal Coach prefixes must not enter the PTT lane');
assert.throws(() => parsePulseTransitTimeLine('PTT1,123,500'), /field count/);
assert.throws(() => parsePulseTransitTimeLine('PTT1,12.5,500,600'), /timestamp/);
assert.throws(() => parsePulseTransitTimeLine('PTT1,,500,600'), /timestamp/);
assert.throws(() => parsePulseTransitTimeLine('PTT1,0x10,500,600'), /timestamp/);
assert.throws(() => parsePulseTransitTimeLine('PTT1,1e3,500,600'), /timestamp/);
assert.throws(() => parsePulseTransitTimeLine('PTT1,+123,500,600'), /timestamp/);
assert.throws(() => parsePulseTransitTimeLine('PTT1,-1,500,600'), /timestamp/);
assert.throws(() => parsePulseTransitTimeLine('PTT1,001,500,600'), /timestamp/);
assert.throws(() => parsePulseTransitTimeLine('PTT1, 123,500,600'), /timestamp/);
assert.throws(() => parsePulseTransitTimeLine('PTT1,123,,600'), /proximal signal/);
assert.throws(() => parsePulseTransitTimeLine('PTT1,123,0x10,600'), /proximal signal/);
assert.throws(() => parsePulseTransitTimeLine('PTT1,123,+500,600'), /proximal signal/);
assert.throws(() => parsePulseTransitTimeLine('PTT1,123,0500,600'), /proximal signal/);
assert.throws(() => parsePulseTransitTimeLine('PTT1,123,500,1e3'), /distal signal/);
assert.throws(() => parsePulseTransitTimeLine('PTT1,123,500,-1'), /distal signal/);
assert.throws(() => parsePulseTransitTimeLine('PTT1,123,500, 600'), /distal signal/);
assert.throws(() => parsePulseTransitTimeLine('PTT1,123,70000,600'), /proximal signal/);
assert.throws(() => parsePulseTransitTimeLine('PTT1,123,500,70000'), /distal signal/);

console.log('Pulse Transit Time protocol tests passed');
