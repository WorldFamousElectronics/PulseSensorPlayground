/*
   Code to detect pulses from the PulseSensor,
   using SoftwareSerial for Serial output.

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

   For more information on the PulseSensor methods and functions
   go to our Resources page
   https://github.com/WorldFamousElectronics/PulseSensorPlayground/blob/master/resources/PulseSensor%20Playground%20Tools.md
*/
/*
   Include the PulseSensor Playground library to get all the good stuff!
   The PulseSensor Playground library will decide whether to use
   a hardware timer to get accurate sample readings by checking
   what target hardware is being used and adjust accordingly.
   You may see a "warning" come up in red during compilation
   if a hardware timer is not being used.
*/
#include <PulseSensorPlayground.h>
#include <SoftwareSerial.h>

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
     PIN_RX = Serial Receive pin (input into Arduino)
     PIN_TX = Serial Transmit pin (output from Arduino)

     In most cases, you'll want to wire the Arduino PIN_RX
     to the TRANSMIT pin of the external serial device,
     and the Arduino PIN_TX to the RECEIVE pin of the
     external device.
*/
const int PIN_RX = 7;
const int PIN_TX = 8;

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
const int PULSE_FADE = 5;      // Must be a PWM pin other than 9 or 10.
const int THRESHOLD = 550;   // Adjust this number to avoid noise when idle

/*
   Our software serial controller.
 */
SoftwareSerial ourSerial(PIN_RX, PIN_TX);

PulseSensorPlayground pulseSensor;

void setup() {

  ourSerial.begin(115200);

  // Configure the PulseSensor manager.

  pulseSensor.analogInput(PULSE_INPUT);
  pulseSensor.blinkOnPulse(PULSE_BLINK);
  pulseSensor.fadeOnPulse(PULSE_FADE);

  pulseSensor.setSerial(ourSerial);
  pulseSensor.setOutputType(OUTPUT_TYPE);
  pulseSensor.setThreshold(THRESHOLD);

  if (!pulseSensor.begin()) {
    /*
     * PulseSensor initialization failed,
     * likely because our particular Arduino platform interrupts
     * aren't supported yet.
     *
     * If your Sketch hangs here, try changing USE_ARDUINO_INTERRUPTS to false.
     */
    for(;;) {
      // Flash the led to show things didn't work.
      digitalWrite(PULSE_BLINK, LOW);
      delay(50);
      digitalWrite(PULSE_BLINK, HIGH);
      delay(50);
    }
  }
}

void loop() {
  /*
     See if a sample is ready from the PulseSensor.

     If USE_HARDWARE_TIMER is true, the PulseSensor Playground
     will automatically read and process samples from
     the PulseSensor.

     If USE_HARDWARE_TIMER is false, the call to sawNewSample()
     will check to see how much time has passed, then read
     and process a sample (analog voltage) from the PulseSensor.
     Call this function often to maintain 500Hz sample rate,
     that is every 2 milliseconds. Best not to have any delay() 
     functions in the loop when using a software timer.

     Check the compatibility of your hardware at this link
     <url>
     and delete the unused code portions in your saved copy, if you like.
  */
  if(pulseSensor.UsingHardwareTimer){
    /*
       Wait a bit.
       We don't output every sample, because our baud rate
       won't support that much I/O.
    */
    delay(20); 
    // write the latest sample to Serial.
    pulseSensor.outputSample();
  } else {
  /*
      When using a software timer, we have to check to see if it is time
      to acquire another sample. A call to sawNewSample will do that.
  */
    if (pulseSensor.sawNewSample()) {
      /*
          Every 20 millisectonds, send the latest Sample.
          We don't print every sample, because our baud rate
          won't support that much I/O.
      */
      if ((pulseSensor.samplesUntilReport--) == 0) {
        pulseSensor.samplesUntilReport = SAMPLES_PER_SERIAL_SAMPLE;
        pulseSensor.outputSample();
      }
    }
  }

  if (pulseSensor.sawStartOfBeat()) {
    pulseSensor.outputBeat();
  }

}
