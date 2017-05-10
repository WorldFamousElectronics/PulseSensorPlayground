# [PulseSensor](https://pulsesensor.com) Playground
An Arduino library based on Joel Murphy's and Yury Gitman's Pulse Sensor Amped Sketches.

See [https://pulsesensor.com](https://pulsesensor.com)

This library shows how to use the PulseSensor, using either Arduino timer Interrupts or delayMicroseconds() to detect pulses.

This software is not intended for medical use.

## To Use
1. Install [Arduino IDE](https://www.arduino.cc/en/Main/Software) and [Processing](https://processing.org/)
2. Download the .ZIP file of this library.
2. Start the Arduino IDE
3. In the IDE, Select Sketch / Include Library / Add .ZIP Library...
4. In the resulting dialog, navigate to the downloaded .ZIP of this library.
5. Follow the instructions to install this library.
6. Once this library is installed...
3. In the IDE, Select one of the examples under File / Examples / PulseSensor Playground.
4. Wire your circuit: Connect the Pulse Sensor signal to the Arduino's A0; connect an LED and 220 ohm resister in series between pin 3
   (or pin 5 for InterruptBeatDetector) and ground (GND).
5. Follow the rest of the instructions in [the original Processing example](https://github.com/WorldFamousElectronics/PulseSensor_Amped_Processing_Visualizer)
  * **Note**: In the Processing code, the line "println(Serial.list());" isn't compatible with Processing 3.0.  You'll need to change that line to "printArray(Serial.list());"
  * **Note2**: The [Arduino 101 specs page](https://www.arduino.cc/en/Main/ArduinoBoard101) says the analog input is "...from ground to 3.3 volts", so experiment with powering the PulseSensor from 3.3V vs. 5V and see which works better for you.
