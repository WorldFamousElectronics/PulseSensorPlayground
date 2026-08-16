/*
  PulseSensor Signal Coach — dual-channel PTT sender

  Sends two raw PulseSensor readings plus the board sample timestamp. Signal
  Coach performs threshold crossing, signal-quality checks, beat pairing, and
  Pulse Transit Time (PTT) measurement in the browser.

  Wiring on Arduino Uno/Nano/Mega:
    Proximal PulseSensor purple -> A0 (earlobe recommended)
    Distal PulseSensor purple   -> A1 (fingertip recommended)
    Both red wires              -> 5V
    Both black wires            -> GND

  Open Signal Coach, choose "Two sensors / PTT", then connect at 250000 baud.
  Educational experiment only. Not a medical device or blood-pressure tool.

  World Famous Electronics LLC
  MIT License — see ../../LICENSE
*/

const int PROXIMAL_PIN = A0;
const int DISTAL_PIN = A1;
const unsigned long SAMPLE_PERIOD_US = 2000; // 500 samples/second.

unsigned long nextSampleAt = 0;

void setup() {
  Serial.begin(250000);
  delay(1000);
  nextSampleAt = micros();
}

void loop() {
  const unsigned long now = micros();
  if ((long)(now - nextSampleAt) < 0) return;
  nextSampleAt += SAMPLE_PERIOD_US;

  const int proximal = analogRead(PROXIMAL_PIN);
  const int distal = analogRead(DISTAL_PIN);

  Serial.print("PTT1,");
  Serial.print(now);
  Serial.print(',');
  Serial.print(proximal);
  Serial.print(',');
  Serial.println(distal);
}
