/*
 * 
 */
#ifndef PULSE_SENSOR_PLAYGROUND_H
#define PULSE_SENSOR_PLAYGROUND_H

#include <Arduino.h>
#include "utility/PulseSensorBeatDetector.h"

class PulseSensorPlayground {
  public:
    PulseSensorBeatDetector *pBeat;
    void beginBeatDetection(int pulse_pin
        , unsigned long sample_interval_ms) {
      pBeat = new PulseSensorBeatDetector(pulse_pin, sample_interval_ms);
    }
    int getBeatSignal() { return pBeat->getSignal(); }
    int getBeatsPerMinute() { return pBeat->getBPM(); }
    int getInterBeatIntervalMs() { return pBeat->getIBI(); }
    boolean isBeat() { return pBeat->isPulse(); }
    boolean readPulseSensor() { return pBeat->readSensor(); }
    
};

#endif // PULSE_SENSOR_PLAYGROUND_H
