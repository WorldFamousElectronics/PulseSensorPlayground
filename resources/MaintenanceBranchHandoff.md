# Maintenance Branch Handoff

Branch:

```text
codex/maintenance-traceability-ci
```

Draft PR:

```text
https://github.com/WorldFamousElectronics/PulseSensorPlayground/pull/207
```

## Current State

The remote PR branch includes:

- timer and bounds maintenance fixes
- version metadata alignment to `2.5.1`
- maintainer notes
- end-user ZIP install smoke test
- manual hardware test plan
- draft `2.5.2` release notes

The local branch has one extra commit that is not pushed:

```text
7e624af Add Arduino compile smoke workflow
```

That commit adds:

```text
.github/workflows/arduino-compile.yml
```

It is local-only because GitHub rejects workflow-file pushes unless the token has `workflow` scope.

## Workflow Auth Step

When a maintainer is available, run:

```sh
gh auth refresh -h github.com -s workflow
```

Complete the GitHub browser/device-code approval. Then push:

```sh
cd /Users/narwhal2/Documents/PulseSensorPlayground
git push
```

If the local branch is not checked out:

```sh
cd /Users/narwhal2/Documents/PulseSensorPlayground
git switch codex/maintenance-traceability-ci
git push
```

## Local Validation Already Run

Basic checks:

```sh
git diff --check
ruby -e 'require "yaml"; YAML.load_file(".github/workflows/arduino-compile.yml"); puts "YAML OK"'
```

End-user install check:

```sh
scripts/end-user-install-test.sh
```

Expanded installed-library compile matrix on `esp32:esp32:XIAO_ESP32S3`:

- `GettingStartedProject`
- `Getting_BPM_to_Monitor`
- `PulseSensor_BPM`
- `PulseSensor_Pulse_Transit_Time`
- `TwoPulseSensors_On_OneArduino`
- `PulseSensor_XIAO_ESP32S3_BPM`

The test installs a ZIP of the current commit into a temporary Arduino sketchbook and compiles from the installed library folder.

## Known Local Limitation

AVR and UNO R4 compiles were attempted locally, but this Mac's installed Arduino toolchains fail before compilation with:

```text
bad CPU type in executable
```

Use the GitHub Actions workflow or another machine to validate:

- `arduino:avr:uno`
- `arduino:avr:mega`
- `arduino:renesas_uno:unor4wifi`

## Hardware Still Needed

Run the checklist in:

```text
resources/ManualHardwareTestPlan.md
```

Highest-value first pass:

1. Arduino Uno R3 or Nano with `PulseSensor_BPM`
2. XIAO ESP32-S3 with `PulseSensor_XIAO_ESP32S3_BPM`
3. UNO R4 WiFi with LED matrix plotter
4. Two-sensor PTT setup

## Related Docs PR

Separate draft PR:

```text
https://github.com/WorldFamousElectronics/PulseSensorPlayground/pull/208
```

That PR links WebSerial and CYD resources without mixing them into the maintenance-fix PR.
