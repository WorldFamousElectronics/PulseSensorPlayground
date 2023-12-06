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
        break;
      default:
        Serial.print("i got "); Serial.println(inChar);
    }
  } // Serial.available
} // checkSerial