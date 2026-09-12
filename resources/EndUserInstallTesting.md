# End User Install Testing

Use this test before merging changes that might affect Arduino IDE users.

The goal is to verify the library the way an end user gets it: as an installed Arduino library, not as a checked-out source tree.

## ZIP install smoke test

Run this from the repository root:

```sh
scripts/end-user-install-test.sh
```

The script:

1. creates a ZIP from the current Git commit
2. installs that ZIP into a temporary Arduino sketchbook
3. compiles examples from the installed library folder

By default it compiles `PulseSensor_XIAO_ESP32S3_BPM` for `esp32:esp32:XIAO_ESP32S3`, because that catches the current ESP32-S3 support path without requiring optional display or web server libraries.

## Broader board matrix

Set `PULSE_SENSOR_END_USER_MATRIX` to run more boards. Each line uses:

```text
name|core|fqbn|sketch|additional_board_manager_urls
```

Example:

```sh
PULSE_SENSOR_INSTALL_CORES=1 PULSE_SENSOR_END_USER_MATRIX='Uno BPM|arduino:avr|arduino:avr:uno|examples/PulseSensor_BPM|
Mega two-sensor|arduino:avr|arduino:avr:mega|examples/TwoPulseSensors_On_OneArduino|
UNO R4 WiFi plotter|arduino:renesas_uno|arduino:renesas_uno:unor4wifi|examples/PulseSensor_UNO_R4_WiFi_LEDmatrix_Plotter|
XIAO ESP32-S3 BPM|esp32:esp32|esp32:esp32:XIAO_ESP32S3|examples/PulseSensor_XIAO_ESP32S3_BPM|https://espressif.github.io/arduino-esp32/package_esp32_index.json' \
scripts/end-user-install-test.sh
```

Some optional examples require extra libraries such as display, websocket, Bluetooth, or async web server packages. Keep those in a separate optional matrix so the core library install test remains focused and dependable.
