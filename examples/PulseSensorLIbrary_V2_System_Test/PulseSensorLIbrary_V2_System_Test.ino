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
  All other functionality is implicitly tested by successfully running the program.
  


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
int beatCounter;
int firstBeatTime, lastBeatTime, firstToLastBeatTime;

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
  printInstructions();
  
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


/*
  Receives millis() and runs a test for TEST_DURATION 
*/
void runTest(unsigned long startTime){
  beatCounter = 0;  // reset the beat counter
  testIBI = 0;
  testBPM = 0;
  testAmp = 0;
  firstBeatTime = -1;
  lastBeatTime = -1;
  Serial.println("\n\tSTART TEST");
  pulseSensor.resume(); // start the sensing!
  while((millis() - startTime) < TEST_DURATION){
    Serial.println(pulseSensor.getLatestSample()); // print raw data for plotter or monitor review
    if(pulseSensor.sawStartOfBeat()){
      beatCounter++;
      if(firstBeatTime < 0){ firstBeatTime = pulseSensor.getLastBeatTime(); }
      testBPM += pulseSensor.getBeatsPerMinute(); 
      testIBI += pulseSensor.getInterBeatIntervalMs();
      testAmp += pulseSensor.getPulseAmplitude();
    }
    delay(20);
  }
  lastBeatTime = pulseSensor.getLastBeatTime();
  pulseSensor.pause();
  testBPM /= beatCounter;
  testIBI /= beatCounter;
  testAmp /= beatCounter;
  firstToLastBeatTime = lastBeatTime - firstBeatTime;

  printResults();
  printInstructions();
  testing = false;
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