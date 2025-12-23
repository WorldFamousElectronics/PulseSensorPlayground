/*
 * PulseSensor_XIAO_ESP32S3_BPM_Monitor.ino
 * Version: 1.0.0
 * 
 * Uses library's built-in averaging for stable BPM.
 * Displays heart rate in Serial Monitor.
 * 
 * Hardware: XIAO ESP32S3, PulseSensor
 * 
 * Wiring:
 *   PulseSensor Signal (Purple) - A0 (GPIO 1)
 *   PulseSensor Power (Red)     - 3.3V
 *   PulseSensor Ground (Black)  - GND
 * 
 * >> https://pulsesensor.com/pages/pulsesensor_xiao_esp32s3
 *
 * Copyright World Famous Electronics LLC - see LICENSE
 * Contributors:
 *   Joel Murphy, https://pulsesensor.com
 *   Yury Gitman, https://pulsesensor.com
 *
 * Licensed under the MIT License, a copy of which
 * should have been included with this software.
 *
 * This software is not intended for medical use.
 */

#include <PulseSensorPlayground.h>

const int PULSE_PIN = 1;
const int LED_PIN = 21;

PulseSensorPlayground pulseSensor;

void setup() {
  Serial.begin(115200);
  Serial.println("PulseSensor_XIAO_ESP32S3_BPM_Monitor.ino");
  Serial.println("PulseSensor.com");

  pulseSensor.analogInput(PULSE_PIN);
  pulseSensor.blinkOnPulse(LED_PIN);
  pulseSensor.setThreshold(550);
  pulseSensor.begin();
}

void loop() {
  int bpm = pulseSensor.getBeatsPerMinute();

  if (pulseSensor.sawStartOfBeat()) {
    Serial.print("BPM: ");
    Serial.println(bpm);
  }
  delay(20);
}
