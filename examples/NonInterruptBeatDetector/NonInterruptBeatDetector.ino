/*
 * Code to detect pulses from the PulseSensor,
 * using inline code instead of interrupts.
 *
 * See https://pulsesensor.com
 * 
 * Portions Copyright (c) 2016, 2017 Bradford Needham, North Plains, Oregon, USA
 * @bneedhamia, https://bluepapertech.com
 *
 * Licensed under the MIT License, a copy of which
 * should have been included with this software.
 * 
 * This software is not intended for medical use.
 */

#include <PulseSensorPlayground.h>

/*
 * The format of our output.
 *
 * Set this to PROCESSING_VISUALIZER if you're going to run
 *  the Processing Visualizer Sketch.
 *  See https://github.com/WorldFamousElectronics/PulseSensor_Amped_Processing_Visualizer
 * 
 * Set this to SERIAL_PLOTTER if you're going to run
 *  the Arduino IDE's Serial Plotter.
 */
const int OUTPUT_TYPE = PROCESSING_VISUALIZER;

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
 */
const boolean HAS_A_REF = false;
const int PIN_INPUT = A0;
const int PIN_BLINK = 13;        // Pin 13 is the on-board LED

/*
 * Planned time (microseconds) between calls to readSensor().
 * 2000 microseconds = 500 samples per second, that is 1000000/2000.
 */
const unsigned long MICROS_PER_READ = 2 * 1000L;

/*
 * The next time, in micros() value, that we wish to call readSensor().
 */
unsigned long wantMicros;

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

/*
 * the latest analog value we've read from the PulseSensor
 */
int lastSampleValue;

/*
 * All the PulseSensor Playground functions.
 */
PulseSensorPlayground pulseSensor;

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
  
  // Setup our pulse detector
  pulseSensor.beginBeatDetection();
  pulseSensor.setBeatSampleIntervalMs(MICROS_PER_READ / 1000L);

  // wait one sample interval before starting to search for pulses.
  wantMicros = micros() + MICROS_PER_READ;
}

void loop() {

  /*
   * Find whether it's time to read from the sensor.
   * Note: because micros() overflows every 90 minutes or so,
   * we must calculate differences in time via (long) (a - b).
   */
  unsigned long nowMicros = micros();
  if ((long) (wantMicros - nowMicros) < 0L) {
    wantMicros = nowMicros + MICROS_PER_READ;
    
    // Read from the PulseSensor and give that value to the Beat Detector.
    lastSampleValue = analogRead(PIN_INPUT);
    boolean QS = pulseSensor.addBeatValue(lastSampleValue);

    // Light the LED if we're in a pulse.
    if (pulseSensor.isBeat()) {
      digitalWrite(PIN_BLINK, HIGH);
    } else {
      digitalWrite(PIN_BLINK, LOW);
    }

    /*
     * Every so often, send the latest Sample.
     * We don't print every sample, because our baud rate
     * won't support that much I/O.
     */
    if (--samplesUntilReport == (byte) 0) {
      samplesUntilReport = SAMPLES_PER_SERIAL_SAMPLE;

      pulseSensor.output(lastSampleValue
        , pulseSensor.getBeatsPerMinute()
        , pulseSensor.getInterBeatIntervalMs());
    }

    // Every beat, report the heart rate and inter-beat-interval
    if (QS) {
      pulseSensor.outputBeat(
        pulseSensor.getBeatsPerMinute()
        , pulseSensor.getInterBeatIntervalMs());
    }

    /*
     * This is a good place to add code that could take up
     * to a millisecond or so to run, but doesn't have to be run
     * every time through loop().
     */
  }
}
