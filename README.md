![PulseSensor logo](https://avatars0.githubusercontent.com/u/7002937?v=3&s=200)

## [PulseSensor.com](https://pulsesensor.com) Playground

Ready-to-use Arduino code for your PulseSensor. See your first pulse wave, calculate BPM, or make something respond to your heartbeat. 💓

**Start here:** Install **PulseSensor Playground** in Arduino Library Manager, open [GettingStartedProject](examples/GettingStartedProject/GettingStartedProject.ino), then try [PulseSensor_BPM](examples/PulseSensor_BPM/PulseSensor_BPM.ino) once your waveform is steady.

- Commented examples help you get started and keep experimenting. 🤘
- Switch projects right in the Arduino IDE. 💻
- Share what you make with the community. 🐝

---

## Buy PulseSensor

- **United States:** [Buy the PulseSensor kit on Amazon](https://www.amazon.com/dp/B01CPP4QM0).
- **International:** Buy from [SparkFun](https://www.sparkfun.com/pulse-sensor.html) or [Adafruit](https://www.adafruit.com/product/1093).
- **Education & Bulk:** [Request a bulk quote](https://pulsesensor.com/pages/school-bulk-orders). 50-kit minimum, in increments of 25 kits.

---

## Connecting the Hardware 😎

<details>
<summary>Easy setup: prepare, connect, and get a steady signal</summary>

### 1. Prepare your sensor

The current kit includes the **TPU Stabilizer Ring**, Velcro finger strap, ear clip, and transparent vinyl insulation dots. You'll also need a compatible development board, a USB data cable, and a computer.

Before skin contact, apply a transparent vinyl dot to the sensor face and fully insulate the rear electronics using the preparation method supplied with your kit. The ring helps hold the sensor steady; it does not replace insulation. Follow the website's [Start Here guide](https://pulsesensor.com/pages/start), or contact [support](mailto:support@pulsesensor.com) if the preparation method is unclear.

### 2. Connect the three wires

For the **Arduino Uno GettingStartedProject**:

| PulseSensor wire | Arduino Uno connection |
| --- | --- |
| Purple — signal | `A0` |
| Red — power | `5V` |
| Black — ground | `GND` |

Using another board? Follow its example's analog pin and voltage instructions. PulseSensor needs an analog input (ADC); a 5V Uno wiring diagram does not apply to every board.

### 3. Hold it gently

After preparation, position the sensor with the ring, strap, or ear clip. Rest your hand, keep the cable from pulling, and use light, steady contact. Load the Getting Started example below and look for a repeating waveform before moving on to BPM.

</details>

## Loading the Playground

<details>
<summary>Install the library and see your first pulse wave 🤓</summary>

1. Install [Arduino IDE](https://www.arduino.cc/en/software/) if you don't already have it.
2. Open **Tools → Manage Libraries…**, search for **PulseSensor Playground**, and click **Install** or **Update**.
3. Open **File → Examples → PulseSensor Playground → GettingStartedProject**.
4. Select your board and USB port, then upload the sketch. For other boards, use the matching [board-specific example](https://pulsesensor.com/pages/installing-our-playground-for-pulsesensor-arduino).
5. Open **Tools → Serial Plotter** and select **115200 baud** for `GettingStartedProject`.
6. Hold the prepared sensor gently against your fingertip. Give the signal 5–10 seconds to settle; look for a repeating pulse wave and the built-in LED blinking as the signal crosses the threshold.

Ready for BPM? Open [PulseSensor_BPM](examples/PulseSensor_BPM/PulseSensor_BPM.ino) and follow the [BPM tutorial](https://pulsesensor.com/pages/getting-advanced).

[Getting Started tutorial](https://pulsesensor.com/pages/code-and-guide) · [Library and example guide](https://pulsesensor.com/pages/installing-our-playground-for-pulsesensor-arduino)

</details>

## Web Serial Signal Coach

<details>
<summary>See your pulse in the browser with Signal Coach</summary>

[Open Signal Coach](https://pulsesensor.com/pages/signal-coach) for a live waveform and guidance on sensor placement in desktop Chrome or Edge.

Upload the [Web Serial example](examples/SignalCoachWebSerial/SignalCoachWebSerial.ino), close Arduino Serial Monitor and Serial Plotter, select **50 samples/s**, then click **Connect**. The website walks you through setup.

Want to run it locally or change the code? See the [full source and home-build guide](docs/signal-coach/README.md).

</details>

## Playground Project Descriptions
<details>
<summary>Tinker and experiment with popular projects 👩🏽‍💻👩🏻‍🔬</summary>

### The Getting Started Project
  See your raw pulse wave in Serial Plotter and blink the built-in LED.

- [**Project Page**](https://pulsesensor.com/pages/code-and-guide)

  <img alt="PulseSensor Getting Started wiring diagram" src="https://cdn.shopify.com/s/files/1/0100/6632/files/PulseSensor_GettingStarted_bb_1024x1024.png?v=1511986616" width="400">
---

### Calculate BPM
  Calculate beats per minute (BPM) once you have a steady raw waveform.

- [**Project Page**](https://pulsesensor.com/pages/getting-advanced)

  <img alt="PulseSensor BPM project wiring diagram" src="https://cdn.shopify.com/s/files/1/0100/6632/files/PulseSensor_GettingAdvanced_bb_1024x1024.png?v=1511986194" width="400">
---

### Make a Sound with Your Heartbeat
  Transform the heartbeat into a live "beep" with a speaker.

- [**Project Page**](https://pulsesensor.com/pages/pulse-sensor-speaker-tutorial)

  <img alt="PulseSensor speaker project wiring diagram" src="https://cdn.shopify.com/s/files/1/0100/6632/files/PulseSensor_Speaker_bb_61a0333f-e868-4123-961d-7456a31fa928_1024x1024.png?v=1510863829" width="400">
---

### Move a Motor with Your Heartbeat
  Make a servo motor pulse to your live heartbeat.

- [**Project Page**](https://pulsesensor.com/pages/pulse-sensor-servo-tutorial)

  <img alt="PulseSensor servo project wiring diagram" src="https://cdn.shopify.com/s/files/1/0100/6632/files/PulseSensor_Servo_bb_87fce9fc-dc47-4208-b708-a7edb6df58a2_1024x1024.png?v=1510863990" width="400">
---

### Connect Two (or More) Pulse Sensors
  Use 2 or more Pulse Sensors on one Arduino.

- [**Project Page**](https://pulsesensor.com/pages/two-or-more-pulse-sensors)

  <img alt="Two PulseSensors connected to an Arduino" src="https://cdn.shopify.com/s/files/1/0100/6632/files/2_PulseSensors_bb_grande.png?v=1516733684" width="400">
---

### Processing Visualizer

  Visualize the pulse waveform and BPM from your Arduino in Processing.

- [**Project Page**](https://pulsesensor.com/pages/processing-visualization)

  <img alt="Processing pulse waveform visualizer" src="https://cdn.shopify.com/s/files/1/0100/6632/files/ScreenShot_1024x1024.png?v=1491857113" width="400">

---

### Pulse Transit Time

  Explore pulse timing differences with two PulseSensors. This is an educational experiment, not a blood-pressure measurement.

- [**Project Page**](https://pulsesensor.com/pages/pulse-transit-time)

  <img alt="Two-sensor pulse timing experiment" src="https://cdn.shopify.com/s/files/1/0100/6632/files/PulseSensor_PTT-17042_grande.jpg?v=1517336059" width="400">

---

</details>

## Developer Resources

<details>
<summary>Functions, board examples, troubleshooting, and support</summary>

### Build on the examples

- [PulseSensor Playground function guide](resources/PulseSensor%20Playground%20Tools.md)
- [All example sketches](examples/)
- [Board-specific examples and library reference](https://pulsesensor.com/pages/installing-our-playground-for-pulsesensor-arduino)

### Troubleshooting your signal

Start with the raw waveform in `GettingStartedProject`:

- **Flat signal?** Check power, ground, the analog pin, and the selected board and port.
- **Noisy signal?** Rest your hand, ease the pressure, reduce strong ambient light, and reseat the prepared sensor in the ring or strap.
- **Extra or missed beats?** Get a steady waveform first, then adjust the threshold in your example. In `GettingStartedProject`, it is `Threshold`; other examples may use `THRESHOLD` or `setThreshold()`.
- **Serial port busy?** Close other serial tools before opening Serial Plotter or Signal Coach.

[More setup help](https://pulsesensor.com/pages/start)

### Give and get feedback

For setup help, email [support@pulsesensor.com](mailto:support@pulsesensor.com) with your board, example name, wiring photo, and Serial Plotter screenshot. Report reproducible library bugs in [PulseSensorPlayground Issues](https://github.com/WorldFamousElectronics/PulseSensorPlayground/issues).

</details>

---

For educational and experimental use. PulseSensor is not a medical device.

PulseSensor.com® · World Famous Electronics LLC · [MIT License](LICENSE)
