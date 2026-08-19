# Build and test Pulse Transit Time Lab

Pulse Transit Time Lab is the dedicated two-sensor PulseSensor project. It is
not a mode of Signal Coach. The projects have separate pages, firmware,
protocol adapters, documentation, and validation gates. The PTT detector
deliberately reuses only Signal Coach's transport-neutral beat-detection core.

Candidate standalone dashboard path after merge and Pages deployment:
<https://worldfamouselectronics.github.io/PulseSensorPlayground/pulse-transit-time/>

## Two-sensor build

1. Connect the proximal sensor's purple wire to A0 and the distal sensor's
   purple wire to A1. Both sensors share 5V and GND.
2. Upload
   [`PulseTransitTimeWebSerial.ino`](../../examples/PulseTransitTimeWebSerial/PulseTransitTimeWebSerial.ino).
3. Close Arduino Serial Monitor and any other application using the serial
   port.
4. From the repository root, start a local web server:

   ```sh
   python3 -m http.server 8000
   ```

5. Open <http://localhost:8000/docs/pulse-transit-time/> in desktop Chrome,
   click **Connect two-sensor stream**, and select the board.

The dedicated sender emits synchronized A0/A1 samples at 500 samples/s and
250000 baud. The page does not compile or flash the board.

## Source map

| File | Responsibility |
|---|---|
| [`README.md`](README.md) | This build, architecture, test, and physical-validation guide. |
| [`index.html`](index.html) | Dedicated two-sensor page structure and controls. |
| [`pulse-transit-time.css`](pulse-transit-time.css) | PTT page layout and responsive waveform presentation. |
| [`pulse-transit-time.mjs`](pulse-transit-time.mjs) | PTT Web Serial ownership, rendering, settings, reset, and bench replay. |
| [`pulse-transit-time-protocol.mjs`](pulse-transit-time-protocol.mjs) | Strict PTT1-only serial adapter. |
| [`pulse-transit-time-protocol.test.mjs`](pulse-transit-time-protocol.test.mjs) | PTT1 acceptance and one-sensor-format rejection tests. |
| [`pulse-transit-time-core.mjs`](pulse-transit-time-core.mjs) | Dual-channel quality gates and one-to-one beat pairing. |
| [`pulse-transit-time-core.test.mjs`](pulse-transit-time-core.test.mjs) | Threshold, missed-beat, timing-window, clipping, and synthetic PTT tests. |
| [`source-pack.test.mjs`](source-pack.test.mjs) | Architecture and source-completeness checks. |
| [`signal-coach-core.mjs`](../signal-coach/signal-coach-core.mjs) | Deliberately shared transport-neutral beat-detection algorithm. |
| [`PulseTransitTimeWebSerial.ino`](../../examples/PulseTransitTimeWebSerial/PulseTransitTimeWebSerial.ino) | Dedicated synchronized A0/A1 sender. |

No Signal Coach browser UI, serial adapter, firmware, product copy, or
validation result is shared with this project.

The repository root [`LICENSE`](../../LICENSE) is the MIT License.

## PTT1 input contract

Each line is one synchronized sample:

```text
PTT1,timestamp_us,proximal,distal
PTT1,123456,532,601
```

The timestamp is the sender's unsigned microsecond clock. All three numeric
fields use canonical unsigned decimal digits only: no empty values, signs,
hexadecimal notation, exponents, or leading zeroes. Both samples belong to
that same acquisition instant. One-sensor numeric, labeled, JSON, and PSWS
formats are intentionally ignored here.

## Detector and pairing behavior

- A0 and A1 each have an explicit fixed or adaptive threshold.
- Each channel must clear warmup, range, and clipping checks.
- A distal beat is accepted only after a proximal beat in the configured
  5–300 ms default window.
- Each proximal beat is consumed at most once; stale cycles are unmatched,
  never carried forward.
- Changing thresholds or the pairing window resets the analysis counters so
  results from different configurations are not mixed, while the live serial
  connection stays open for tuning during placement.
- Disconnect, reconnect, and QA reset clear the prior median and pair counts.

The result is an educational timing measurement. It is not blood pressure and
must not be used for diagnosis, treatment, or health decisions.

## Automated checks

No npm install is required. From the repository root run:

```sh
node docs/pulse-transit-time/pulse-transit-time-protocol.test.mjs
node docs/pulse-transit-time/pulse-transit-time-core.test.mjs
node docs/pulse-transit-time/source-pack.test.mjs
```

Browser-only simulated replay:

<http://localhost:8000/docs/pulse-transit-time/?bench=1>

The page must say **SIMULATED BENCH SIGNAL — NOT A PERSON**. The normal URL
never invents fallback samples.

## Physical HOLD gate

The browser transport smoke test is not physiological validation. Keep the
project in HOLD until a real two-sensor run records all of the following:

- both sensors share 5V and GND, with A0 proximal and A1 distal;
- both channel quality states remain GOOD without ADC clipping;
- accepted same-cycle pairs increase repeatedly rather than once;
- median PTT is reasonably stable across a sustained placement;
- disconnect/reconnect begins a visibly fresh session;
- raw placement, threshold, and timing-window notes are captured with the run.

Do not merge the release candidate, publish a customer page, close issue #202,
or notify customers until that physical gate passes.
