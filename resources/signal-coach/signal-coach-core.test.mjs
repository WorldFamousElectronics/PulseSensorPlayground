// Deterministic Signal Coach state-transition tests.
import assert from 'node:assert/strict';
import { LegacySignalCoach, adviceFor, stateColor } from './signal-coach-core.mjs';

const flat = new LegacySignalCoach();
let result;
for (let index = 0; index < 100; index += 1) {
  result = flat.update({ signal: 2048 + (index % 3), bpm: 0, beat: 0 }, index * 20);
}
assert.equal(result.state, 'NO_SIGNAL');
assert.equal(result.locked, false);

const coached = new LegacySignalCoach();
for (let index = 0; index < 450; index += 1) {
  const time = index * 20;
  const phase = time % 833;
  const signal = 1750 + (phase < 110 ? Math.round((110 - phase) * 7) : 0);
  const beat = phase < 20 && (time < 20 || ((time - 20) % 833) > 813) ? 1 : 0;
  result = coached.update({ signal, bpm: index > 180 ? 72 : 0, beat }, time);
}
assert.equal(result.state, 'QUALIFIED');
assert.equal(result.locked, true);
assert.ok(result.quality >= 10);

assert.equal(stateColor('QUALIFIED'), 'green');
assert.equal(stateColor('LOCKING'), 'yellow');
assert.equal(stateColor('NO_SIGNAL'), 'blue');
assert.match(adviceFor('HOLD_STEADY'), /light, steady pressure/i);
assert.match(adviceFor('QUALIFIED'), /BPM is ready/i);

console.log('PulseSensor Signal Coach core tests passed');
