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
  #if defined(__AVR_ATmega328P__) || defined(__AVR_ATmega168__) || defined(__AVR_ATmega32U4__) || defined(__AVR_ATmega16U4__)
    // Initializes Timer1 to throw an interrupt every 2mS.
    // Interferes with PWM on pins 9 and 10
    TCCR1A = 0x00; // DISABLE OUTPUTS AND PWM ON DIGITAL PINS 9 & 10
    TCCR1C = 0x00; // DON'T FORCE COMPARE
    #if F_CPU == 16000000L   //  if using 16MHz crystal
      TCCR1B = 0x0C; // GO INTO 'CTC' MODE, PRESCALER = 265
      OCR1A = 0x007C;  // TRIGGER TIMER INTERRUPT EVERY 2mS
    #elif F_CPU == 8000000L  // if using 8MHz crystal
      TCCR1B = 0x0B; // prescaler = 64
      OCR1A = 0x00F9;  // count to 249 for 2mS interrupt
    #endif
    TIMSK1 = 0x02; // ENABLE OCR1A MATCH INTERRUPT
    ENABLE_PULSE_SENSOR_INTERRUPTS;
    return true;


  #elif defined(__AVR_ATtiny85__)
    GTCCR &= 0x81;     // Disable PWM, don't connect pins to events
    OCR1C = 0x7C;      // Set the top of the count to  124 TEST VALUE
    OCR1A = 0x7C;      // Set the timer to interrupt after counting to TEST VALUE
    #if F_CPU == 16000000L
      TCCR1 = 0x88;      // Clear Timer on Compare, Set Prescaler to 128 TEST VALUE
    #elif F_CPU == 8000000L
      TCCR1 = 0x89;      // Clear Timer on Compare, Set Prescaler to 128 TEST VALUE
    #endif
    bitSet(TIMSK,6);   // Enable interrupt on match between TCNT1 and OCR1A
    ENABLE_PULSE_SENSOR_INTERRUPTS;
    return true;

  #else
    return false;      // unknown or unsupported platform.
  #endif
}

#if defined(__AVR__)
ISR(TIMER1_COMPA_vect)
{
  DISABLE_PULSE_SENSOR_INTERRUPTS;         // disable interrupts while we do this
  
  PulseSensorPlayground::OurThis->onSampleTime();
 
  ENABLE_PULSE_SENSOR_INTERRUPTS;          // enable interrupts when you're done
}
#endif

