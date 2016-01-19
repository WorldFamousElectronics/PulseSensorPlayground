# PulseSensorBPM
An Arduino library based on Joel Murphy's and Yury Gitman's Pulse Sensor Amped Sketch.

This library enables Sketches that use either Timer Interrupts or delayMicroseconds() to detect pulses.

Tested on Arduino 101 and Arduino Uno.

## To Use
1. Install [Arduino IDE](https://www.arduino.cc/en/Main/Software) and [Processing](https://processing.org/)
2. git clone this repository into your Arduino/libraries folder
2. Start the Arduino IDE
3. In the IDE, Select File / Examples / PulseSensorBPM / PulseLooper
  * Or PulseInterrupter.  PulseLooper runs on anything; PulseInterrupter uses ARM-specific interrupts.
4. Wire your circuit: Connect the Pulse Sensor signal to A0; connect an LED and 220 ohm resister in series to pin 3 (or pin 5 for PulseInterrupter).
5. Follow the rest of the instructions in [the original Processing example] (https://github.com/WorldFamousElectronics/PulseSensor_Amped_Processing_Visualizer)
  * **Note**: In the Processing code, the line "println(Serial.list());" isn't compatible with Processing 3.0.  You'll need to change that line to "printArray(Serial.list());"
  * **Note2**: The [Arduino 101 specs page](https://www.arduino.cc/en/Main/ArduinoBoard101) says the analog input is "...from ground to 3.3 volts", so experiment with powering the Pulse Sensor from 3.3V vs. 5V and see which works better for you.
