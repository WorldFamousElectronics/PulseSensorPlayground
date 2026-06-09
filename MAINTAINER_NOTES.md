# Maintainer Notes

## 2026-06-09 maintenance branch

This branch is intentionally kept on the `2.5.1` release line so behavior can be tested against the published release without introducing a new public version number.

The maintenance goals are:

- align the Arduino Library Manager metadata and in-code version macros with release `v2.5.1`
- make sensor index checks reject `sensorIndex == SensorCount`
- make `sawNewSample()` match its documented behavior when hardware timers are used
- align timer setup, pause, and resume platform macros for nRF52 and Due
- fix small timer setup issues found by inspection before adding broader compile coverage

Open follow-up areas:

- Add GitHub Actions smoke tests for supported Arduino cores that can be compiled automatically. A workflow file was prepared during this branch, but pushing `.github/workflows/arduino-compile.yml` requires a GitHub token with `workflow` scope.
- Expand CI to optional examples that require display, websocket, Bluetooth, and web server dependencies.
- Decide whether Servo timer selection should depend on `Servo.h` being included by the sketch instead of merely available in the installed core.
- Add board-specific manual test notes for Pulse Transit Time and speaker/tone timer interactions.
