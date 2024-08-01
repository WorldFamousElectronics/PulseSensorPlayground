
#ifndef TIMERHANDLER_H
#define TIMERHANDLER_H

/*
   Interrupt handling  functions for PulseSensors.
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
   This library will determine the value of USE_HARDWARE_TIMER:
   1) #define USE_HARDWARE_TIMER true - if using interrupts;
   2) #define USE_HARDWARE_TIMER false - if not using interrupts.

   Preprocessor directives will determine if the library supports 
   hardware interrupts for the target hardware platform.

*/


  
//  SAVED FOR FUTURE SUPPORT OF TEENSY INTERRUPTS
#if defined(__MK66FX1M0__)||(__MK64FX512__)||(__MK20DX256__)||(__MK20DX128__)
// #include <FlexiTimer2.h>
#endif


/*
   We create the Interrupt Service Routine only if the library knows
   how to engage them.

   We define the ISR that handles the timer that
   PulseSensorPlaygroundSetupInterrupt() set up.
   NOTE: Make sure that this ISR uses the appropriate timer for
   the platform detected by PulseSensorPlaygroundSetupInterrupt().
*/
    #if defined(__AVR_ATmega328P__) || defined(__AVR_ATmega168__) || defined(__AVR_ATmega32U4__) || defined(__AVR_ATmega16U4__) || defined(__AVR_ATtiny85__)
        #if __has_include (<Servo.h>)
            #warning "Detected Servo library in TimerHandler.h"
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
            #ifndef TIMER_VECTOR
                #define TIMER_VECTOR
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
        #if __has_include (<Servo.h>)
#warning "Detected Servo library in TimerHandler.h"
            #ifndef TIMER_VECTOR
            #define TIMER_VECTOR
            ISR(TIMER2_COMPA_vect)
            {
                DISABLE_PULSE_SENSOR_INTERRUPTS;         // disable interrupts while we do this

                PulseSensorPlayground::OurThis->onSampleTime();

                ENABLE_PULSE_SENSOR_INTERRUPTS;          // enable interrupts when you're done
            }
            #endif
        #else
            #ifndef TIMER_VECTOR
            #define TIMER_VECTOR
            ISR(TIMER1_COMPA_vect)
            {
                DISABLE_PULSE_SENSOR_INTERRUPTS;         // disable interrupts while we do this

                PulseSensorPlayground::OurThis->onSampleTime();

                ENABLE_PULSE_SENSOR_INTERRUPTS;          // enable interrupts when you're done
            }
            #endif
        #endif
    #endif

    #if defined(ARDUINO_ARCH_RENESAS)
        #include "FspTimer.h"
        FspTimer sampleTimer;
        void sampleTimerISR(timer_callback_args_t __attribute((unused)) *p_args){
          PulseSensorPlayground::OurThis->onSampleTime();
        }
    #endif


    #if defined(ARDUINO_SAM_DUE)
        /*
          Include the DueTimer library. If you don't have it, use library manager to get it.
          You can also find it at https://github.com/ivanseidel/DueTimer
          If you use the Servo library, probably want to include that before this inlude. Just sayin'...
          This will grab the next available timer and call it sampleTimer for use throughout the code
        */
        #include <DueTimer.h>
        DueTimer sampleTimer = Timer.getAvailable();
        void sampleTimer_ISR(){ 
          PulseSensorPlayground::OurThis->onSampleTime();
        }
    #endif

    #if defined(ARDUINO_ARCH_RP2040)
        /*
          Include the TimerInterrupt library 
          https://github.com/khoih-prog/RPI_PICO_TimerInterrupt
          Set the sample rate to 500Hz
         */
        #include "RPi_Pico_TimerInterrupt.h"
        #define SAMPLE_INTERVAL_US 2000L
        RPI_PICO_Timer sampleTimer(0); // the paramater may need to change, depending?
        bool sampleTimer_ISR(struct repeating_timer *t){ 
          (void) t;
          PulseSensorPlayground::OurThis->onSampleTime();
          return true;
        }
    #endif

    #if defined(ARDUINO_NRF52_ADAFRUIT)
        /*
            If you are using an Adafruit or Seeed nRF52 platform,
            use this library and ISR
        */
        #include "NRF52TimerInterrupt.h"
        #define TIMER3_INTERVAL_US        2000 // critical fine tuning here!
        NRF52Timer sampleTimer(NRF_TIMER_3);
        void Timer3_ISR(){
          PulseSensorPlayground::OurThis->onSampleTime();
        }
    #endif

    #if defined(ARDUINO_ARCH_ESP32)
        /*
           The hw_timer_t variable type and all the timer tools
           come in the board download so we don't need an external library
        */
        hw_timer_t *sampleTimer = NULL;
        portMUX_TYPE timerMux = portMUX_INITIALIZER_UNLOCKED;
        void ARDUINO_ISR_ATTR onInterrupt() {
          portENTER_CRITICAL_ISR(&timerMux);
            PulseSensorPlayground::OurThis->onSampleTime();
          portEXIT_CRITICAL_ISR(&timerMux);
        }
    #endif

    #if defined(ARDUINO_ARCH_ESP8266)
        /*
            If you are using ESP8266, include the TimerInterrupt library
            https://github.com/khoih-prog/ESP8266TimerInterrupt
        */
        #define USING_TIM_DIV16 true
        #include "ESP8266TimerInterrupt.h"
        
        void IRAM_ATTR onInterrupt(){
          PulseSensorPlayground::OurThis->onSampleTime();
        }
        ESP8266Timer sampleTimer;

    #endif

    #if defined(ARDUINO_SAMD_ZERO) || defined(ARDUINO_ARCH_SAMD)
        // These define's must be placed at the beginning before #include "SAMDTimerInterrupt.h"
        // _TIMERINTERRUPT_LOGLEVEL_ from 0 to 4
        // Don't define _TIMERINTERRUPT_LOGLEVEL_ > 0. Only for special ISR debugging only. Can hang the system.
        // Don't define TIMER_INTERRUPT_DEBUG > 2. Only for special ISR debugging only. Can hang the system.
        #define TIMER_INTERRUPT_DEBUG         0
        #define _TIMERINTERRUPT_LOGLEVEL_     1
        // Select only one to be true for SAMD21. Must must be placed at the beginning before #include "SAMDTimerInterrupt.h"
        #define USING_TIMER_TC3         true      // Only TC3 can be used for SAMD51
        #define USING_TIMER_TC4         false     // Not to use with Servo library
        #define USING_TIMER_TC5         false
        #define USING_TIMER_TCC         false
        #define USING_TIMER_TCC1        false
        #define USING_TIMER_TCC2        false     // Don't use this, can crash on some boards

        #include "SAMDTimerInterrupt.h"

        #if USING_TIMER_TC3
        #define SELECTED_TIMER      TIMER_TC3
        #endif
        // Define selected SAMD timer and set up ISR
        SAMDTimer sampleTimer(SELECTED_TIMER);
        void onInterrupt(){
          PulseSensorPlayground::OurThis->onSampleTime();
        }
    #endif
        

    #if defined(__MK66FX1M0__)||(__MK64FX512__)||(__MK20DX256__)||(__MK20DX128__)
        // Interrupts here for Teensy in future
    #endif



#endif // SANDBOX_H