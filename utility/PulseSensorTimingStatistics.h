/*
 * Sample time statistics functions.
 * Designed to provide insite into the timing accuracy of
 * programs that don't use interrupts to read data from a PulseSensor.
 * 
 * See https://www.pulsesensor.com
 * and https://github.com/WorldFamousElectronics/PulseSensorPlayground
 * 
 * Portions Copyright (c) 2016, 2017 Bradford Needham, North Plains, Oregon, USA
 * @bneedhamia, https://bluepapertech.com
 * 
 * Licensed under the MIT License, a copy of which
 * should have been included with this software.
 * 
 * This software is not intended for medical use.
 */
#ifndef PULSE_SENSOR_TIMING_STATISTICS_H
#define PULSE_SENSOR_TIMING_STATISTICS_H

#include <Arduino.h>

/*
 * Timing statistics show how accurate the beats per minute
 * and inter-beat interval measurements are.
 * 
 * An average offset other than zero shows that samples were recorded
 * at a rate different from the expected rate.
 * For example, for an expected sample interval of 2000 microseconds
 * (500 samples per second), an offset of 60 microseconds indicates that
 * samples were recorded at a rate 3% slower than expected, which in turn
 * shows that the measured beats per minute and inter-beat interval
 * have a 3% error due to timing offset.
 * 
 * A large span between minimum and maximum jitter shows that sometimes
 * the sampling loop was slow or fast. This could be due to, for example,
 * unexpectedly slow code that executes only every so often.
 */
class PulseSensorTimingStatistics {
  public:
    PulseSensorTimingStatistics(long sampleIntervalMicros, int samplesToMeasure);
    void resetStatistics();
    int recordSampleTime();  // returns the number of samples remaining.
    void outputStatistics();
    
    int getMinJitterMicros() { return MinJitterMicros; }
    int getMaxJitterMicros() { return MaxJitterMicros; }
    int getAverageOffsetMicros();
  
  private:
    long SampleIntervalMicros; // desired sample interval, in microseconds.
    int SamplesDesired;       // total number of samples we want to record.
    unsigned long LastSampleMicros; // time (microseconds) of the previous sample.
    int MinJitterMicros;      // minimum offset seen.
    int MaxJitterMicros;      // maximum offset seen.
    float OffsetsSum;         // sum of offsets so far.
    int SamplesSeen;          // number of samples seen so far.
};
#endif // PULSE_SENSOR_TIMING_STATISTICS_H
