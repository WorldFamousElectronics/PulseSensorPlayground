# Signal Coach parallel-preview release review — 2026-08-12

Status: **SHIP AS A PARALLEL TESTER PREVIEW**

## Scope

- Publish a waveform-first Signal Coach experience on a new Shopify page while
  leaving the existing WebSerial page unchanged for comparison and validation.
- Keep the published `signal,bpm,ibi,beat` Arduino/ESP32 stream compatible.
- Keep PulseLink StickS3 `PSWS` v1 and v2 streams compatible.
- Move IBI, raw Signal, Beat, and transport diagnostics into Technical details.

## Verification

- Protocol unit tests: pass for legacy four-value, PSWS v1, and PSWS v2 frames.
- Signal Coach unit test: pass through noisy, locking, and qualified states.
- Deterministic browser replay: reaches QUALIFIED at 72 BPM with 12/12 quality
  and zero sequence gaps; the replay is visibly marked SIM.
- Responsive browser check: 390 × 844 layout remains readable and usable.
- Attached StickS3 check: live PSWS v2 stream received at 115200 baud, waveform
  rendered, coach reached GOOD WAVE, and sequence gaps remained at zero.
- Published Shopify iframe check: Web Serial permission chooser opened from the
  cross-origin Signal Coach embed and the attached StickS3 streamed live PSWS v2
  frames into the production page.
- Preservation check: the existing Shopify page still contains its original
  Signal Waveform and Four Values content and does not contain the new preview.

## Claims and support review

- The dashboard does not invent live sensor data or silently fall back to a
  simulation.
- BPM is withheld until the coach accepts the recent signal.
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
