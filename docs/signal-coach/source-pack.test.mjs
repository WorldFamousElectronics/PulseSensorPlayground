import assert from 'node:assert/strict';
import { access, readFile } from 'node:fs/promises';
import { fileURLToPath } from 'node:url';
import path from 'node:path';

const folder = path.dirname(fileURLToPath(import.meta.url));
const root = path.resolve(folder, '../..');
const required = [
  'docs/signal-coach/README.md',
  'docs/signal-coach/index.html',
  'docs/signal-coach/signal-coach.css',
  'docs/signal-coach/signal-coach.mjs',
  'docs/signal-coach/signal-coach-core.mjs',
  'docs/signal-coach/ptt-coach-core.mjs',
  'docs/signal-coach/pulse-webserial-protocol.mjs',
  'docs/signal-coach/protocol.test.mjs',
  'docs/signal-coach/signal-coach-core.test.mjs',
  'docs/signal-coach/ptt-coach-core.test.mjs',
  'examples/SignalCoachWebSerial/SignalCoachWebSerial.ino',
  'examples/SignalCoachDualWebSerial/SignalCoachDualWebSerial.ino',
  'resources/signal-coach/shopify/page-signal-coach.html',
  'LICENSE',
];

await Promise.all(required.map((relativePath) => access(path.join(root, relativePath))));

const guide = await readFile(path.join(folder, 'README.md'), 'utf8');
const dashboard = await readFile(path.join(folder, 'index.html'), 'utf8');
const browserApp = await readFile(path.join(folder, 'signal-coach.mjs'), 'utf8');
const sketch = await readFile(path.join(root, 'examples/SignalCoachWebSerial/SignalCoachWebSerial.ino'), 'utf8');
const dualSketch = await readFile(path.join(root, 'examples/SignalCoachDualWebSerial/SignalCoachDualWebSerial.ino'), 'utf8');
const tutorial = await readFile(path.join(root, 'resources/signal-coach/shopify/page-signal-coach.html'), 'utf8');

for (const relativePath of required) {
  assert.ok(guide.includes(path.basename(relativePath)), `source map should name ${relativePath}`);
}
assert.match(dashboard, /Build it at home/i);
assert.match(dashboard, /View all source/i);
assert.match(dashboard, /wave-coach-layout/);
assert.match(dashboard, /id="coachLabel"/);
assert.match(dashboard, /id="coachAdvice"/);
assert.match(dashboard, /Blue <b>Adjust<\/b>/);
assert.match(dashboard, /Light touch/);
assert.match(sketch, /Serial\.println\(analogRead\(PULSE_PIN\)\)/);
assert.match(sketch, /SAMPLE_PERIOD_MS = 20/);
assert.match(tutorial, /Hack Signal Coach at home/i);
assert.match(tutorial, /SignalCoachWebSerial\.ino/);
assert.match(tutorial, /docs\/signal-coach\/README\.md/);
assert.match(tutorial, /another finger or person/);
assert.doesNotMatch(tutorial, /<div class="psc-coach-grid">/);
assert.doesNotMatch(tutorial, /Signal Coach tester preview/);
assert.doesNotMatch(tutorial, /See the wave\. Improve it\./);
assert.match(tutorial, /1 Upload the one- or two-sensor sketch/);
assert.match(tutorial, /3 Choose the mode and Connect/);
assert.match(tutorial, /Loading Signal Coach/);
assert.match(tutorial, /Taking longer than expected/);
assert.match(tutorial, /Open Signal Coach/);
assert.match(tutorial, /Coach not loading\? Open directly\./);
assert.match(tutorial, /pulsesensor-signal-coach-ready/);
assert.match(dashboard, /pulsesensor-signal-coach-ready/);
assert.match(dashboard, /Two sensors \/ PTT/);
assert.match(dashboard, /id="proximalThresholdMode"/);
assert.match(dashboard, /id="distalThresholdMode"/);
assert.match(dashboard, /UNO R4 WiFi setup/);
assert.match(dashboard, /Flash from this page: not yet/);
assert.match(dashboard, /id="copySketchBtn"/);
assert.match(dashboard, /Serial\.begin\(250000\)/);
assert.match(dashboard, /Serial\.print\("PTT1,"\)/);
assert.match(dashboard, /ARDUINO_UNOR4_WIFI/);
assert.match(dualSketch, /analogReadResolution\(10\)/);
assert.match(tutorial, /SignalCoachDualWebSerial\.ino/);
assert.match(tutorial, /timing experiment, not a blood-pressure reading/i);

const releaseVersion = dashboard.match(/signal-coach\.mjs\?v=([a-z0-9-]+)/i)?.[1];
assert.ok(releaseVersion, 'dashboard should version its browser module');
assert.ok(dashboard.includes(`signal-coach.css?v=${releaseVersion}`));
assert.ok(browserApp.includes(`pulse-webserial-protocol.mjs?v=${releaseVersion}`));
assert.ok(browserApp.includes(`signal-coach-core.mjs?v=${releaseVersion}`));
assert.ok(browserApp.includes(`ptt-coach-core.mjs?v=${releaseVersion}`));

console.log('Signal Coach public source-pack tests passed');
