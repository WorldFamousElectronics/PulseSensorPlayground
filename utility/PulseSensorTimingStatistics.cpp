/*
   Sample time statistics functions.
   Designed to provide insite into the timing accuracy of
   programs that read data from a PulseSensor.

   See https://www.pulsesensor.com
   and https://github.com/WorldFamousElectronics/PulseSensorPlayground

   Portions Copyright (c) 2016, 2017 Bradford Needham, North Plains, Oregon, USA
   @bneedhamia, https://bluepapertech.com
   Licensed under the MIT License, a copy of which
   should have been included with this software.

   This software is not intended for medical use.
*/

#include <PulseSensorTimingStatistics.h>

/*
   Constructs an object for measuring statistics about the timing
   of samples from the PulseSensor.

   sampleIntervalMicros = expected time between samples, in microseconds.
   samplesToMeasure = number of samples to measure timing over.
*/
PulseSensorTimingStatistics::PulseSensorTimingStatistics(
  long sampleIntervalMicros, int samplesToMeasure) {
  SamplesDesired = samplesToMeasure;
  SampleIntervalMicros = sampleIntervalMicros;

  resetStatistics();
}

/*
   (re)start the collection of timing statistics.
*/
void PulseSensorTimingStatistics::resetStatistics() {
  SamplesSeen = 0;
  MinJitterMicros = 0;
  MaxJitterMicros = 0;
  OffsetsSum = 0.0;
  LastSampleMicros = 0L;
}

/*
   Record the fact that we just now read the PulseSensor output.

   Returns the number of samples remaining to be recorded.
   The caller should stop calling recordSampleTime() once
   this function returns 0.
*/
int PulseSensorTimingStatistics::recordSampleTime() {
  unsigned long nowMicros = micros();

  if (SamplesSeen > 0) {
    long offsetMicros =
      (long) (nowMicros - LastSampleMicros) - SampleIntervalMicros;
    offsetMicros = constrain(offsetMicros, -32767, 32767);

    OffsetsSum += (float) offsetMicros;

    if (MinJitterMicros > offsetMicros) {
      MinJitterMicros = offsetMicros;
    }
    if (MaxJitterMicros < offsetMicros) {
      MaxJitterMicros = offsetMicros;
    }
  }

  LastSampleMicros = nowMicros;
  ++SamplesSeen;

  return (SamplesDesired - SamplesSeen);
}

/*
   Serial prints the sample timing statistics.
*/
void PulseSensorTimingStatistics::outputStatistics() {
  Serial.print(MinJitterMicros);
  Serial.print(" ");
  Serial.print(getAverageOffsetMicros());
  Serial.print(" ");
  Serial.println(MaxJitterMicros);
}

/*
   Returns the average offset seen so far, in microseconds.
*/
int PulseSensorTimingStatistics::getAverageOffsetMicros() {
  // the number of offsets is the number of samples - 1.
  if (SamplesSeen - 1 <= 0) {
    return 0;
  }
  return (int) ((OffsetsSum + 0.5) / (SamplesSeen - 1));
}
