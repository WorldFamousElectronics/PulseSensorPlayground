/*
  This example will test functionality of PulseSensor user functions.
  Program any hardware with this sketch and open a serial port
  to view the test instructions, operation and results.
  Functionality that is explicity tested by the program:
    pause()
    resume()
    getInterBeatIntervalMs()
    getBeatsPerMinute()
    getLatestSample()
    getPulseAmplitude()
    getLastBeatTime()
  All other functionality is implicitly tested by running the program.
  


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
   Test notes here
*/

#include <PulseSensorPlayground.h>

// Test variables
#define TEST_DURATION 10000 // run time in milliseconds
unsigned long thisTime;
bool testing = false;
bool normal = false;
uint8_t errorCode = 0x00; // maybe used for anything automatic?
int testBPM, testIBI, testAmp, testLastBeatTime; // test variables

// Standard PulseSensor Stuff
const int OUTPUT_TYPE = SERIAL_PLOTTER;
const int PULSE_INPUT = A0;
const int PULSE_BLINK = LED_BUILTIN;
const int PULSE_FADE = 5;
const int THRESHOLD = 550;   // Adjust this number to avoid noise when idle

PulseSensorPlayground pulseSensor;

void setup() {

  Serial.begin(115200);
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
  pulseSensor.pause();
  delay(100);
  
}

void loop() {
  if(testing){
    runTest(millis());
  }
  if(normal){
    runNormal();
  }
  checkSerial();
} // loop


void printInstructions(){
  Serial.print("PulseSensor Playground "); Serial.println(PULSESENSOR_PLAYGROUND_VERSION_STRING);
  Serial.println("Full System Test Program Instructions");
  Serial.println("\n\t1) Connect PulseSensor wires to the board under test");
  Serial.println("\t2) Use a known good signal source to connect PulseSensor to");
  Serial.println("\t3) Send 'b' to begin the software funcional test");
  Serial.println("\t4) The test will run 10 seconds, be still and wait for results");
  Serial.println("\nDuring the test, the board will output PulseSensor raw data");
  Serial.println("for review in monitor or plotter.");
  Serial.println("\nSend 'r' to run the pulseSensor with normal output");
  Serial.println("Send 'p' to pause normal output, and print this message");
  // Serial.println("");
  // Serial.println("");
}


/*
  Receives millis() and runs a test for TEST_DURATION 
*/
uint8_t runTest(unsigned long startTime){
  Serial.println("\n\tSTART TEST");
  pulseSensor.resume(); // start the sensing!


}
    

void runNormal(){
if(pulseSensor.UsingHardwareTimer){
      delay(20); 
      pulseSensor.outputSample();
    } else {
      if (pulseSensor.sawNewSample()) {
        if (--pulseSensor.samplesUntilReport == (byte) 0) {
          pulseSensor.samplesUntilReport = SAMPLES_PER_SERIAL_SAMPLE;
          pulseSensor.outputSample();
        }
      }
    }
    if (pulseSensor.sawStartOfBeat()) {
      pulseSensor.outputBeat();
    }
}