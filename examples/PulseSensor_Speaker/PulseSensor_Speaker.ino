/*
   Code to detect pulses from the PulseSensor

   >>>>  THIS EXAMPLE OUTPUTS USES TONE COMMAND <<<<
   >>>>  TO MAKE A SPEAKER BEEP WITH HEARTBEAT! <<<<

   Here is a link to the tutorial
   https://pulsesensor.com/pages/pulse-sensor-speaker-tutorial

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
   You may see a "warning" come up in red during compilation
   if a hardware timer is not being used.
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
  Setup the things we need for driving the Speaker
  NOTE: Speaker MUST be AC coupled! Connect PIN_SPEAKER to red speaker wire.
        Then connect black speaker wire to + side of electrolytic capacitor.
        Then connect - side of electrolytic capacitor to GND.
        Capacitor value should be 1uF or higher!
        Follow this tutorial:
        https://pulsesensor.com/pages/pulse-sensor-speaker-tutorial
*/
const int SPEAKER_PIN = 3;    // speaker on pin3 makes a beep with your heartbeat


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

       If your Sketch hangs here, try changing USE_ARDUINO_INTERRUPTS to false.
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
          Every so often, send the latest Sample.
          We don't print every sample, because our baud rate
          won't support that much I/O.
      */
      if (--pulseSensor.samplesUntilReport == (byte) 0) {
        pulseSensor.samplesUntilReport = SAMPLES_PER_SERIAL_SAMPLE;
        pulseSensor.outputSample();
      }
    }
  }

  /*
     If a beat has happened since we last checked,
     write the per-beat information to Serial.
     write a frequency to the PIN_SPEAKER
     NOTE: Do not set the optional duration of tone! That is blocking!
   */
  if (pulseSensor.sawStartOfBeat()) {
    pulseSensor.outputBeat();
    heartBeep(SPEAKER_PIN,true);
  }

  /*
    The Pulse variable is true only for a short time after the heartbeat is detected
    Use this to time the duration of the beep
  */
  if(pulseSensor.isInsideBeat() == false){
    heartBeep(SPEAKER_PIN,false);
  }

}
/*
  heartBeep(to beep or not to beep)
  This function will reliably output a clean tone (500Hz on AVR, sounds about Bb). 
  You can try the tone() function that comes in the Arduino core if you want a different note, but it will be noisy.
    The Arduino tone() function starts up a hardware timer at a specific freqeuency, and initializes an interrupt.
    The problem with using tone() is that the interrupt needs to be called in order to toggle the pin at frequency.
    The tone() interrupt collides with our PulseSensor interrupt and operations. Thankfully, tone() breaks and not PulseSensor!
    The arduino core needs to be updated so that the tone library operates 'hands free' like the PWM library.
    Or, the PWM library needs to be updated to accept a frequency parameter to ensure a clean(er) note.
    There are some architectures upon which the Tone library might work: nRF52? ESP32? 
*/
void heartBeep(int pin, bool beep){
  if(beep){
    analogWrite(pin,127);
  } else {
    analogWrite(pin,0);
  }
}
