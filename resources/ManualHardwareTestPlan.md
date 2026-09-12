# Manual Hardware Test Plan

Use this checklist before merging or releasing maintenance changes to PulseSensor Playground.

The goal is to confirm that the library works for end users after installation through Arduino IDE or a downloaded ZIP, not only from a developer checkout.

## Install Path

1. Download the branch ZIP:

   ```text
   https://github.com/WorldFamousElectronics/PulseSensorPlayground/archive/refs/heads/codex/maintenance-traceability-ci.zip
   ```

2. In Arduino IDE, use `Sketch > Include Library > Add .ZIP Library...`.
3. Confirm examples appear under `File > Examples > PulseSensor Playground`.
4. Open each target example from the Examples menu, not from the Git checkout.

## Boards To Smoke Test

### Arduino Uno R3 or Nano

Examples:

- `PulseSensor_BPM`
- `PulseSensor_Speaker`
- `TwoPulseSensors_On_OneArduino`

Checks:

- Upload succeeds.
- Serial Plotter shows live samples.
- BPM becomes non-zero after several clean beats.
- LED blink follows `sawStartOfBeat()`.
- Speaker example uses PWM-style beep behavior without `tone()` duration.

### Arduino Mega 2560

Example:

- `TwoPulseSensors_On_OneArduino`

Checks:

- Upload succeeds.
- Both channels produce independent sample traces.
- Calling two-sensor methods does not hang or reset the board.
- Out-of-range sensor indexes are ignored or return error values rather than reading past the sensor array.

### Arduino UNO R4 WiFi

Examples:

- `PulseSensor_UNO_R4_WiFi_LEDmatrix_Plotter`
- `PulseSensor_UNO_R4_WiFi_LEDmatrix_Heartbeat`

Checks:

- Upload succeeds.
- LED matrix updates.
- Serial Plotter output remains readable.
- `pause()` / `resume()` can be tried manually if a small sketch is available.

### Seeed XIAO ESP32-S3

Example:

- `PulseSensor_XIAO_ESP32S3_BPM`

Checks:

- Upload succeeds.
- Serial Monitor reports BPM after clean beats.
- `analogReadResolution(10)` behavior remains compatible with Playground's 0..1023 detector assumptions where examples use ESP32 ADC.

### Pulse Transit Time Setup

Example:

- `PulseSensor_Pulse_Transit_Time`

Checks:

- Both sensors share the same voltage and ground.
- Visualizer or Serial Plotter shows two clean waveforms.
- PTT values change only when both channels detect beats.
- Fingertip/earlobe placement gives cleaner signals than wrist placement.

## Regression Areas

- `sawNewSample()` should return `true` after hardware-timer samples.
- `sensorIndex == SensorCount` should not access outside the sensor array.
- `pause()` and `resume()` should return `true` on supported hardware timer platforms.
- Unsupported boards should fall back to software timing with a compile warning.
- Library version output should report `v2.5.1` while this branch remains a test branch.

## Pass Criteria

This maintenance branch is ready to merge when:

- At least one AVR board passes.
- XIAO ESP32-S3 passes.
- UNO R4 WiFi passes, or any failure is documented as board-core/toolchain specific.
- The branch ZIP install path works in Arduino IDE.
- No example requires editing the installed library files to compile.
