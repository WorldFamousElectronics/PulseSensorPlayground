/*
 * PulseSensor serial output class.
 * 
 * See https://www.pulsesensor.com
 * and https://github.com/WorldFamousElectronics/PulseSensor_Amped_Arduino
 * 
 * Based on Pulse Sensor Amped 1.5.0 by Joel Murphy and Yury Gitman.
 * Portions Copyright (c) 2016, 2017 Bradford Needham, North Plains, Oregon, USA
 * @bneedhamia, https://bluepapertech.com
 * 
 * Licensed under the MIT License, a copy of which
 * should have been included with this software.
 * 
 * This software is not intended for medical use.
 */
#ifndef PULSE_SENSOR_SERIAL_OUTPUT_H
#define PULSE_SENSOR_SERIAL_OUTPUT_H

#include <Arduino.h>

/*
 * Destinations for serial output:
 * PROCESSING_VISUALIZER = write to the Processing Visualizer Sketch.
 * SERIAL_PLOTTER = write to the Arduino IDE Serial Plotter.
 */
#define PROCESSING_VISUALIZER 1
#define SERIAL_PLOTTER 2

class PulseSensorSerialOutput {
  public:
    PulseSensorSerialOutput(int outputType);
    void output(int signal, int bpm, int ibi);
    void outputBeat(int bpm, int ibi);
  
  private:
    /*
     * The destination of data:
     * PROCESSING_VISUALIZER or SERIAL_PLOTTER
     */
    int OutputType;
    
    void outputToSerial(char symbol, int data);
};
#endif // PULSE_SENSOR_SERIAL_OUTPUT_H
