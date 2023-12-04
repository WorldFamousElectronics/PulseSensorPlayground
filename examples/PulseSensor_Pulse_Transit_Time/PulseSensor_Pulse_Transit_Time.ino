/*
   Arduino Sketch to detect pulses from two PulseSensors
   and measures the time between! This can be used to derive
   Pulse Transit Time (PTT)

   Here is a link to the PTT tutorial
   https://pulsesensor.com/pages/pulse-transit-time
   Using two PulseSensors, connect the one on A0 closer to your heart.
   Connect the one on A1 at a point further from your heart.

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
   Every Sketch that uses the PulseSensor Playground must
   define USE_ARDUINO_INTERRUPTS before including PulseSensorPlayground.h.
   Here, #define USE_ARDUINO_INTERRUPTS false tells the library to
   not use interrupts to read data from the PulseSensor.

   If you want to use interrupts, simply change the line below
   to read:
     #define USE_ARDUINO_INTERRUPTS true

   Set US_PS_INTERRUPTS to false if either
   1) Your Arduino platform's interrupts aren't yet supported
   by PulseSensor Playground, or
   2) You don't wish to use interrupts because of the side effects.

   NOTE: if US_PS_INTERRUPTS is false, your Sketch must
   call pulse.sawNewSample() at least once every 2 milliseconds
   to accurately read the PulseSensor signal.
*/
#define USE_ARDUINO_INTERRUPTS true
#include <PulseSensorPlayground.h>


/*
   The format of our output.

   Set this to PROCESSING_VISUALIZER if you're going to run
    the multi-sensor Processing Visualizer Sketch.
    See https://github.com/WorldFamousElectronics/PulseSensorAmped_2_Sensors

   Set this to SERIAL_PLOTTER if you're going to run
    the Arduino IDE's Serial Plotter.
*/
const int OUTPUT_TYPE = PROCESSING_VISUALIZER;

/*
   Number of PulseSensor devices we're reading from.
*/
const int PULSE_SENSOR_COUNT = 2;

/*
   Pinout:
     PULSE_INPUT = Analog Input. Connected to the pulse sensor
      purple (signal) wire. Ends with index number.
     PULSE_BLINK = digital Output. Connected to an LED (and 1K series resistor)
      that will flash on each detected pulse. Ends with index number.
     PULSE_FADE = digital Output. PWM pin onnected to an LED (and 1K series resistor)
      that will smoothly fade with each pulse. Ends with index number.
      NOTE: PULSE_FADE must be a pin that supports PWM. Do not use
      pin 9 or 10, because those pins' PWM interferes with the sample timer. Ends with index number.
     THRESHOLD should be set higher than the PulseSensor signal idles
      at when there is nothing touching it. The expected idle value
      should be 512, which is 1/2 of the ADC range. To check the idle value
      open a serial monitor and make note of the PulseSensor signal values
      with nothing touching the sensor. THRESHOLD should be a value higher
      than the range of idle noise by 25 to 50 or so. When the library
      is finding heartbeats, the value is adjusted based on the pulse signal
      waveform. THRESHOLD sets the default when there is no pulse present.
      Adjust as neccesary.  Ends with index number.
*/
const int PULSE_INPUT0 = A0;
const int PULSE_BLINK0 = LED_BUILTIN;
const int PULSE_FADE0 = 5;
const int THRESHOLD0 = 550;  

const int PULSE_INPUT1 = A1;
const int PULSE_BLINK1 = 12;
const int PULSE_FADE1 = 11;
const int THRESHOLD1 = 550;

/*
   All the PulseSensor Playground functions.
   We tell it how many PulseSensors we're using.
*/
PulseSensorPlayground pulseSensor(PULSE_SENSOR_COUNT);

/*
  Variables used to determine PTT.
  NOTE: This code assumes the Pulse Sensor on analog pin 0 is closer to he heart.
*/
unsigned long lastBeatSampleNumber[PULSE_SENSOR_COUNT];
int PTT;

void setup() {
  /*
     Use 250000 baud because that's what the Processing Sketch expects to read,
     and because that speed provides about 25 bytes per millisecond,
     or 50 characters per PulseSensor sample period of 2 milliseconds.

     If we used a slower baud rate, we'd likely write bytes faster than
     they can be transmitted, which would mess up the timing
     of readSensor() calls, which would make the pulse measurement
     not work properly.
  */
  Serial.begin(250000);

  /*
     Configure the PulseSensor manager,
     telling it which PulseSensor (0 or 1)
     we're configuring.
  */

  pulseSensor.analogInput(PULSE_INPUT0, 0);
  pulseSensor.blinkOnPulse(PULSE_BLINK0, 0);
  pulseSensor.fadeOnPulse(PULSE_FADE0, 0);
  pulseSensor.setThreshold(THRESHOLD0, 0);

  pulseSensor.analogInput(PULSE_INPUT1, 1);
  pulseSensor.blinkOnPulse(PULSE_BLINK1, 1);
  pulseSensor.fadeOnPulse(PULSE_FADE1, 1);
  pulseSensor.setThreshold(THRESHOLD1, 1);

  pulseSensor.setSerial(Serial);
  pulseSensor.setOutputType(OUTPUT_TYPE);


  // Now that everything is ready, start reading the PulseSensor signal.
  if (!pulseSensor.begin()) {
    /*
       PulseSensor initialization failed,
       likely because our Arduino platform interrupts
       aren't supported yet.

       If your Sketch hangs here, try changing USE_ARDUINO_INTERRUPTS to false.
    */
    for (;;) {
      // Flash the led to show things didn't work.
      digitalWrite(PULSE_BLINK0, LOW);
      delay(50);
      digitalWrite(PULSE_BLINK0, HIGH);
      delay(50);
    }
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
     If a beat has happened on a given PulseSensor
     since we last checked, write the per-beat information
     about that PulseSensor to Serial.
  */
  for (int i = 0; i < PULSE_SENSOR_COUNT; ++i) {
    if (pulseSensor.sawStartOfBeat(i)) {
      pulseSensor.outputBeat(i);

      lastBeatSampleNumber[i] = pulseSensor.getLastBeatTime(i);
      if(i == 1){
        PTT = lastBeatSampleNumber[1] - lastBeatSampleNumber[0];
        pulseSensor.outputToSerial('|',PTT);
      }
    }
  }

}
