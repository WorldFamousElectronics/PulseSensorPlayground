/*
   Sample time statistics functions.
   Designed to provide insite into the timing accuracy of
   programs that read data from a PulseSensor.
   See PulseSensorPlayground.h PULSE_SENSOR_TIMING_ANALYSIS

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

PulseSensorTimingStatistics::PulseSensorTimingStatistics(
  long sampleIntervalMicros, int samplesToMeasure) {
  SamplesDesired = samplesToMeasure;
  SampleIntervalMicros = sampleIntervalMicros;

  resetStatistics();
}

void PulseSensorTimingStatistics::resetStatistics() {
  SamplesSeen = 0;
  MinJitterMicros = 0;
  MaxJitterMicros = 0;
  OffsetsSum = 0.0;
  LastSampleMicros = 0L;
}

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

void PulseSensorTimingStatistics::outputStatistics() {
  // Compile Serial only if we're doing timing analysis
  // so that compilation succeeds on Software-Serial-only Arduinos.
#if PULSE_SENSOR_TIMING_ANALYSIS
  Serial.print(MinJitterMicros);
  Serial.print(" ");
  Serial.print(getAverageOffsetMicros());
  Serial.print(" ");
  Serial.println(MaxJitterMicros);
#endif // PULSE_SENSOR_TIMING_ANALYSIS
}

int PulseSensorTimingStatistics::getAverageOffsetMicros() {
  // the number of offsets is the number of samples - 1.
  if (SamplesSeen - 1 <= 0) {
    return 0;
  }
  return (int) ((OffsetsSum + 0.5) / (SamplesSeen - 1));
}
