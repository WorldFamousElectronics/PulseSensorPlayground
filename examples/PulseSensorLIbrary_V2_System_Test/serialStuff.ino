/*

*/

void checkSerial(){
  if(Serial.available()){
    char inChar = Serial.read();
    switch(inChar){
      case 't':
        testing = true;
        break;
      case 'r':
        normal = true;
        pulseSensor.resume();
        break;
      case 'p':
        normal = false;
        pulseSensor.pause();
        printInstructions();
        break;
      default:
        Serial.print("i got "); Serial.println(inChar);
    }
  } // Serial.available
} // checkSerial


void printResults(){
  float durationOfBeats = float(firstToLastBeatTime/1000.0);
  Serial.println("\tTEST COMPLETE"); 
  Serial.print("\tAverage BPM: "); Serial.println(testBPM);
  Serial.print("\tAverage IBI: "); Serial.println(testIBI);
  Serial.print("\tAverage Pulse Amplitude: "); Serial.println(testAmp);
  Serial.print("\tFirst to last heartbeat time: "); Serial.print(durationOfBeats,3); Serial.println(" Seconds"); 
  Serial.print("\tPlayground Library is using a ");
  if(pulseSensor.UsingHardwareTimer){
    Serial.println("hardware timer");
  } else {
    Serial.println("software timer");
  }
}

void printInstructions(){
  Serial.print("\nPulseSensor Playground "); Serial.println(PULSESENSOR_PLAYGROUND_VERSION_STRING);
  Serial.println("Full System Test Instructions:");
  Serial.println("\n\t1) Connect PulseSensor wires to the board under test");
  Serial.println("\t2) Use a known good signal source to connect PulseSensor to");
  Serial.println("\t3) Send 't' to begin the software funcional test");
  Serial.println("\t4) The test will run 10 seconds, be still and wait for results");
  Serial.println("\nDuring the test, the board will output PulseSensor raw data");
  Serial.println("for review in monitor or plotter.");
  Serial.println("\nSend 'r' to run the pulseSensor with normal output");
  Serial.println("Send 'p' to pause normal output, and print this message");
  Serial.print("PulseSensor is currently ");
  if(pulseSensor.isPaused()){
    Serial.println("paused");
  } else {
    Serial.println("running!");
  }
  // Serial.println("");
  // Serial.println("");
}