/*
 * Code to detect pulses from a pulse Oximeter sensor,
 * using inline code instead of interrupts.
 *
 * Example Pulse Sensors:
 *   Pulse Sensor Amped (https://www.pulsesensor.com)
 *     available through SparkFun, Adafruit, and others.
 *   
 * This code has been tested with the Pulse Sensor Amped
 * and Arduino 101 and Arduino Uno.
 * 
 * Copyright (c) 2016, 2017 Bradford Needham, North Plains, Oregon, USA
 * @bneedhamia, https://bluepapertech.com
 * Licensed under the MIT License, a copy of which
 * should have been included with this software.
 * 
 * This software is not intended for medical use.
 */

#include <PulseSensorAmpedObject.h>

/*
 * Pinout:
 *   HAS_A_REF = set to true if you have connected
 *     the pulse sensor power pin to A-REF.
 *     If you are powering the pulse sensor below 5V,
 *     you must set HAS_A_REF to true.
 *   PIN_INPUT = Analog Input. Connected to the pulse sensor
 *    purple (signal) wire.
 *   PIN_BLINK = digital Output. Connected to an LED (and 220 ohm resistor)
 *    that will flash on each detected pulse.
 *   PIN_FADE = digital Output. Connected to an LED (and resistor)
 *    that will smoothly fade with each pulse.
 */
const boolean HAS_A_REF = false; //BUG? analogReference(EXTERNAL) causes a compile error on Arduino 101.
const int PIN_INPUT = A0;
const int PIN_BLINK = 13;        // Pin 13 is the on-board LED
const int PIN_FADE = 3;          // must be a pin that supports PWM.

/*
 * Planned time (microseconds) between calls to readSensor().
 * Must be evenly divisible by 1000L.
 * I believe 500 samples per second (2000 microseconds per sample)
 * is the fasted rate (smallest MICROS_PER_READ) we can achieve.
 */
const unsigned long MICROS_PER_READ = 2 * 1000L;

/*
 * REPORT_JITTER_AND_HANG = normally false.
 * Set this to true only if you want to see the jitter calculation
 * in the Arduino IDE Serial Monitor.
 * NOTE: If you set this to true, this app won't work with
 * the Processing Sketch because it will report the measured jitter
 * and hang after running for 60 seconds.
 * 
 * Normally you'll set this to false and run a Processing Sketch
 * to read the results output by this Sketch.
 * 
 * Set this to true only during development,
 * when you want to double-check that the calculations are not taking
 * too much time, causing the Sketch to delay analogRead().
 * I (Brad) have tested this on an Arduino 101 and Arduino UNO.
 * On one Arduino 101, maximum jitter range was 1 microsecond;
 * On one Arduino UNO, maximum jitter range was 64 microseconds.
 * 
 * Centering jitter is important because if the average jitter is non-zero,
 * that means that the code is sampling at a consistently faster (or slower)
 * rate than you want.
 */
const boolean REPORT_JITTER_AND_HANG = false;

/*
 * OFFSET_MICROS = amount to add to our delaymicroseconds() calls
 *  in order to center the jitter (delay past our desired analogRead() time)
 *  around zero.
 *  Calibrate this offset by setting REPORT_JITTER_AND_HANG to true
 *  and observing the jitter that is output after 60 seconds.
 *  
 *  For one Arduino Uno I observed an OFFSET_MICROS of 20;
 *  For one Arduino 101, I observed a value of 1;
 */
const long OFFSET_MICROS = 1L;  // NOTE: must be non-negative

/*
 * Variables around timing of readSensor():
 *
 * wantMicros = the next time, in micros() value,
 *   that we wish to call readSensor().
 * minJitterMicros = the minimum jitter (delay in analogRead()) observed so far.
 * maxJitterMicros = the maximum jitter observed so far.
 */
unsigned long wantMicros;
long minJitterMicros;
long maxJitterMicros;

// time (value of micros()) when we last reported jitter.
unsigned long lastReportMicros;

/*
 * samplesUntilReport = the number of readSensor() calls remaining to perform
 * until we want to report a sample over the serial connection.
 * 
 * Like the original Pulse Sensor Sketch, we want to report a sample value
 * over the serial port only once every 20ms to avoid doing Serial output
 * faster than the Arduino can do.
 */
byte samplesUntilReport;
const byte SAMPLES_PER_SERIAL_SAMPLE = 20;

// PWM steps per fade step.  More fades faster; less fades slower.
const int PWM_STEPS_PER_FADE = 12;

/* 
 * fadePWM = Pulse Width Modulation value for our fading LED
 * (0 = off; 255 = full on)
 */
int fadePWM;

/*
 * The per-sample processing code.
 */
PulseSensorAmpedObject pulseDetector(PIN_INPUT, MICROS_PER_READ / 1000L);

