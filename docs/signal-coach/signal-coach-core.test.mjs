import assert from 'node:assert/strict';
import { BrowserSignalCoach, adviceFor, stateColor } from './signal-coach-core.mjs';

function pulseSample(timestamp, baseline = 512, scale = 1) {
  const phase = timestamp % 833;
  let pulse = 0;
  if (phase < 55) pulse = phase * 5;
  else if (phase < 145) pulse = 275 - (phase - 55) * 2.4;
  else if (phase < 205) pulse = 59 - (phase - 145) * .9;
  else if (phase < 245) pulse = (phase - 205) * 1.1;
  else if (phase < 315) pulse = 44 - (phase - 245) * .6;
  return Math.round((baseline + Math.max(0, pulse)) * scale);
}

const flat = new BrowserSignalCoach();
let result;
for (let index = 0; index < 200; index += 1) {
  result = flat.update(512 + (index % 3), index * 20);
}
assert.equal(result.state, 'NO_SIGNAL');
assert.equal(result.locked, false);

const untrustedSender = new BrowserSignalCoach();
for (let index = 0; index < 200; index += 1) {
  result = untrustedSender.update({ signal: 512, bpm: 72, ibi: 833, beat: 1, quality: 12 }, index * 20);
}
assert.equal(result.state, 'NO_SIGNAL');
assert.equal(result.bpm, 0, 'sender metrics must not qualify a flat waveform');

for (const rate of [50, 100, 200, 500]) {
  const arduino = new BrowserSignalCoach();
  const step = 1000 / rate;
  for (let timestamp = 0; timestamp < 12000; timestamp += step) {
    result = arduino.update(pulseSample(timestamp), timestamp);
  }
  assert.equal(result.state, 'QUALIFIED', `${rate} samples/s should qualify`);
  assert.equal(result.locked, true);
  assert.ok(result.quality >= 10);
  assert.ok(result.bpm >= 70 && result.bpm <= 74);
  assert.ok(result.ibi >= 800 && result.ibi <= 860);
}

const esp32 = new BrowserSignalCoach();
for (let timestamp = 0; timestamp < 12000; timestamp += 20) {
  result = esp32.update(pulseSample(timestamp, 512, 4), timestamp);
}
assert.equal(result.state, 'QUALIFIED');
assert.ok(result.bpm >= 70 && result.bpm <= 74);

result = esp32.resync(12020);
assert.equal(result.state, 'RESYNC');
assert.equal(result.quality, 0);

assert.equal(stateColor('QUALIFIED'), 'green');
assert.equal(stateColor('LOCKING'), 'yellow');
assert.equal(stateColor('NO_SIGNAL'), 'blue');
assert.match(adviceFor('HOLD_STEADY'), /light, steady pressure/i);
assert.match(adviceFor('QUALIFIED'), /Browser BPM is ready/i);

console.log('Browser-native Signal Coach core tests passed');
