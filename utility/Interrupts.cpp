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
#include <PulseSensorPlayground.h>

boolean PulseSensorPlaygroundSetupInterrupt() {
  // No support for Arduino 101 (arc) yet.
#if !defined(__arc__)
  //TODO: attachInterrupt(digitalPinToInterrupt(pin), ISR, mode);	?
  // Initializes Timer2 to throw an interrupt every 2mS.
  TCCR2A = 0x02;     // DISABLE PWM ON DIGITAL PINS 3 AND 11, AND GO INTO CTC MODE
  TCCR2B = 0x06;     // DON'T FORCE COMPARE, 256 PRESCALER
  OCR2A = 0X7C;      // SET THE TOP OF THE COUNT TO 124 FOR 500Hz SAMPLE RATE
  TIMSK2 = 0x02;     // ENABLE INTERRUPT ON MATCH BETWEEN TIMER2 AND OCR2A
  ENABLE_PULSE_SENSOR_INTERRUPTS; // MAKE SURE GLOBAL INTERRUPTS ARE ENABLED
  return true;
#endif

  return false;      // unknown or unsupported platform.
}

// No support for Arduino 101 (arc) yet.
#if !defined(__arc__)
// THIS IS THE TIMER 2 INTERRUPT SERVICE ROUTINE.
// Timer 2 makes sure that we take a reading every 2 miliseconds
ISR(TIMER2_COMPA_vect) {                   // triggered when Timer2 counts to 124
  DISABLE_PULSE_SENSOR_INTERRUPTS;         // disable interrupts while we do this
  
  PulseSensorPlayground::OurThis->onSampleTime();
 
  ENABLE_PULSE_SENSOR_INTERRUPTS;          // enable interrupts when you're done

}// end isr
#endif
