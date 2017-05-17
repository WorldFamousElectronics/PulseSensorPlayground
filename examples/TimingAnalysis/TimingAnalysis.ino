/*
   An example of how to use the Timing Statistics Playground functions.
   This Sketch is basically NonInterruptBeatDetector.ino plus
   Timing Statistics calls.

   See https://www.pulsesensor.com to get started.

   Copyright World Famous Electronics LLC - see LICENSE
   Contributors:
     Joel Murphy, https://pulsesensor.com
     Yury Gitman, https://pulsesensor.com
     Bradford Needham, @bneedhamia, https://bluepapertech.com

   Licensed under the MIT License, a copy of which
   should have been included with this software.

   This software is not intended for medical use.
*/
#include <PulseSensorPlayground.h>

/*
   Non-interrrupt code for reading from the PulseSensor
   can be sensitive to the other code you add to your loop().
   In some cases, this sensitivity can cause errors in the Beat Detector's
   measurements.

   Use the Timing Statistics functions to measure how much
   your Sketch's time-per-PulseSensor sample is affected by the rest of
   your Sketch's code.

   A little background on sample rate timing statistics:
   Whenever you want to do something at a consistent rate, there are two
   engineering measures of variations in that rate:

   1) Jitter. Jitter describes how much the timing varies from one sample
   to the next. In this example, we describe Jitter by two statistics:
   the minimum and maximum difference between the desired time between
   samples, and the actual time between samples. By definition, the average
   Jitter is 0.

   2) Offset. Offset describes the *average* difference between the
   desired time between samples and the actual time between samples.
   In other words, Offset says how much slower or faster your sample rate
   is than your desired sample rate.


   Once you've measured Jitter and Offset, you can correct for them.
   If the Offset is larger than you'd like, subtract the Offset from the
   desired time between samples. That should reduce the measured offset.
   If the Jitter is larger than you'd like, that suggests that there is
   something in your code that sometimes (but not always) takes longer
   than you can afford. The solution here is to look for places in your
   code that can sometimes take extra time, and rewrite them to be more
   consistent or to not take so much time.

   So you're probably asking "how much is too much?"
   That depends on how accurate your Pulse Sensor signal needs to be.
   If you don't care about measuring a slightly high or low beats-per-minute
   or inter-beat interval, you can probably ignore timing statistics.
   If instead, you care that your beats per minute is a few percent higher
   or lower than an accurate Pulse Oximeter would measure, you probably
   want to run Timing Statistics on your sketch.

   For example, suppose your Sketch is doing an analogRead() from
   from a PulseSensor every 2000 microseconds (2 milliseconds,
   or 500 samples per second). Suppose that your Timing Statistics
   report an offset of 60 microseconds. 60 microseconds is 3% of 2000
   (60/2000), so your Sketch's beats-per-minute and inter-beat interval
   will be 3% lower than they should be.

   Suppose that after adding Offset correction into your Sketch, the
   Timing Statistics report an offset of 10 microseconds. 10 microseconds
   is 1/2 of 1% of 2000 (10/2000), which is very good for a non-medical-grade
   pulse measurement.
*/


/*
   During code development:
   If you want to see the accuracy of sample timing, set this to
   the number of samples to collect before reporting and stopping.
   Also look at the output only with the Arduino Serial Monitor
   rather than with the Processing Visualizer or Serial Plotter.

   500 * 30 = 500 samples per second * 30 seconds.

   Normally, set this value to 0L, to turn off those measures.
*/

#define SAMPLES_BEFORE_STOPPING (500 * 30L)

/*
   The format of our output.

   Set this to PROCESSING_VISUALIZER if you're normally going to run
    the Processing Visualizer Sketch.
    See https://github.com/WorldFamousElectronics/PulseSensor_Amped_Processing_Visualizer

   Set this to SERIAL_PLOTTER if you're normally going to run
    the Arduino IDE's Serial Plotter.
*/
const int OUTPUT_TYPE = PROCESSING_VISUALIZER;

/*
   Pinout:
     HAS_A_REF = set to true if you have connected
       the pulse sensor power pin to A-REF.
       If you are powering the pulse sensor below 5V,
       you must set HAS_A_REF to true.
     PIN_INPUT = Analog Input. Connected to the pulse sensor
      purple (signal) wire.
     PIN_BLINK = digital Output. Connected to an LED (and 220 ohm resistor)
      that will flash on each detected pulse.
     PIN_FADE = digital Output. Connected to an LED (and resistor)
      that will smoothly fade with each pulse.
*/
const boolean HAS_A_REF = false;
const int PIN_INPUT = A0;
const int PIN_BLINK = 13;    // Pin 13 is the on-board LED
const int PIN_FADE = 5;      // must be a pin that supports PWM. Can't be pin 3 or 11 (see ISR()).

/*
   Planned time (microseconds) between calls to readSensor().
   2000 microseconds = 500 samples per second, that is 1000000/2000.
*/
const unsigned long MICROS_PER_READ = 2 * 1000L;

