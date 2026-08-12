// Signal Coach protocol compatibility tests.
import assert from 'node:assert/strict';
import { parsePulseLine } from './pulse-webserial-protocol.mjs';

assert.deepEqual(parsePulseLine('1847,72,833,1'), {
  source: 'playground', version: 0, mode: 'LIVE', sequence: null,
  timestampMs: null, signal: 1847, bpm: 72, ibi: 833, beat: 1,
  quality: null, state: 'BEAT_TRACKING',
});

assert.deepEqual(parsePulseLine('PSWS,1,SIM,42,12345,2840,72,833,1,12,QUALIFIED'), {
  source: 'sticks3', version: 1, mode: 'SIM', sequence: 42,
  timestampMs: 12345, signal: 2840, bpm: 72, ibi: 833, beat: 1,
  quality: 12, state: 'QUALIFIED',
});

assert.deepEqual(parsePulseLine('PSWS,2,LIVE,42,12345,2840,72,833,1,12,QUALIFIED,550,470,760,86,1,0,1'), {
  source: 'sticks3', version: 2, mode: 'LIVE', sequence: 42,
  timestampMs: 12345, signal: 2840, bpm: 72, ibi: 833, beat: 1,
  quality: 12, state: 'QUALIFIED', threshold: 550, signalMin: 470,
  signalMax: 760, battery: 86, charging: true, linked: false, locked: true,
});

assert.equal(parsePulseLine('sig=500 amp=20 bpm=0'), null);
assert.throws(() => parsePulseLine('PSWS,1,SIM,42,12345,5000,72,833,1,12,QUALIFIED'), /signal/);
assert.throws(() => parsePulseLine('PSWS,1,UNKNOWN,42,12345,2048,72,833,1,12,QUALIFIED'), /mode/);
assert.throws(() => parsePulseLine('PSWS,3,LIVE,42,12345,2048,72,833,1,12,QUALIFIED'), /unsupported/);

console.log('PulseSensor Web Serial protocol tests passed');
