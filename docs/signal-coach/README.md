# Build and hack Signal Coach at home

This folder is the complete source for the public PulseSensor Signal Coach.
There is no framework, package install, bundler, minified file, server-side
detector, or private runtime. Chrome receives a pulse-wave sample and runs beat
detection, IBI, BPM, confidence, coaching states, and Re-sync locally.

Public app: <https://pulsesensor.com/pages/signal-coach>  
Standalone dashboard: <https://worldfamouselectronics.github.io/PulseSensorPlayground/signal-coach/>

## Five-minute home build

1. Download or clone this repository.
2. Open
   [`examples/SignalCoachWebSerial/SignalCoachWebSerial.ino`](../../examples/SignalCoachWebSerial/SignalCoachWebSerial.ino)
   in Arduino IDE.
3. Confirm `PULSE_PIN` matches the analog pin connected to the purple signal
   wire, then upload. The sketch prints one raw sample every 20 ms at 115200
   baud; no Arduino library is required.
4. From the repository root, start a local web server:

   ```sh
   python3 -m http.server 8000
   ```

5. Open <http://localhost:8000/docs/signal-coach/> in desktop Chrome, choose
   **50 samples/s**, click **Connect**, and select the board's serial port.

Close Arduino Serial Monitor before connecting because only one application can
own the serial port at a time. Web Serial works on HTTPS pages and on
`localhost`; opening `index.html` directly as a file is not supported.

## Complete source map

| File | What to hack |
|---|---|
| [`README.md`](README.md) | This reproducible home-build, customization, testing, and publishing guide. |
| [`index.html`](index.html) | Accessible dashboard structure and controls. |
| [`signal-coach.css`](signal-coach.css) | Large waveform display, coaching colors, layout, and responsive styles. |
| [`signal-coach.mjs`](signal-coach.mjs) | Web Serial connection, sample clock, canvas rendering, UI, and blue Re-sync action. |
| [`signal-coach-core.mjs`](signal-coach-core.mjs) | Transport-neutral beat detector, BPM/IBI, confidence, state order, copy, and v1.1-resync recovery. |
| [`pulse-webserial-protocol.mjs`](pulse-webserial-protocol.mjs) | Text/CSV/JSON/PSWS input adapters. |
| [`signal-coach-core.test.mjs`](signal-coach-core.test.mjs) | 10-bit, 12-bit, multi-rate, flat-wave, false-sender, and Re-sync tests. |
| [`protocol.test.mjs`](protocol.test.mjs) | Accepted and rejected serial-line examples. |
| [`source-pack.test.mjs`](source-pack.test.mjs) | Verifies that the public home-build source pack and links stay complete. |
| [`SignalCoachWebSerial.ino`](../../examples/SignalCoachWebSerial/SignalCoachWebSerial.ino) | Minimal Arduino/ESP32 raw pulse-wave sender. |
| [`page-signal-coach.html`](../../resources/signal-coach/shopify/page-signal-coach.html) | Complete source for the surrounding PulseSensor.com tutorial page. |

The repository root [`LICENSE`](../../LICENSE) is the MIT License. You may use,
copy, modify, merge, publish, and redistribute the code while preserving the
license notice.

## Pulse-wave input contract

The smallest valid stream is one integer per line:

```text
512
518
537
```

Signal Coach also accepts:

- `S512`, `S:512`, or `S=512`;
- `Signal 512`, `signal=512`, `sig:512`, `sample=512`, or `raw=512`;
- `timestamp,signal` CSV;
- JSON containing `signal`, `sample`, `raw`, or `value`;
- Arduino `signal,bpm,ibi,beat` CSV; and
- PulseLink PSWS v1/v2.

The browser uses sender timestamps when present. Otherwise it advances time
using the **Samples/s** choice. Sender BPM, IBI, beat, quality, and coach state
are diagnostics only; raw waveform data always controls the browser coach.

## How the parts fit

```text
PulseSensor -> analogRead() -> USB serial -> input adapter
            -> BrowserSignalCoach -> waveform + guidance + qualified BPM
```

The board is deliberately simple. This makes the same browser coach usable
with Arduino, ESP32, CircuitPython, MicroPython, a desktop serial bridge, or a
new device that prints supported pulse samples.

## Make your own coach

- Change thresholds and timing in `DEFAULTS` at the top of
  `signal-coach-core.mjs`.
- Change beginner-facing guidance in `COACH_COPY`.
- Change state priority only in `coachState()` and add a replay test for the
  new behavior.
- Add a serial format in `parsePulseLine()` and add its exact input to
  `protocol.test.mjs`.
- Restyle the dashboard in `signal-coach.css`; keep blue/yellow/green meanings
  and visible keyboard focus.
- Edit the waveform renderer or technical display in `signal-coach.mjs`.

Do not qualify the coach from a device's claimed BPM or beat flag. If you
change shared behavior, start with a raw-wave replay test so every platform can
port the same evidence-backed rule.

## Test before sharing

No npm install is required. With Node.js installed, run from the repository
root:

```sh
node docs/signal-coach/protocol.test.mjs
node docs/signal-coach/signal-coach-core.test.mjs
node docs/signal-coach/source-pack.test.mjs
```

For visual testing, open this explicitly simulated URL:

<http://localhost:8000/docs/signal-coach/?bench=1>

The page must say **SIMULATED BENCH SIGNAL — NOT A PERSON**. The normal URL
never invents fallback samples.

## Publish a fork

1. Fork this repository on GitHub.
2. In the fork, open **Settings -> Pages**.
3. Deploy from your main branch's `/docs` folder.
4. Open `https://YOUR-NAME.github.io/PulseSensorPlayground/signal-coach/`.

If you embed the dashboard in another page, use an iframe with
`allow="serial"`. Keep the dashboard and its JavaScript modules on HTTPS.

Signal Coach is educational biofeedback software, not a medical device and not
for diagnosis, treatment, or health decisions.
