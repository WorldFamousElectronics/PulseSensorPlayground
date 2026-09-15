# Signal Coach parallel-preview release review — 2026-08-12

Status: **SHIP AS A PARALLEL TESTER PREVIEW**

## Scope

- Publish a waveform-first Signal Coach experience on a new Shopify page while
  leaving the existing WebSerial page unchanged for comparison and validation.
- Keep the published `signal,bpm,ibi,beat` Arduino/ESP32 stream compatible.
- Keep PulseLink StickS3 `PSWS` v1 and v2 streams compatible.
- Run beat detection, confidence, IBI, BPM, and coach states in Chrome from the
  pulse wave; sender-calculated values are diagnostic only.
- Accept raw numeric, `S512`, labeled, timestamped CSV, and JSON pulse samples.
- Move IBI, raw Signal, Beat, and transport diagnostics into Technical details.
- Mark the public experience Beta with a visible release date and time.
- Add an on-page Shopify contact form that delivers tester feedback to the
  store contact address without publishing that address.
- Make the PulseLink v1.1 recovery a prominent blue `Re-sync Signal` button;
  the action runs against the live wave in Chrome.

## Verification

- Protocol unit tests: pass for raw numeric, S-value, labeled, timestamped CSV,
  JSON, legacy four-value, PSWS v1, and PSWS v2 frames.
- Browser-native Signal Coach unit tests: pass for flat input, 10-bit Arduino
  pulse waves, 12-bit ESP32 pulse waves, re-sync, and 50/100/200/500 samples/s.
- Re-sync regression: a clean 12-bit wave re-qualifies within four seconds
  after stale timing, confidence, and threshold state are cleared.
- Release-cache regression: the dashboard stylesheet and behavior module carry
  the same blue-Re-sync version key so new markup cannot run an older script.
- Public source-pack test: the unminified UI, coach core, transport adapters,
  tests, MIT license, Shopify tutorial source, home-build guide, and minimal
  Arduino sketch are present and cross-linked.
- Graph-companion layout keeps the short plain-English purpose, color states,
  placement and pressure tips, and Re-sync action physically beside the wave.
  The embedded app uses the PulseSensor.com light palette and site typography.
- Embedded mode removes the dashboard's duplicate title and description so the
  connection controls, waveform, and attached guide rise directly under the
  page's short introduction; the standalone dashboard keeps its identity.
- Feedback guidance asks for board/rate, expected and actual behavior,
  repeatable steps, and whether a second finger or person reproduces the result.
- Deterministic browser replay: reaches QUALIFIED at 72 BPM with 12/12 quality
  and zero sequence gaps; the replay is visibly marked SIM.
- Responsive browser check: 390 × 844 layout remains readable and usable.
- Attached StickS3 check: live PSWS v2 stream received at 115200 baud, waveform
  rendered, coach reached GOOD WAVE, and sequence gaps remained at zero.
- Published Shopify iframe check: Web Serial permission chooser opened from the
  cross-origin Signal Coach embed and the attached StickS3 streamed live PSWS v2
  frames into the production page.
- Feedback-form check: required result, notes, and reply-email controls render;
  the form posts to Shopify `/contact` and was not submitted during QA.
- Preservation check: the existing Shopify page still contains its original
  Signal Waveform and Four Values content and does not contain the new preview.

## Claims and support review

- The dashboard does not invent live sensor data or silently fall back to a
  simulation.
- BPM is withheld until the browser coach accepts the recent signal.
- PulseLink and Arduino BPM/IBI/beat/quality values cannot qualify the browser
  coach or make BPM appear.
- Removed the old page's unsupported stress and resting-range claims.
- Added local-processing and educational/not-medical disclosures.
- Troubleshooting now maps coach states to concrete placement, pressure, motion,
  wiring, browser, and serial-port actions.

## Existing-page preservation and rollback

The existing Shopify page `gid://shopify/Page/117788999785` is not changed by
this release. Its body is preserved in `shopify/page-before-2026-08-12.html` as
an audit snapshot. GitHub Pages can be rolled back by reverting the Signal Coach
release commit on `master`; the separate Signal Coach page can be unpublished or
removed without changing the existing WebSerial page.
