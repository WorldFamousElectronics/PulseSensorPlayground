/*
 * A central Playground object to exercise the PulseSensor.
 * See https://www.pulsesensor.com to get started.
 * 
 * Portions Copyright (c) 2016, 2017 Bradford Needham, North Plains, Oregon, USA
 * @bneedhamia, https://bluepapertech.com
 * 
 * Licensed under the MIT License, a copy of which
 * should have been included with this software.
 * 
 * This software is not intended for medical use.
 */
#ifndef PULSE_SENSOR_PLAYGROUND_H
#define PULSE_SENSOR_PLAYGROUND_H

#include <Arduino.h>
#include "utility/PulseSensorBeatDetector.h"
#include "utility/PulseSensorSerialOutput.h"
#include "utility/PulseSensorTimingStatistics.h"

class PulseSensorPlayground {
  public:
    /*
     * We fill these pointers in separate begin() functions
     * to minimize our memory footprint.
     */
    PulseSensorBeatDetector *pBeat;
    PulseSensorSerialOutput *pSerial;
    PulseSensorTimingStatistics *pTiming;

    /*
     * Beat detection functions
     */

    void beginBeatDetection() {
      pBeat = new PulseSensorBeatDetector();
    }
    void setBeatSampleIntervalMs(long sampleIntervalMs) {
      pBeat->setSampleIntervalMs(sampleIntervalMs);
	}
    int getBeatsPerMinute() { return pBeat->getBPM(); }
    int getInterBeatIntervalMs() { return pBeat->getIBI(); }
    boolean isBeat() { return pBeat->isBeat(); }
    boolean addBeatValue(int analogValue) {
      return pBeat->addBeatValue(analogValue);
    }

    /*
     * Serial Output functions
     */

    void beginSerialOutput(int outputType) {
      pSerial = new PulseSensorSerialOutput(outputType);
    }
    void output(int signal, int bpm, int ibi) {
      pSerial->output(signal, bpm, ibi);
    }
    void outputBeat(int bpm, int ibi) {
      pSerial->outputBeat(bpm, ibi);
    }

    /*
     * Timing Statistics functions
     */
    void beginTimingStatistics(long sampleIntervalMicros, int samplesToMeasure) {
      pTiming = new PulseSensorTimingStatistics(
          sampleIntervalMicros
          , samplesToMeasure);
    }
    void resetStatistics() {
      pTiming->resetStatistics();
    }
    int recordSampleTime() {
      return pTiming->recordSampleTime();
    }
    void outputStatistics() { pTiming->outputStatistics(); }
    int getMinJitterMicros() { return pTiming->getMinJitterMicros(); }
    int getMaxJitterMicros() { return pTiming->getMaxJitterMicros(); }
    int getAverageOffsetMicros() {
      return pTiming->getAverageOffsetMicros();
    }
};
#endif // PULSE_SENSOR_PLAYGROUND_H
