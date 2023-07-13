/*
   Code to detect pulses from the PulseSensor,
   using an interrupt service routine.

   This code is designed to target an RP2040 microcontroller
   You may need to ajust the number in the RPI_PICO_Timer parameter, maybe?
   If you have trouble, please post an issue on github
   
   Mostly works. Tested on:
        Adafruit Feather RP2040
        Raspberry Pi Pico H
      

   Copyright World Famous Electronics LLC - see LICENSE
   Contributors:
     Joel Murphy, https://pulsesensor.com
     Yury Gitman, https://pulsesensor.com
     Bradford Needham, @bneedhamia, https://bluepapertech.com

   Licensed under the MIT License, a copy of which
   should have been included with this software.

   This software is not intended for medical use.
*/


/*
 *    The TimerInterrupt library 
 *    https://github.com/khoih-prog/RPI_PICO_TimerInterrupt
 *    Set the sample rate to 500Hz
 *    Keep the name sampleTimer! Don't change it, cause it's used in the library!
 */

#include "RPi_Pico_TimerInterrupt.h"
#define SAMPLE_INTERVAL_US 2000L
RPI_PICO_Timer sampleTimer(0); // the paramater may need to change, depending?

/*
   Every Sketch that uses the PulseSensor Playground must
   define USE_ARDUINO_INTERRUPTS before including PulseSensorPlayground.h.
   Here, #define USE_ARDUINO_INTERRUPTS true tells the library to use
   interrupts to automatically read and process PulseSensor data.

   See PulseSensor_BPM_Alternative.ino for an example of not using interrupts.
*/
#define USE_ARDUINO_INTERRUPTS true
#include <PulseSensorPlayground.h>

/*
 *  Declare the interrupt service routine
 *  This will be used in setup as the interrupt callback
 */
boolean sampleTimer_ISR(struct repeating_timer *t){ 
  (void) t;
  PulseSensorPlayground::OurThis->onSampleTime();
  return true;
}

/*
   The format of our output.

   Set this to PROCESSING_VISUALIZER if you're going to run
    the Processing Visualizer Sketch.
    See https://github.com/WorldFamousElectronics/PulseSensor_Amped_Processing_Visualizer

   Set this to SERIAL_PLOTTER if you're going to run
    the Arduino IDE's Serial Plotter.
*/
const int OUTPUT_TYPE = SERIAL_PLOTTER;

/*
   Pinout:
     PULSE_INPUT = Analog Input. Connected to the pulse sensor
      purple (signal) wire.
     PULSE_BLINK = digital Output. Connected to an LED (and 1K series resistor)
      that will flash on each detected pulse.
     PULSE_FADE = digital Output. PWM pin onnected to an LED (and 1K series resistor)
      that will smoothly fade with each pulse.
      NOTE: PULSE_FADE must be a pin that supports PWM. Do not use
      pin 9 or 10, because those pins' PWM interferes with the sample timer.
     THRESHOLD should be set higher than the PulseSensor signal idles
      at when there is nothing touching it. The expected idle value
      should be 512, which is 1/2 of the ADC range. To check the idle value
      open a serial monitor and make note of the PulseSensor signal values
      with nothing touching the sensor. THRESHOLD should be a value higher
      than the range of idle noise by 25 to 50 or so. When the library
      is finding heartbeats, the value is adjusted based on the pulse signal
      waveform. THRESHOLD sets the default when there is no pulse present.
      Adjust as neccesary.
*/
const int PULSE_INPUT = A0;
const int PULSE_BLINK = LED_BUILTIN;
const int PULSE_FADE = 7;
const int THRESHOLD = 550;   // Adjust this number to avoid noise when idle

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
//  Stream &myPort = (Stream &)Serial;
  // Configure the PulseSensor manager.

  pulseSensor.analogInput(PULSE_INPUT);
  pulseSensor.blinkOnPulse(PULSE_BLINK);
  pulseSensor.fadeOnPulse(PULSE_FADE);

  pulseSensor.setSerial(Serial);
  pulseSensor.setOutputType(OUTPUT_TYPE);
  pulseSensor.setThreshold(THRESHOLD);

  analogReadResolution(10);

  // Now that everything is ready, start reading the PulseSensor signal.
  if (!pulseSensor.begin()) {
    /*
       PulseSensor initialization failed,
       likely because our particular Arduino platform interrupts
       aren't supported yet.

       If your Sketch hangs here, try PulseSensor_BPM_Alternative.ino,
       which doesn't use interrupts.
    */
    for(;;) {
      // Flash the led to show things didn't work.
      digitalWrite(PULSE_BLINK, LOW);
      delay(50);
      Serial.println('!');
      digitalWrite(PULSE_BLINK, HIGH);
      delay(50);
    }
  }
  /*  This starts the sample timer interrupt
   *  Use pause() and resume() to start and stop sampling on the fly
   *  Check Resources folder in the library for more tools
   */
  if (!sampleTimer.attachInterruptInterval(SAMPLE_INTERVAL_US, sampleTimer_ISR)){
    Serial.println("Can't set ISR! Select another timer?");
  }
  
}  


void loop() {
  /*
     Wait a bit.
     We don't output every sample, because our baud rate
     won't support that much I/O.
  */
  delay(20);

  // write the latest sample to Serial.
 pulseSensor.outputSample();

  /*
     If a beat has happened since we last checked,
     write the per-beat information to Serial.
   */
  if (pulseSensor.sawStartOfBeat()) {
   pulseSensor.outputBeat();
  }
}
