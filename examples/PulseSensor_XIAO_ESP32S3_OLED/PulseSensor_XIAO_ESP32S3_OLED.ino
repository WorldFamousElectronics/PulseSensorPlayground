/*
 * PulseSensor_XIAO_ESP32S3_OLED_Lib.ino
 * Version: 1.0.0
 * 
 * OLED display with library-powered beat detection.
 * Shows waveform and BPM on 1.3" SH1106 OLED.
 * 
 * Hardware: XIAO ESP32S3, PulseSensor, 1.3" SH1106 OLED
 * 
 * Wiring:
 *   PulseSensor Signal (Purple) - A0 (GPIO 1)
 *   PulseSensor Power (Red)     - 3.3V
 *   PulseSensor Ground (Black)  - GND
 *   OLED SDA                    - D4 (GPIO 5)
 *   OLED SCL                    - D5 (GPIO 6)
 *   OLED VCC                    - 3.3V
 *   OLED GND                    - GND
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
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SH110X.h>

const int PULSE_PIN = 1;
const int LED_PIN = 21;

PulseSensorPlayground pulseSensor;
Adafruit_SH1106G display(128, 64, &Wire, -1);

// Waveform buffer
#define WAVE_WIDTH 100
int waveBuffer[WAVE_WIDTH];
int waveIndex = 0;

void setup() {
  Serial.begin(115200);
  Serial.println("PulseSensor_XIAO_ESP32S3_OLED_Lib.ino");
  Serial.println("PulseSensor.com");

  // Initialize PulseSensor
  pulseSensor.analogInput(PULSE_PIN);
  pulseSensor.blinkOnPulse(LED_PIN);
  pulseSensor.setThreshold(550);
  pulseSensor.begin();

  // Initialize OLED
  Wire.begin(5, 6);
  display.begin(0x3C, true);
  display.setTextColor(SH110X_WHITE);

  // Splash screen
  display.clearDisplay();
  display.setTextSize(1);
  display.setCursor(20, 20);
  display.println("PulseSensor");
  display.setCursor(20, 35);
  display.println("OLED + Library");
  display.display();
  delay(2000);

  // Initialize waveform buffer
  for (int i = 0; i < WAVE_WIDTH; i++) {
    waveBuffer[i] = 32;
  }
}

void loop() {
  // Get data from library
  int signal = pulseSensor.getLatestSample();
  int bpm = pulseSensor.getBeatsPerMinute();
  bool beat = pulseSensor.sawStartOfBeat();

  // Update waveform buffer
  int y = map(signal, 0, 4095, 63, 20);
  y = constrain(y, 20, 63);
  waveBuffer[waveIndex] = y;
  waveIndex = (waveIndex + 1) % WAVE_WIDTH;

  // Draw display
  display.clearDisplay();

  // Header
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.print("BPM: ");
  if (bpm > 40 && bpm < 200) {
    display.print(bpm);
  } else {
    display.print("--");
  }

  // Heart icon on beat
  if (beat) {
    display.setCursor(60, 0);
    display.print("*");
  }

  // Runtime
  unsigned long secs = millis() / 1000;
  unsigned long mins = secs / 60;
  secs = secs % 60;
  display.setCursor(90, 0);
  if (mins < 10) display.print("0");
  display.print(mins);
  display.print(":");
  if (secs < 10) display.print("0");
  display.print(secs);

  // Draw waveform
  for (int i = 0; i < WAVE_WIDTH - 1; i++) {
    int idx = (waveIndex + i) % WAVE_WIDTH;
    int idxNext = (waveIndex + i + 1) % WAVE_WIDTH;
    display.drawLine(14 + i, waveBuffer[idx], 14 + i + 1, waveBuffer[idxNext], SH110X_WHITE);
  }

  display.display();
  delay(20);
}
