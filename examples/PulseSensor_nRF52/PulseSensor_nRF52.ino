/*
   Code to detect pulses from the PulseSensor,
   using an interrupt service routine.

   This example is made to target boards in the nRF52 family.
   Install the dependent library. Go to Sketch > Include Library > Mange Libraries.
   When the Library Manager loads, search for NRF52_TimerInterrupt, if you are using 
   an Adafruit or Seeed nRF52 platform. 
   If you are using MBED, like the Nano 33 BLE, search for NRF52_MBED_TimerInterrupt.

   Install the latest version.

   This is a prototype version of PulseSensor_nRF52.ino use at your own risk.

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
    If you are using an Adafruit or Seeed nRF52 platform,
    uncomment the next line to use the NRF52TimerInterrupt library
*/
#include "NRF52TimerInterrupt.h"

/*
    If you are using an MBED nRF52 platform, like Nano 33 BLE,
    uncomment the next lines to use the NRF52_MBED_TimerInterrupt library
*/
// #include <NRF52_MBED_TimerInterrupt.h>
// #include <NRF52_MBED_TimerInterrupt.hpp>
// #include <NRF52_MBED_ISR_Timer.h>
// #include <NRF52_MBED_ISR_Timer.hpp>

#define TIMER3_INTERVAL_US        2000 // critical fine tuning here!

/*
    If you are using an Adafruit or Seeed nRF52 platform,
    uncomment the next line to use the NRF52TimerInterrupt library
*/
NRF52Timer sampleTimer(NRF_TIMER_3);

/*
    If you are using an MBED nRF52 platform, like Nano 33 BLE,
    uncomment the next lines to use the NRF52_MBED_TimerInterrupt library
*/
// NRF52_MBED_Timer sampleTimer(NRF_TIMER_3);

/*
   Every Sketch that uses the PulseSensor Playground must
   define USE_ARDUINO_INTERRUPTS before including PulseSensorPlayground.h.
   Here, #define USE_ARDUINO_INTERRUPTS true tells the library to use
   interrupts to automatically read and process PulseSensor data.

   See ProcessEverySample.ino for an example of not using interrupts.
*/
#define USE_ARDUINO_INTERRUPTS true
#include <PulseSensorPlayground.h>

void Timer3_ISR(){
  PulseSensorPlayground::OurThis->onSampleTime();
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
const int PULSE_BLINK = 13;
const int PULSE_FADE = 12;
const int THRESHOLD = 550;   // Adjust this number to avoid noise when idle

/*
   All the PulseSensor Playground functions.
*/
PulseSensorPlayground pulseSensor;

void setup() {
  /*
     115200 provides about 11 bytes per millisecond.
     If we used a slower baud rate, we'd likely write bytes faster than
     they can be transmitted,.
  */
  Serial.begin(115200);
  while (!Serial && millis() < 5000);

  // Configure the PulseSensor manager.

  pulseSensor.analogInput(PULSE_INPUT);
  pulseSensor.blinkOnPulse(PULSE_BLINK);
  pulseSensor.fadeOnPulse(PULSE_FADE);
  pulseSensor.setThreshold(THRESHOLD);

  pulseSensor.setSerial(Serial);
  pulseSensor.setOutputType(OUTPUT_TYPE);

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
      digitalWrite(PULSE_BLINK, HIGH);
      delay(50);
    }
  }
  if (sampleTimer.attachInterruptInterval(TIMER3_INTERVAL_US, Timer3_ISR)){
    Serial.println(F("Starting Timer 3"));
  } else {
    Serial.println(F("Timer 3 Startup failed!"));
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