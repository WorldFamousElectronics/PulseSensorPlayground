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

#include <PulseSensorSerialOutput.h>

/*
 * Constructs an object that will output serial data to the given destination.
 * 
 * outputType = PROCESSING_VISUALIZER or SERIAL_PLOTTER.
 */
PulseSensorSerialOutput::PulseSensorSerialOutput(int outputType) {
  OutputType = outputType;
}

/*
 * Outputs the given signal value, and optionally the heartbeat rate.
 * 
 * signal = the analogRead() values from the PulseSensor (0..1023).
 * bpm = the current beats per minute calculation result.
 * ibi = the inter-beat-interval, in milliseconds, between the previous beat
 * and the current one.
 */
void PulseSensorSerialOutput::output(int signal, int bpm, int ibi) {
  switch (OutputType) {
  case PROCESSING_VISUALIZER:
    outputToSerial('S', signal);
    // Ignore bpm and ibi for this output.
    break;
    
  case SERIAL_PLOTTER:
    Serial.print(bpm);
    Serial.print(",");
    Serial.print(ibi);
    Serial.print(",");
    Serial.println(signal);
    break;
    
  default: // Unknown output type. Do nothing.
    break;
  }
}

/*
 * Output the heartbeat timing measurement lines.
 * Usually called on detection of a beat.
 * Currently appropriate only for PROCESSING_VISUALIZER output.
 * 
 * bpm = the current beats per minute calculation result.
 * ibi = the inter-beat-interval, in milliseconds, between the previous beat
 * and the current one.
 */
void PulseSensorSerialOutput::outputBeat(int bpm, int ibi) {
  outputToSerial('B', bpm);
  outputToSerial('Q', ibi);
}

/*
 * Output a single piece of information, along with a single character
 *  that says what data it is, and and end-of-line.
 * 
 * symbol = the one-character variable name. For the PROCESSING_VISUALIZER,
 *   possible values are:
 *   'S' = signal (PulseSensor data),
 *   'B' = Beats per minute,
 *   'Q' = IBI (Inter-beat interval), in milliseconds.
 */
void PulseSensorSerialOutput::outputToSerial(char symbol, int data) {
  Serial.print(symbol);
  Serial.println(data);
}
