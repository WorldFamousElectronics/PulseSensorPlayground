/*
   A central Playground object to manage the PulseSensor.
   See https://www.pulsesensor.com to get started.

   Copyright World Famous Electronics LLC - see LICENSE
   Contributors:
     Joel Murphy, https://pulsesensor.com
     Yury Gitman, https://pulsesensor.com
     Bradford Needham, @bneedhamia, https://bluepapertech.com

   Licensed under the MIT License, a copy of which
   should have been included with this software.

   This software is not intended for medical use.
*/
#include <PulseSensorPlayground.h>

PulseSensorPlayground::PulseSensorPlayground(int numberOfSensors) {

  // By default, we attempt to use interrupts to sample.
  UsingInterrupts = true;

  // Dynamically create the array to minimize ram usage.
  SensorCount = (byte) numberOfSensors;
  Sensors = new PulseSensor[SensorCount];
}

boolean PulseSensorPlayground::PulseSensorPlayground::begin() {
  if (pSerial) {
    pSerial->begin();
  }

  for (int i = 0; i < numberOfSensors) {
    Sensors[i].begin();  //TODO does all the LED initialization.
  }

  // Lastly, set up the interrupts and set the time of the next sample.
  //TODO if UsingInterrupts: ISR, interrupts-on.

  NextSampleMicros = micros() + MICROS_PER_READ;

  return true;
}

void PulseSensorPlayground::analogInput(int inputPin, int sensorIndex) {
  if (sensorIndex != constrain(sensorIndex, 0, numberOfSensors)) {
    return; // out of range.
  }
  Sensors[sensorIndex].analogInput(inputPin);
}

void PulseSensorPlayground::blinkOnPulse(int blinkPin, int sensorIndex) {
  if (sensorIndex != constrain(sensorIndex, 0, numberOfSensors)) {
    return; // out of range.
  }
  Sensors[sensorIndex].blinkOnPulse(blinkPin);
}

void PulseSensorPlayground::fadeOnPulse(int fadePin, int sensorIndex) {
  if (sensorIndex != constrain(sensorIndex, 0, numberOfSensors)) {
    return; // out of range.
  }
  Sensors[sensorIndex].fadeOnPulse(fadePin);
}

boolean PulseSensorPlayground::sawNewSample() { 
  /*
     If using interrupts, this function reads and clears the
     'saw a sample' flag that is set by the ISR.

     When not using interrupts, this function sees whether it's time
     to sample and, if so, reads the sample and processes it.
  */
  
  if (UsingInterrupts) {
    //TODO test and clear volatile 'saw a sample'.
    return true; //TODO if saw a sample was true.
  }

  // Not using interrupts
  
  unsigned long nowMicros = micros();
  if ((long) (NextSampleMicros - nowMicros) > 0L) {
    return false;  // not time yet.
  }
  NextSampleMicros = nowMicros + MICROS_PER_READ;

  // Read signals from all pulse sensors.
  // To minimize jitter, we read all samples first, then process them.
  for (int i = 0; i < numberOfSensors; ++i) {
    Sensors[i].readNextSample();
  }
  for (int i = 0; i < numberOfSensors; ++i) {
    sensor[i].processLatestSample();
  }

  return true;
}

int PulseSensorPlayground::getLatestSample(int sensorIndex) {
  if (sensorIndex != constrain(sensorIndex, 0, numberOfSensors)) {
    return; // out of range.
  }
  return Sensors[sensorIndex].getLatestSample();
}

int PulseSensorPlayground::getBeatsPerMinute(int sensorIndex) {
  if (sensorIndex != constrain(sensorIndex, 0, numberOfSensors)) {
    return; // out of range.
  }
  return Sensors[sensorIndex].getBeatsPerMinute(); 
}

int PulseSensorPlayground::getInterBeatIntervalMs(int sensorIndex) {
  if (sensorIndex != constrain(sensorIndex, 0, numberOfSensors)) {
    return; // out of range.
  }
  return Sensors[sensorIndex].getInterBeatIntervalMs(); 
}

boolean PulseSensorPlayground::sawStartOfBeat(int sensorIndex) {
  if (sensorIndex != constrain(sensorIndex, 0, numberOfSensors)) {
    return; // out of range.
  }
  return Sensors[sensorIndex].sawStartOfBeat(); 
}

boolean PulseSensorPlayground::isInsideBeat(int sensorIndex) {
  if (sensorIndex != constrain(sensorIndex, 0, numberOfSensors)) {
    return; // out of range.
  }
  return Sensors[sensorIndex].isInsideBeat();
}

void PulseSensorPlayground::setOutputType(int outputType) {
  SerialOutput.outputType(outputType);
}

void PulseSensorPlayground::outputSample() {
  SerialOutput.outputSample(Sensors, SensorCount);
}

void PulseSensorPlayground::outputBeat() {
  SerialOutput.outputBeat(Sensors, SensorCount);
}

