/*
   Code to detect pulses from the PulseSensor,
   using inline code instead of interrupts.

   You would typically use this code if the interrupt library
   doesn't yet support your Arduino model
   or you don't feel comfortable using interrrupts.

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
   The format of our output.

   Set this to PROCESSING_VISUALIZER if you're going to run
    the Processing Visualizer Sketch.
    See https://github.com/WorldFamousElectronics/PulseSensor_Amped_Processing_Visualizer

   Set this to SERIAL_PLOTTER if you're going to run
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
const unsigned long MICROS_PER_READ = (2 * 1000L);

/*
   PWM steps per fade step.  More fades faster; less fades slower.
   Because we fade once per sample rather than a slower rate,
   this value is lower than the one in the InterruptBeatDetector.
*/
const int PWM_STEPS_PER_FADE = 1;

/*
   fadePWM = Pulse Width Modulation value for our fading LED
   (0 = off; 255 = full on)
*/
int fadePWM;

// The next time, in micros() value, that we wish to call readSensor().
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

// the latest analog value we've read from the PulseSensor
int lastSampleValue;

// All the PulseSensor Playground functions.
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

  // wait one sample interval before starting to search for pulses.
  wantMicros = micros() + MICROS_PER_READ;
}

void loop() {

  /*
     Find whether it's time to read from the sensor.
     Note: because micros() overflows every 90 minutes or so,
     we must calculate differences in time via (long) (b - a).
  */
  unsigned long nowMicros = micros();
  if ((long) (wantMicros - nowMicros) < 0L) {
    wantMicros = nowMicros + MICROS_PER_READ;

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

    /*******
       This is a good place to add code that could take up
       to a millisecond or so to run.
    *******/

    // fade the LED a bit.
    fadePWM = constrain(fadePWM - PWM_STEPS_PER_FADE, 0, 255);
    analogWrite(PIN_FADE, fadePWM);
  }

  /******
     Don't add code here, because it could slow the sampling
     from the PulseSensor.
  ******/
}
