
#ifndef SANDBOX_H
#define SANDBOX_H
#pragma once
			// #warning "dragon"

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

/*
   Any Sketch using the Playground must do one of two things:
   1) #define USE_ARDUINO_INTERRUPTS true - if using interrupts;
   2) #define USE_ARDUINO_INTERRUPTS false - if not using interrupts.

   Only the Sketch must define USE_ARDUINO_INTERRUPTS.
   If the Sketch doesn't define USE_ARDUINO_INTERRUPTS, or if some other file
   defines it as well, a link error will result.

   See notes in PulseSensorPlayground.h

   The code below is rather convoluted, with nested #if's.
   This structure is used to achieve two goals:
   1) Minimize the complexity the user has to deal with to use or
      not use interrupts to sample the PulseSensor data;
   2) Create an ISR() only if the Sketch uses interrupts.  Defining an
      ISR(), even if not used, may interfere with other libraries' use
      of interrupts.

   The nesting goes something like this:
     if the Sketch is being compiled...              #if defined(USE_ARDUINO_INTERRUPTS)
       if the user wants to use interrupts...        #if USE_ARDUINO_INTERRUPTS
         #if's for the various Arduino platforms...  #if defined(__AVR_ATmega328P__)...

   RULES of the constant USE_ARDUINO_INTERRUPTS:
   1) This file, interrupts.h, should be the only file that uses USE_ARDUINO_INTERRUPTS
     (although PulseSensorPlayground's comments talk about it to the user).
     If other code in the library wants to know whether interrupts are being used,
     that code should use PulseSensorPlayground::UsingInterrupts, which is true
     if the Sketch wants to use interrupts.
   1) Always use #if USE_ARDUINO_INTERRUPTS inside an #if defined(USE_ARDUINO_INTERRUPTS).
      If you don't first test the #if defined(...), a compile error will occur
      when compiling the library modules.
   2) USE_ARDUINO_INTERRUPTS is defined only when this file is being included
      by the user's Sketch; not when the rest of the library is compiled.
   3) USE_ARDUINO_INTERRUPTS is true if the user wants to use interrupts;
      it's false if they don't.
*/

// #ifndef PULSE_SENSOR_INTERRUPTS_H
// #define PULSE_SENSOR_INTERRUPTS_H


//TODO: if noInterrupts() and interrupts() are defined for Arduino 101,
// Use them throughout and eliminate these DISABLE/ENAGLE macros.
//

  
//  SAVED FOR FUTURE SUPPORT OF TEENSY INTERRUPTS
#if defined(__MK66FX1M0__)||(__MK64FX512__)||(__MK20DX256__)||(__MK20DX128__)
// #include <FlexiTimer2.h>
#endif


/*
   We create the Interrupt Service Routine only if the Sketch is
   using interrupts. If we defined it when we didn't use it,
   the ISR() will inappropriately intercept timer interrupts that
   we don't use when not using interrupts.

   We define the ISR that handles the timer that
   PulseSensorPlaygroundSetupInterrupt() set up.
   NOTE: Make sure that this ISR uses the appropriate timer for
   the platform detected by PulseSensorPlaygroundSetupInterrupt(), above.
*/
    #if defined(__AVR_ATmega328P__) || defined(__AVR_ATmega168__) || defined(__AVR_ATmega32U4__) || defined(__AVR_ATmega16U4__) || defined(__AVR_ATtiny85__)
      #if defined Servo_h
            #if defined(__AVR_ATmega328P__) || defined(__AVR_ATmega168__)
                #ifndef TIMER_VECTOR
                #define TIMER_VECTOR
                ISR(TIMER2_COMPA_vect)
                {
                  DISABLE_PULSE_SENSOR_INTERRUPTS;         // disable interrupts while we do this

                  PulseSensorPlayground::OurThis->onSampleTime();

                  ENABLE_PULSE_SENSOR_INTERRUPTS;          // enable interrupts when you're done
                }
                #endif
            #elif defined(__AVR_ATmega32U4__) || defined(__AVR_ATmega16U4__)
                #ifndef TIMER_VECTOR
                #define TIMER_VECTOR
                ISR(TIMER3_COMPA_vect)
                {
                    DISABLE_PULSE_SENSOR_INTERRUPTS;         // disable interrupts while we do this

                    PulseSensorPlayground::OurThis->onSampleTime();

                    ENABLE_PULSE_SENSOR_INTERRUPTS;          // enable interrupts when you're done
                }
                #endif
            #endif
      #else
            // #warning "dragons"
        #ifndef TIMER_VECTOR
        #define TIMER_VECTOR
            // #warning "dragons"
        ISR(TIMER1_COMPA_vect)
        {
          DISABLE_PULSE_SENSOR_INTERRUPTS;         // disable interrupts while we do this

          PulseSensorPlayground::OurThis->onSampleTime();

          ENABLE_PULSE_SENSOR_INTERRUPTS;          // enable interrupts when you're done
        }
        #endif
      #endif
    #endif

    #if defined(__AVR_ATmega1280__) || defined(__AVR_ATmega2560__)
        #if defined Servo_h
            #ifndef TIMER_VECTOR
            #define TIMER_VECTOR
            ISR(TIMER1_COMPA_vect)
            {
                DISABLE_PULSE_SENSOR_INTERRUPTS;         // disable interrupts while we do this

                PulseSensorPlayground::OurThis->onSampleTime();

                ENABLE_PULSE_SENSOR_INTERRUPTS;          // enable interrupts when you're done
            }
            #endif
        #else
            #ifndef TIMER_VECTOR
            #define TIMER_VECTOR
            ISR(TIMER2_COMPA_vect)
            {
                DISABLE_PULSE_SENSOR_INTERRUPTS;         // disable interrupts while we do this

                PulseSensorPlayground::OurThis->onSampleTime();

                ENABLE_PULSE_SENSOR_INTERRUPTS;          // enable interrupts when you're done
            }
            #endif
        #endif
    #endif

    

    #if defined(__MK66FX1M0__)||(__MK64FX512__)||(__MK20DX256__)||(__MK20DX128__)
        // Interrupts not supported yet for Teensy
    #endif



#endif // SANDBOX_H