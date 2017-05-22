/*
   Interrupt handling helper functions for PulseSensors.
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
#ifndef PULSE_SENSOR_INTERRUPTS_H
#define PULSE_SENSOR_INTERRUPTS_H

// Macros to link to interrupt disable/enable only if they exist
// The name is long to avoid collisions with Sketch and Library symbols.
#if defined(__arc__)
  // Arduino 101 doesn't have cli() and sei().
#define DISABLE_PULSE_SENSOR_INTERRUPTS
#define ENABLE_PULSE_SENSOR_INTERRUPTS
#else
#define DISABLE_PULSE_SENSOR_INTERRUPTS cli()
#define ENABLE_PULSE_SENSOR_INTERRUPTS sei()
#endif

/*
   (internal to the library)
   Sets up the sample timer interrupt for this Arduino Platform.
   
   Returns true if successful, false if we don't yet support
   the timer interrupt on this Arduino.
*/
boolean PulseSensorPlaygroundSetupInterrupt();


#endif // PULSE_SENSOR_INTERRUPTS_H
