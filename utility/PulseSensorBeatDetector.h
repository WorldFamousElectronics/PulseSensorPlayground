/*
 * Beats-Per-Minute (BPM) heart rate calculation library.
 * Based on Pulse Sensor Amped 1.4 by Joel Murphy and Yury Gitman
 * See https://www.pulsesensor.com
 * and https://github.com/WorldFamousElectronics/PulseSensor_Amped_Arduino
 * 
 * This library supports interrupt and non-interrupt use, enabling its
 * use in a wider set of Arduino compatible boards and using a smaller set
 * of I/O pins than the original Sketch.
 * 
 * Source, etc. available at
 * https://github.com/bneedhamia/PulseSensorBeatDetector
 * 
 * Portions Copyright (c) 2016, 2017 Bradford Needham, North Plains, Oregon, USA
 * @bneedhamia, https://bluepapertech.com
 * Licensed under the MIT License, a copy of which
 * should have been included with this software.
 * 
 * This software is not intended for medical use.
 */
#ifndef PULSE_SENSOR_BEAT_DETECTOR_H
#define PULSE_SENSOR_BEAT_DETECTOR_H

#include <Arduino.h>

/*
 * Default expected time between samples from the PulseSensor.
 */
const long DEFAULT_SAMPLE_INTERVAL_MS = 2L;

class PulseSensorBeatDetector {
  public:
    PulseSensorBeatDetector();
    void setSampleIntervalMs(long newSampleIntervalMs);
    int getBPM();
    int getIBI();
    boolean isBeat();
    boolean addBeatValue(int analogValue);

  private:
    // Pulse detection output variables.
    // Volatile because our pulse detection code could be called from an Interrupt
    volatile int BPM;                // int that holds raw Analog in 0. updated every call to readSensor()
    volatile int Signal;             // holds the latest incoming raw data (0..1023)
    volatile int IBI;                // int that holds the time interval (ms) between beats! Must be seeded! 
    volatile boolean Pulse;          // "True" when User's live heartbeat is detected. "False" when not a "live beat". 
    
    // Variables internal to the pulse detection algorithm.
    // Not volatile because we use them only in the pulse detection function.
    unsigned long sampleIntervalMs;  // expected time between calls to readSensor(), in milliseconds.
    int rate[10];                    // array to hold last ten IBI values (ms)
    unsigned long sampleCounter;     // used to determine pulse timing. Milliseconds since we started.
    unsigned long lastBeatTime;      // used to find IBI. Time (sampleCounter) of the previous detected beat start.
    int P;                           // used to find peak in pulse wave, seeded (sample value)
    int T;                           // used to find trough in pulse wave, seeded (sample value)
    int thresh;                      // used to find instant moment of heart beat, seeded (sample value)
    int amp;                         // used to hold amplitude of pulse waveform, seeded (sample value)
    boolean firstBeat;               // used to seed rate array so we startup with reasonable BPM
    boolean secondBeat;              // used to seed rate array so we startup with reasonable BPM
};
#endif // PULSE_SENSOR_BEAT_DETECTOR_H
