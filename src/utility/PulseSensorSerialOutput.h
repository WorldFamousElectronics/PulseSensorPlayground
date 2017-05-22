/*
   Serial output formatter for the PulseSensor Playground.
   This object knows all about the formats for our Serial output.
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
#ifndef PULSE_SENSOR_SERIAL_OUTPUT_H
#define PULSE_SENSOR_SERIAL_OUTPUT_H

#include <Arduino.h>
#include "PulseSensor.h" // to access PulseSensor state.

/*
   Destinations for serial output:
   PROCESSING_VISUALIZER = write to the Processing Visualizer Sketch.
   SERIAL_PLOTTER = write to the Arduino IDE Serial Plotter.
*/
#define PROCESSING_VISUALIZER ((byte) 1)
#define SERIAL_PLOTTER ((byte) 2)

class PulseSensorSerialOutput {
  public:

    /*
       Constructs a default Serial output manager.
    */
    PulseSensorSerialOutput();
    
    /*
       Tells the library what Serial output to use,
       such as Serial, Serial1, or a SoftwareSerial.
    */
    void setSerial(Stream &output);

    /*
       Sets the format (destination) of the Serial Output:
       SERIAL_PLOTTER or PROCESSING_VISUALIZER.
    */
    void setOutputType(byte outputType);

    /*
       Output the Signal data for all PulseSensors
    */
    void outputSample(PulseSensor sensors[], int numberOfSensors);

    /*
       Output the per-beat data (Beats per Minute, Inter-beat Interval)
       for all PulseSensors
    */
    void outputBeat(PulseSensor sensors[], int numberOfSensors);

  private:
    // If non-null, the output stream to print to. If null, don't print.
    Stream *pOutput;
    
    // The destination of data: PROCESSING_VISUALIZER or SERIAL_PLOTTER
    int OutputType;

    // Write the given data prefixed by the given symbol.
    void outputToSerial(char symbol, int data);
};
#endif // PULSE_SENSOR_SERIAL_OUTPUT_H