void setup() {
  /*
   * Use 115200 baud because that's what the Processing Sketch expects to read,
   * and because that speed provides about 11 bytes per millisecond.
   * 
   * If we used a slower baud rate, we'd likely write bytes faster than
   * they can be transmitted, which would mess up the timing
   * of readSensor() calls, which would make the pulse measurement
   * not work properly.
   */
  Serial.begin(115200);

  // Set up the I/O pins
  
  if (HAS_A_REF) {
    //BUG? Causes a compile error on Arduino 101: analogReference(EXTERNAL);
  }
  // PIN_INPUT is set up by the pulseDetector constructor.
  pinMode(PIN_BLINK, OUTPUT);
  digitalWrite(PIN_BLINK, LOW);
  pinMode(PIN_FADE, OUTPUT);
  fadePWM = 0;
  analogWrite(PIN_FADE, fadePWM);   // sets PWM duty cycle

  // Setup our reporting and jitter measurement.
  samplesUntilReport = SAMPLES_PER_SERIAL_SAMPLE;
  lastReportMicros = 0L;
  resetJitter();

  // wait one sample interval before starting to search for pulses.
  wantMicros = micros() + MICROS_PER_READ;
}

void loop() {
  /*
   * Find how long we have to wait until the next readSensor() call.
   * Note: because micros() overflows every 90 minutes or so,
   * we must calculate differences in time via (long) (a - b).
   */
  unsigned long nowMicros = micros();
  if ((long) (wantMicros - nowMicros) > 1000L) {
    return;  // we have time to do other things
  }

  /*
   * Wait the desired time before doing an analogRead().
   * delayMicroseconds() is documented to not work for values < 3 microseconds.
   * We include OFFSET_MICROS so that our timing error (jitter)
   * is centered around zero.
   */
  if ((long) (wantMicros - nowMicros) > 3L + OFFSET_MICROS) {
    delayMicroseconds((unsigned int) (wantMicros - nowMicros) - OFFSET_MICROS);
    nowMicros = micros();    
  }

  /*
   * Calculate our jitter (how late we are compared to
   *   when we wanted to call readSensor().
   * jitter = how late we are relative to our desired time.
   *   Positive values indicate sampling late;
   *   Negative values indicate sampling early.
   */
  long jitterMicros = (long) (nowMicros - wantMicros);
  if (minJitterMicros > jitterMicros) {
    minJitterMicros = jitterMicros;
  }
  if (maxJitterMicros < jitterMicros) {
    maxJitterMicros = jitterMicros;
  }

  /*
   * If desired, after 60 seconds of running,
   * report our measured Jitter and hang.
   * 
   * NOTE: this mode won't work with the Processing Sketch.
   * It's designed for debug only.
   */
  if (REPORT_JITTER_AND_HANG
      && (long) (nowMicros - lastReportMicros) > 60000000L) {
    lastReportMicros = nowMicros;
    
    Serial.print(F("Jitter (min, max) = "));
    Serial.print(minJitterMicros);
    Serial.print(F(", "));
    Serial.print(maxJitterMicros);
    Serial.println();
    
    resetJitter();

    //hang because our prints are incompatible with the Processing Sketch
    for (;;) { }
  }
  
  wantMicros = nowMicros + MICROS_PER_READ;
  boolean QS = pulseDetector.readSensor();

  if (pulseDetector.isPulse()) {
    digitalWrite(PIN_BLINK, HIGH);
  } else {
    digitalWrite(PIN_BLINK, LOW);
  }

  if (QS) {
    fadePWM = 255;  // start fading on the start of each beat.
    analogWrite(PIN_FADE, fadePWM);
  }


  /*
   * Perform our Serial output. We don't worry about timing, because
   * the documentation for Serial says that "As of version 1.0,
   * serial transmission is asynchronous; Serial.print() will return
   * before any characters are transmitted."
   * 
   * The reader (the Processing Sketch) must read continuously
   * or else our app will block (stop temporarily).
   */

  /*
   * Every so often, send the latest Sample to the Processing Sketch.
   * We don't print every sample, because our baud rate
   * won't support that much I/O.
   */
  if (--samplesUntilReport == (byte) 0) {
    samplesUntilReport = SAMPLES_PER_SERIAL_SAMPLE;

    Serial.print('S');
    Serial.println(pulseDetector.getSignal());

    // Coincidentally, fade the LED a bit.
    fadePWM -= PWM_STEPS_PER_FADE;
    if (fadePWM < 0) {
      fadePWM = 0;
    }
    analogWrite(PIN_FADE, fadePWM);
    
  }

  // Every beat, report the heart rate and inter-beat-interval
  if (QS) {
    Serial.print('B');
    Serial.println(pulseDetector.getBPM());
    Serial.print('Q');
    Serial.println(pulseDetector.getIBI());
  }

}

/*
 * Resets our measurements of analogRead() timing jitter (delay).
 */
void resetJitter() {
  // min = a number so large that any value will be smaller than it;
  // max = a number so small that any value will be larger than it.
  minJitterMicros = 60 * 1000L;
  maxJitterMicros = -1;
}

