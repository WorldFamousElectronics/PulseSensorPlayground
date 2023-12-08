/*
   Code to target the UNO R4 WiFi board.
   This example will beat a heart shape on the LED matrix!

   Check out the PulseSensor Playground Tools for explaination
   of all user functions and directives.
   https://github.com/WorldFamousElectronics/PulseSensorPlayground/blob/master/resources/PulseSensor%20Playground%20Tools.md

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
   Include the PulseSensor Playground library to get all the good stuff!
   The PulseSensor Playground library will decide whether to use
   a hardware timer to get accurate sample readings by checking
   what target hardware is being used and adjust accordingly.
   You will see a warning during compilation that notes if 
   a hardware timer is being used or not.
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
const int PULSE_FADE = 5;
const int THRESHOLD = 550;   // Adjust this number to avoid noise when idle

/*
   All the PulseSensor Playground functions.
*/
PulseSensorPlayground pulseSensor;

/*
    Library and variables used to draw the heart animation
    The Arduino heart icon will pulse with your heartbeat!
*/
#include "Arduino_LED_Matrix.h"
ArduinoLEDMatrix beatingHeart;
byte heart[8][12] = {
  { 0,0,0,0,0,0,0,0,0,0,0,0 },
  { 0,0,0,0,0,0,0,0,0,0,0,0 },
  { 0,0,0,0,0,1,0,1,0,0,0,0 },
  { 0,0,0,0,1,0,1,0,1,0,0,0 },
  { 0,0,0,0,1,0,0,0,1,0,0,0 },
  { 0,0,0,0,0,1,0,1,0,0,0,0 },
  { 0,0,0,0,0,0,1,0,0,0,0,0 },
  { 0,0,0,0,0,0,0,0,0,0,0,0 }
};
byte heartPulse[8][12] = {
  { 0,0,0,0,0,0,0,0,0,0,0,0 },
  { 0,0,0,0,1,1,0,1,1,0,0,0 },
  { 0,0,0,1,0,0,1,0,0,1,0,0 },
  { 0,0,0,1,0,0,0,0,0,1,0,0 },
  { 0,0,0,0,1,0,0,0,1,0,0,0 },
  { 0,0,0,0,0,1,0,1,0,0,0,0 },
  { 0,0,0,0,0,0,1,0,0,0,0,0 },
  { 0,0,0,0,0,0,0,0,0,0,0,0 }
};

void setup() {
  /*
     Use 115200 baud because that's what the Processing Sketch expects to read,
     and because that speed provides about 11 bytes per millisecond.

     If we used a slower baud rate, we'd likely write bytes faster than
     they can be transmitted, which would mess up the sample reading
     calls, which would make the pulse measurement not work properly.
  */
  Serial.begin(115200);

  // Configure the PulseSensor manager.

  pulseSensor.analogInput(PULSE_INPUT);
  pulseSensor.blinkOnPulse(PULSE_BLINK);
  pulseSensor.fadeOnPulse(PULSE_FADE);

  pulseSensor.setSerial(Serial);
  pulseSensor.setOutputType(OUTPUT_TYPE);
  pulseSensor.setThreshold(THRESHOLD);

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
      delay(50); Serial.println('!');
      digitalWrite(PULSE_BLINK, HIGH);
      delay(50);
    }
  }

  // start up the LED matrix so we can control it.
  beatingHeart.begin();
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
    The method isInsideBeat returns true when the pulse wave is above THRESHOLD.
    The timing makes a nice heart pulse along with your heartbeat.
*/
  if(pulseSensor.isInsideBeat()){
    beatingHeart.renderBitmap(heartPulse, 8, 12);
  } else {
    beatingHeart.renderBitmap(heart, 8, 12);
  }
  /*
     If a beat has happened since we last checked,
     write the per-beat information to Serial.
   */
  if (pulseSensor.sawStartOfBeat()) {
   pulseSensor.outputBeat();
  }

}

