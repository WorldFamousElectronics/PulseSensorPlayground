import assert from 'node:assert/strict';
import { BrowserSignalCoach } from './signal-coach-core.mjs';
import { DualSignalCoach, PttPairer } from './ptt-coach-core.mjs';

function pulseSample(timestamp, delay = 0) {
  const phase = ((timestamp - delay) % 833 + 833) % 833;
  if (phase < 50) return 500 + Math.round(phase * 3);
  if (phase < 140) return 650 - Math.round((phase - 50) * 1.5);
  return 500;
}

const fixed = new BrowserSignalCoach({ pulseThreshold: 550, thresholdMode: 'fixed' });
for (let timestamp = 0; timestamp < 5000; timestamp += 2) fixed.update(pulseSample(timestamp), timestamp);
assert.equal(fixed.lastResult.threshold, 550, 'fixed threshold must not drift');

const adaptive = new BrowserSignalCoach({ pulseThreshold: 550, thresholdMode: 'adaptive' });
for (let timestamp = 0; timestamp < 5000; timestamp += 2) adaptive.update(pulseSample(timestamp), timestamp);
assert.notEqual(adaptive.lastResult.threshold, 550, 'adaptive threshold should follow the waveform');

const pairer = new PttPairer({ minimumMs: 5, maximumMs: 300 });
pairer.pushProximal(1000);
assert.equal(pairer.pushDistal(1040), 40);
pairer.pushProximal(2000);
pairer.pushProximal(2833);
assert.equal(pairer.pushDistal(2873), 40, 'a new proximal beat must replace a missed old cycle');
assert.equal(pairer.snapshot().unmatched, 1);
pairer.pushProximal(4000);
assert.equal(pairer.pushDistal(4400), null, 'out-of-window pair must be rejected');
assert.equal(pairer.snapshot().rejected, 1);

const dual = new DualSignalCoach({
  proximal: { pulseThreshold: 550, thresholdMode: 'fixed' },
  distal: { pulseThreshold: 550, thresholdMode: 'fixed' },
  quality: { windowSize: 500, warmupSize: 100 },
});
let result;
for (let timestamp = 0; timestamp < 12000; timestamp += 2) {
  result = dual.update(pulseSample(timestamp), pulseSample(timestamp, 40), timestamp);
}
assert.ok(result.ptt.accepted >= 5, 'clean dual wave should produce paired beats');
assert.ok(result.ptt.median >= 36 && result.ptt.median <= 44, `expected about 40 ms, got ${result.ptt.median}`);

const clipped = new DualSignalCoach({ quality: { windowSize: 100, warmupSize: 20 } });
for (let timestamp = 0; timestamp < 12000; timestamp += 2) {
  result = clipped.update(1023, pulseSample(timestamp, 40), timestamp);
}
assert.equal(result.proximalQuality.state, 'CLIPPED');
assert.equal(result.ptt.accepted, 0, 'clipped channels must not produce accepted PTT');

console.log('Signal Coach dual-channel PTT tests passed');
