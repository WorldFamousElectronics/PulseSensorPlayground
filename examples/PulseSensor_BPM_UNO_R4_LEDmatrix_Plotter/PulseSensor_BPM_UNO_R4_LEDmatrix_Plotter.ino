/*
   Code to detect pulses from the PulseSensor,
   using an interrupt service routine.

>>>>  This example targest the Arduino UNO R4 WiFi.
>>>>  It will plot the PulseSensor signal on the LED matrix of the UNO R4.
>>>>  Use the delay() inside the loop to change the matrix frame rate.

   Here is a link to the tutorial
   https://pulsesensor.com/pages/getting-advanced

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
   We use the FspTimer to setup a timer interrupt for sample acquisition
   FspTimer is part of the hardware core files for the UNO R4 
*/
#include "FspTimer.h"
FspTimer sampleTimer;


/*
   Every Sketch that uses the PulseSensor Playground must
   define USE_ARDUINO_INTERRUPTS before including PulseSensorPlayground.h.
   Here, #define USE_ARDUINO_INTERRUPTS true tells the library to use
   interrupts to automatically read and process PulseSensor data.

   See PulseSensorBPM_Alternative.ino for an example of not using interrupts.
*/
#define USE_ARDUINO_INTERRUPTS true
#include <PulseSensorPlayground.h>

/*
  This is the timer interrupt service routine where we acquire and process samples
*/
void sampleTimerISR(timer_callback_args_t __attribute((unused)) *p_args){
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
const int PULSE_BLINK = LED_BUILTIN;
const int PULSE_FADE = 5;
const int THRESHOLD = 550;   // Adjust this number to avoid noise when idle

/*
   All the PulseSensor Playground functions.
*/
PulseSensorPlayground pulseSensor;

/*
    library and variables used to plot on the LED matrix
*/
#include "Arduino_LED_Matrix.h"
ArduinoLEDMatrix plotter;
int maxX = 11;
int maxY = 7;
int minX = 0;
int minY = 0;
int pulseSignal;
byte frame[8][12] = { // frame array is [y][x]
  { 0,0,0,0,0,0,0,0,0,0,0,0 },
  { 0,0,0,0,0,0,0,0,0,0,0,0 },
  { 0,0,0,0,0,0,0,0,0,0,0,0 },
  { 0,0,0,0,0,0,0,0,0,0,0,0 },
  { 0,0,0,0,0,0,0,0,0,0,0,0 },
  { 0,0,0,0,0,0,0,0,0,0,0,0 },
  { 0,0,0,0,0,0,0,0,0,0,0,0 },
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

/*
  We have to get control of a timer on the UNO R4. First, we try and see if there are any free timers available.
  If there are no free timers available, we will just take control of one from some other purpose. 
  We shouldn't have to force things, but if you use alot of timers, beware of this force use code!
  You can check to see if you are forcing by un-commenting the "forcing timer get" print line.
  You can check to see what timer you have under your control by un-commenting the "got timer " print line.
*/
  uint8_t timer_type = GPT_TIMER;
  int8_t tindex = FspTimer::get_available_timer(timer_type);
  if(tindex == 0){
    // Serial.println("forcing timer get;")
    FspTimer::force_use_of_pwm_reserved_timer();
    tindex = FspTimer::get_available_timer(timer_type);
  }
  // Serial.print("got timer "); Serial.println(tindex);

/*
  sampleTimer.begin sets up the timer that we just got control of as a periodic timer with 500Hz frequency.
  It also passes the interrupt service routine that we made above. 
  SAMPLE_RATE_500HZ is defined in the PulseSensorPlayground.h file.
*/
  sampleTimer.begin(TIMER_MODE_PERIODIC, timer_type, tindex, SAMPLE_RATE_500HZ, 0.0f, sampleTimerISR);
  sampleTimer.setup_overflow_irq();
  sampleTimer.open();
  sampleTimer.start();

  // start up the LED matrix so we can control it.
  plotter.begin();
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
    Get the latest PulseSensor signal value and scale it to fit the LED matrix.
    advanceLEDplotter shifts the data history to the left.
*/
  pulseSignal = pulseSensor.getLatestSample(); // copy the latest sample value
  pulseSignal = 1023 - pulseSignal; // invert for matrix disply to show with X axis along power and analog pins
  pulseSignal = pulseSignal/128;  // scale to the LED matrix height
  pulseSignal = constrain(pulseSignal, minY, maxY); // limit the singal so it won't get out of the frame
  advanceLEDplotter();
  plotter.renderBitmap(frame, 8, 12);

  /*
     If a beat has happened since we last checked,
     write the per-beat information to Serial.
   */
  if (pulseSensor.sawStartOfBeat()) {
   pulseSensor.outputBeat();
  }

}


/*
  New PulseSensor data comes in on the right of the plotter.
  The 'right' is the matrix edge along the Qwiic/STEMMA connector edge of the board.
*/
void advanceLEDplotter(){
  for(int y=0; y<=maxY; y++){
    for(int x=0; x<=maxX-1; x++){
      if(frame[y][x+1] == 1){
        frame[y][x] = 1;
        frame[y][x+1] = 0;
      } else {
        frame[y][x] = 0;
      }
    }
  }
  frame[pulseSignal][maxX] = 1;
}
