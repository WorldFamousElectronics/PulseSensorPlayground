import assert from 'node:assert/strict';
import { access, readFile } from 'node:fs/promises';
import { fileURLToPath } from 'node:url';
import path from 'node:path';

const folder = path.dirname(fileURLToPath(import.meta.url));
const root = path.resolve(folder, '../..');
const required = [
  'docs/pulse-transit-time/README.md',
  'docs/pulse-transit-time/index.html',
  'docs/pulse-transit-time/pulse-transit-time.css',
  'docs/pulse-transit-time/pulse-transit-time.mjs',
  'docs/pulse-transit-time/pulse-transit-time-core.mjs',
  'docs/pulse-transit-time/pulse-transit-time-core.test.mjs',
  'docs/pulse-transit-time/pulse-transit-time-protocol.mjs',
  'docs/pulse-transit-time/pulse-transit-time-protocol.test.mjs',
  'docs/pulse-transit-time/source-pack.test.mjs',
  'docs/signal-coach/signal-coach-core.mjs',
  'examples/PulseTransitTimeWebSerial/PulseTransitTimeWebSerial.ino',
  'LICENSE',
];

await Promise.all(required.map((relativePath) => access(path.join(root, relativePath))));

const guide = await readFile(path.join(folder, 'README.md'), 'utf8');
const dashboard = await readFile(path.join(folder, 'index.html'), 'utf8');
const browserApp = await readFile(path.join(folder, 'pulse-transit-time.mjs'), 'utf8');
const protocol = await readFile(path.join(folder, 'pulse-transit-time-protocol.mjs'), 'utf8');
const core = await readFile(path.join(folder, 'pulse-transit-time-core.mjs'), 'utf8');
const sketch = await readFile(path.join(root, 'examples/PulseTransitTimeWebSerial/PulseTransitTimeWebSerial.ino'), 'utf8');

for (const relativePath of required) {
  assert.ok(guide.includes(path.basename(relativePath)), `source map should name ${relativePath}`);
}

assert.match(dashboard, /Pulse Transit Time Lab/);
assert.match(dashboard, /A0 proximal · A1 distal/);
assert.match(dashboard, /EDUCATIONAL TIMING EXPERIMENT/);
assert.match(dashboard, /id="proximalThresholdMode"/);
assert.match(dashboard, /id="distalThresholdMode"/);
assert.match(dashboard, /id="acceptedPairs"/);
assert.match(dashboard, /PulseTransitTimeWebSerial\.ino/);
assert.match(dashboard, /Serial\.begin\(250000\)/);
assert.match(dashboard, /Serial\.print\("PTT1,"\)/);
assert.match(dashboard, /pulsesensor-pulse-transit-time-ready/);
assert.doesNotMatch(dashboard, /Signal Coach/);
assert.doesNotMatch(browserApp, /coachMode|pulse-webserial-protocol|SignalCoachWebSerial/);
assert.match(browserApp, /pulse-transit-time-protocol\.mjs\?v=/);
assert.match(browserApp, /pulse-transit-time-core\.mjs\?v=/);
assert.match(browserApp, /function clearSession\(\)/);
assert.match(browserApp, /resetSession\(\)/);
assert.match(browserApp, /async function finalizeConnection\(targetPort\)/);
assert.match(browserApp, /if \(!targetPort \|\| targetPort !== port\) return false;/);
assert.match(browserApp, /finalizeConnection\(event\.target\)/);
const connectBody = browserApp.match(/async function connect\(\) \{([\s\S]*?)\n\}\n\nconnectBtn/)?.[1];
assert.ok(connectBody, 'PTT connect function should be inspectable');
assert.doesNotMatch(connectBody, /control\.disabled = true/, 'live PTT controls must remain enabled');
assert.match(protocol, /fields\[0\] !== 'PTT1'/);
assert.match(protocol, /\^\(\?:0\|\[1-9\]\\d\*\)\$/);
assert.doesNotMatch(protocol, /PSWS|raw-number|arduino-csv/);
assert.match(core, /\.\.\/signal-coach\/signal-coach-core\.mjs/);
assert.doesNotMatch(core, /signal-coach\.mjs|pulse-webserial-protocol/);
assert.match(sketch, /PulseSensor Pulse Transit Time/);
assert.doesNotMatch(sketch, /Signal Coach/);
assert.match(sketch, /analogReadResolution\(10\)/);
assert.match(sketch, /SAMPLE_PERIOD_US = 2000/);
assert.match(sketch, /Serial\.begin\(250000\)/);
assert.match(sketch, /Serial\.print\("PTT1,"\)/);

const releaseVersion = dashboard.match(/pulse-transit-time\.mjs\?v=([a-z0-9-]+)/i)?.[1];
assert.ok(releaseVersion, 'dashboard should version its browser module');
assert.ok(dashboard.includes(`pulse-transit-time.css?v=${releaseVersion}`));
assert.ok(browserApp.includes(`pulse-transit-time-protocol.mjs?v=${releaseVersion}`));
assert.ok(browserApp.includes(`pulse-transit-time-core.mjs?v=${releaseVersion}`));
assert.ok(core.includes(`signal-coach-core.mjs?v=${releaseVersion}`));

await assert.rejects(access(path.join(root, 'examples/SignalCoachDualWebSerial/SignalCoachDualWebSerial.ino')));
await assert.rejects(access(path.join(root, 'docs/signal-coach/ptt-coach-core.mjs')));

console.log('Pulse Transit Time source-pack tests passed');
