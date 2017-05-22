/*
   Formatting of Serial output from PulseSensors.
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
#include "PulseSensorSerialOutput.h"

PulseSensorSerialOutput::PulseSensorSerialOutput() {
  pOutput = NULL;
  OutputType = SERIAL_PLOTTER;
}

void PulseSensorSerialOutput::setSerial(Stream &output) {
  pOutput = &output;
}

void PulseSensorSerialOutput::setOutputType(byte outputType) {
  OutputType = outputType;
}

void PulseSensorSerialOutput::outputSample(PulseSensor sensors[], int numSensors) {
  if (!pOutput) {
    return;  // no serial output object has been set.
  }

  switch (OutputType) {
    case SERIAL_PLOTTER:
      if (numSensors == 1) {
        pOutput->print(sensors[0].getBeatsPerMinute());
        pOutput->print(",");
        pOutput->print(sensors[0].getInterBeatIntervalMs());
        pOutput->print(",");
        pOutput->println(sensors[0].getLatestSample());
      } else {
        //TODO: support 2 or more sensors.
      }
      break;
    case PROCESSING_VISUALIZER:
      if (numSensors == 1) {
        outputToSerial('S', sensors[0].getLatestSample());
        // Ignore bpm and ibi for this output.
      } else {
        //TODO: support 2 or more sensors.
      }
      break;
    default:
      // unknown output type: no output
      break;
  }
}

void PulseSensorSerialOutput::outputBeat(PulseSensor sensors[], int numSensors) {
  if (!pOutput) {
    return;  // no serial output object has been set.
  }

  switch (OutputType) {
    case SERIAL_PLOTTER:
      // We've already printed this info in outputSample().
      break;
    case PROCESSING_VISUALIZER:
      if (numSensors == 1) {
        outputToSerial('B', sensors[0].getBeatsPerMinute());
        outputToSerial('Q', sensors[0].getInterBeatIntervalMs());
      } else {
        //TODO: support 2 or more sensors.
      }
      break;
    default:
      // unknown output type: no output
      break;
  }
}

void PulseSensorSerialOutput::outputToSerial(char symbol, int data) {
  if (!pOutput) {
    return;  // no serial output object has been set.
  }

  pOutput->print(symbol);
  pOutput->println(data);
}
