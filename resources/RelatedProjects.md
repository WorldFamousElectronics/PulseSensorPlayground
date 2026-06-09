# Related PulseSensor Projects

These projects use PulseSensor Playground but are not bundled as Arduino Library Manager examples.

Keeping them linked here gives users a path from the core library to richer browser and ESP32 display demos without making the installed Arduino library package larger or harder to maintain.

## PulseSensor WebSerial

- Tutorial: [PulseSensor and Webserial](https://pulsesensor.com/pages/pulsesensor-and-webserial)
- Development repo: [yury-g/webserial](https://github.com/yury-g/webserial)
- Local library resource: [webserial-explainer](webserial-explainer/index.html)

WebSerial lets Chrome, Edge, or Brave read USB serial data directly from an Arduino or ESP32. It is useful for quick waveform/BPM testing because users do not need Processing or a desktop application.

## PulseSensor on CYD

- Tutorial: [PulseSensor on CYD](https://pulsesensor.com/pages/cyd)
- Official source repo: [WorldFamousElectronics/PulseSensor_CYD](https://github.com/WorldFamousElectronics/PulseSensor_CYD)
- Development repo: [yury-g/CYD-Two-PulseSensor](https://github.com/yury-g/CYD-Two-PulseSensor)

The CYD project targets ESP32-2432S028 display boards and demonstrates how Playground readings can drive an embedded display: waveform, BPM, IBI, pulse amplitude, beat state, and threshold behavior.

## Why These Are Linked Instead Of Bundled

The Arduino Library Manager package should stay focused on examples that compile directly from Arduino IDE with predictable dependencies.

These related projects are richer applications. They may need browser features, display configuration, board-specific pin mapping, or additional libraries. Linking them keeps the installed Playground library lean while still making the project path discoverable.
