/*
  PulseSensor Signal Coach — minimal Web Serial sender

  Sends one raw analog sample per line at 50 samples/second. Chrome owns beat
  detection, IBI, BPM, confidence, coaching states, and Re-sync.

  Wiring:
    PulseSensor purple -> PULSE_PIN
    PulseSensor red    -> 3.3V or 5V supported by your board/sensor setup
    PulseSensor black  -> GND

  Common PULSE_PIN values:
    Arduino Uno / Nano / Mega: A0
    XIAO ESP32-S3:             A0 (GPIO 1)
    ESP32 DevKit:              34

  Use 115200 baud and choose 50 samples/s on the Signal Coach page.

  World Famous Electronics LLC
  MIT License — see ../../LICENSE
*/

const int PULSE_PIN = A0;                  // Change if the purple wire uses another analog pin.
const unsigned long SAMPLE_PERIOD_MS = 20; // 1000 ms / 20 ms = 50 samples/second.

unsigned long nextSampleAt = 0;

void setup() {
  Serial.begin(115200);
  delay(1000);
}

void loop() {
  if ((long)(millis() - nextSampleAt) < 0) return;
  nextSampleAt += SAMPLE_PERIOD_MS;

  Serial.println(analogRead(PULSE_PIN));
}
