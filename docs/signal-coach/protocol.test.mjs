import assert from 'node:assert/strict';
import { parsePulseLine } from './pulse-webserial-protocol.mjs';

assert.deepEqual(parsePulseLine('1847,72,833,1'), {
  source: 'arduino-csv', format: 'Arduino signal,bpm,ibi,beat CSV', version: 0,
  mode: 'LIVE', sequence: null, timestampMs: null, signal: 1847,
  sourceBpm: 72, sourceIbi: 833, sourceBeat: 1, sourceQuality: null,
  sourceState: null,
});

assert.deepEqual(parsePulseLine('512'), {
  source: 'raw-number', format: 'raw numeric samples', version: 0,
  mode: 'LIVE', sequence: null, timestampMs: null, signal: 512,
  sourceBpm: null, sourceIbi: null, sourceBeat: null, sourceQuality: null,
  sourceState: null,
});

assert.equal(parsePulseLine('S:2048').signal, 2048);
assert.equal(parsePulseLine('signal=612').signal, 612);
assert.equal(parsePulseLine('Signal 613').signal, 613);
assert.equal(parsePulseLine('1200,2048').timestampMs, 1200);
assert.equal(parsePulseLine('1200,2048').signal, 2048);
assert.equal(parsePulseLine('{"timestampMs":1200,"raw":2048,"bpm":70}').sourceBpm, 70);
assert.equal(parsePulseLine('B72'), null);
assert.equal(parsePulseLine('Q833'), null);

const ptt = parsePulseLine('PTT1,123456,520,618');
assert.equal(ptt.source, 'ptt1');
assert.equal(ptt.timestampUs, 123456);
assert.equal(ptt.timestampMs, 123.456);
assert.equal(ptt.signal, 520);
assert.equal(ptt.secondarySignal, 618);

const v1 = parsePulseLine('PSWS,1,SIM,42,12345,2840,72,833,1,12,QUALIFIED');
assert.equal(v1.source, 'pulselink');
assert.equal(v1.format, 'PulseLink PSWS v1');
assert.equal(v1.signal, 2840);
assert.equal(v1.sourceBpm, 72);
assert.equal(v1.sourceState, 'QUALIFIED');

const v2 = parsePulseLine('PSWS,2,LIVE,42,12345,2840,72,833,1,12,QUALIFIED,550,470,760,86,1,0,1');
assert.equal(v2.source, 'pulselink');
assert.equal(v2.format, 'PulseLink PSWS v2');
assert.equal(v2.sourceThreshold, 550);
assert.equal(v2.battery, 86);
assert.equal(v2.sourceLocked, true);

assert.equal(parsePulseLine('sig=500 amp=20 bpm=0'), null);
assert.throws(() => parsePulseLine('70000'), /invalid signal/);
assert.throws(() => parsePulseLine('PSWS,1,SIM,42,12345,70000,72,833,1,12,QUALIFIED'), /signal/);
assert.throws(() => parsePulseLine('PSWS,1,UNKNOWN,42,12345,2048,72,833,1,12,QUALIFIED'), /mode/);
assert.throws(() => parsePulseLine('PSWS,3,LIVE,42,12345,2048,72,833,1,12,QUALIFIED'), /unsupported/);
assert.throws(() => parsePulseLine('PTT1,123,500'), /field count/);
assert.throws(() => parsePulseLine('PTT1,123,500,70000'), /secondary signal/);

console.log('PulseSensor pulse-stream adapter tests passed');