/*
   The offset compensation (microseconds) to subtract from
   MICROS_PER_READ so that the measured offset is close to zero.

   For some slow or older Arduinos, this value may need to be
   as high as 30.
*/
const int OFFSET_MICROS = 0;

/*
   PWM steps per fade step.  More fades faster; less fades slower.
   Because we fade once per sample rather than a slower rate,
   this value is lower than the one in the InterruptBeatDetector.
*/
const int PWM_STEPS_PER_FADE = 2;

/*
   fadePWM = Pulse Width Modulation value for our fading LED
   (0 = off; 255 = full on)
*/
int fadePWM;

/*
   The next time, in micros() value, that we wish to call readSensor().
*/
unsigned long wantMicros;

/*
   samplesUntilReport = the number of readSensor() calls remaining to perform
   until we want to report a sample over the serial connection.

   We want to report a sample value over the serial port
   only once every 20ms to avoid doing Serial output
   faster than the Arduino can do.
*/
byte samplesUntilReport;
const byte SAMPLES_PER_SERIAL_SAMPLE = 20;

/*
   the latest analog value we've read from the PulseSensor
*/
int lastSampleValue;

/*
   All the PulseSensor Playground functions.
*/
PulseSensorPlayground pulseSensor;

void setup() {
  /*
     Use 115200 baud because that's what the Processing Sketch expects to read,
     and because that speed provides about 11 bytes per millisecond.

     If we used a slower baud rate, we'd likely write bytes faster than
     they can be transmitted, which would mess up the timing
     of readSensor() calls, which would make the pulse measurement
     not work properly.
  */
  Serial.begin(115200);
  pulseSensor.beginSerialOutput(OUTPUT_TYPE);

  // Set up the I/O pins

#if !defined (__arc__) // EXTERNAL causes a compile error on Arduino 101.
  if (HAS_A_REF) {
    analogReference(EXTERNAL);
  }
#endif

  // no setup is necessary for PIN_INPUT.
  pinMode(PIN_BLINK, OUTPUT);
  digitalWrite(PIN_BLINK, LOW);
  pinMode(PIN_FADE, OUTPUT);
  fadePWM = 0;
  analogWrite(PIN_FADE, fadePWM);   // sets PWM duty cycle

  // Setup our pulse detector
  pulseSensor.beginBeatDetection();
  pulseSensor.setBeatSampleIntervalMs(MICROS_PER_READ / 1000L);

#if SAMPLES_BEFORE_STOPPING > 0
  // Set up our timing statistics.
  pulseSensor.beginTimingStatistics(MICROS_PER_READ, SAMPLES_BEFORE_STOPPING);
#endif

  // wait one sample interval before starting to search for pulses.
  wantMicros = micros() + MICROS_PER_READ - OFFSET_MICROS;
}

void loop() {

  /*
     Find whether it's time to read from the sensor.
     Note: because micros() overflows every 90 minutes or so,
     we must calculate differences in time via (long) (b - a).
  */
  unsigned long nowMicros = micros();
  if ((long) (wantMicros - nowMicros) < 0L) {
    wantMicros = nowMicros + MICROS_PER_READ - OFFSET_MICROS;

#if SAMPLES_BEFORE_STOPPING > 0
    /*
       Put this code immediately before your analogRead() call.

       Update the sample timing statistics.
       If enough samples have been collected, report results and stop.
    */
    if (pulseSensor.recordSampleTime() <= 0) {
      pulseSensor.outputStatistics();
      for (;;); // hang
    }
#endif

    // Read from the PulseSensor and give that value to the Beat Detector.
    lastSampleValue = analogRead(PIN_INPUT);
    boolean QS = pulseSensor.addBeatValue(lastSampleValue);

    /*
       Every so often, send the latest Sample.
       We don't print every sample, because our baud rate
       won't support that much I/O.
    */
    if (--samplesUntilReport == (byte) 0) {
      samplesUntilReport = SAMPLES_PER_SERIAL_SAMPLE;

      pulseSensor.output(lastSampleValue
                         , pulseSensor.getBeatsPerMinute()
                         , pulseSensor.getInterBeatIntervalMs());
    }

    // Blink the non-fading LED when the start of a pulse is detected.
    if (pulseSensor.isBeat()) {
      digitalWrite(PIN_BLINK, HIGH);
    } else {
      digitalWrite(PIN_BLINK, LOW);
    }

    // Every beat, report the heart rate and inter-beat-interval
    if (QS) {
      fadePWM = 255;  // start fading on the start of each beat.

      pulseSensor.outputBeat(
        pulseSensor.getBeatsPerMinute()
        , pulseSensor.getInterBeatIntervalMs());
    }

    /*
       This is a good place to add code that could take up
       to a millisecond or so to run.
    */

    // fade the LED a bit.
    fadePWM = constrain(fadePWM - PWM_STEPS_PER_FADE, 0, 255);
    analogWrite(PIN_FADE, fadePWM);
  }

  /*
     Don't add code here, because it could slow the sampling
     from the PulseSensor.
  */
}
