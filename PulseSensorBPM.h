/*
 * Beats-Per-Minute (BPM) heart rate calculation library.
 * Based on Pulse Sensor Amped 1.4 by Joel Murphy and Yury Gitman
 * See http://www.pulsesensor.com
 * and https://github.com/WorldFamousElectronics/PulseSensor_Amped_Arduino
 * 
 * This library supports interrupt and non-interrupt use, enabling its
 * use in a wider set of Arduino compatible boards and using a smaller set
 * of I/O pins than the original Sketch.
 * 
 * Source, etc. available at https://github.com/bneedhamia/PulseSensorBPM
 * 
 * Portions Copyright (c) 2016 Bradford Needham, North Plains, Oregon
 * @bneedhamia, https://www.needhamia.com
 * Licensed under the MIT License, a copy of which
 * should have been included with this software.
 * 
 * This software is not intended for medical use.
 */
#ifndef pulsesensorbpm_h
#define pulsesensorbpm_h

#include <Arduino.h>

/*
 * PulseAmpedSensorVersion = binary version number of this library.
 *   rightmost (lsb) 4 hexidecimal digits = minor version number;
 *   next left 4 digits = major version number.
 */
#define PULSE_AMPED_BPM_VERSION 0x00010000L

class PulseSensorBPM {
  private:
    int pinPulse;                   // Analog Input pin the Pulse Sensor is connected to.
    
    // Pulse detection output variables.
    // Volatile because our pulse detection code could be called from an Interrupt
    volatile int BPM;                // int that holds raw Analog in 0. updated every call to readSensor()
    volatile int Signal;             // holds the incoming raw data (0..1023)
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

  public:
    static long getVersion();
    PulseSensorBPM(int pulse_pin, unsigned long sample_interval_ms);
    int getSignal();
    int getBPM();
    int getIBI();
    boolean isPulse();
    boolean readSensor();
};
#endif // pulsesensorbpm_h
