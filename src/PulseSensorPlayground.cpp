/*
   A central Playground object to manage a set of PulseSensors.
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

/*
  Define the "this" pointer for the Inerrupt Service Routine, if used.
  TimerHandler.h will define the Interrupt Service Rooutine
  if hardware timer interrutps are used.
  It is placed here so that happens only once.
*/
#if USE_HARDWARE_TIMER
  PulseSensorPlayground *PulseSensorPlayground::OurThis;
#include "utility/TimerHandler.h"   
#endif

PulseSensorPlayground::PulseSensorPlayground(int numberOfSensors) {
  // Save a static pointer to our playground so the ISR can read it.
#if USE_HARDWARE_TIMER    
  OurThis = this;
#endif

  // Dynamically create the array to minimize ram usage.
  SensorCount = (byte) numberOfSensors;
  Sensors = new PulseSensor[SensorCount];

// set our internal variable to reflect hardware timer use
  UsingHardwareTimer = USE_HARDWARE_TIMER;

#if PULSE_SENSOR_TIMING_ANALYSIS
  // We want sample timing analysis, so we construct it.
  pTiming = new PulseSensorTimingStatistics(MICROS_PER_READ, 500 * 30L);
#endif // PULSE_SENSOR_TIMING_ANALYSIS
}

bool PulseSensorPlayground::PulseSensorPlayground::begin() {

  for (int i = 0; i < SensorCount; ++i) {
    Sensors[i].initializeLEDs();
  }

  // Note the time, for non-interrupt sampling and for timing statistics.
  NextSampleMicros = micros() + MICROS_PER_READ;

  SawNewSample = false;
	Paused = false;

#if PULSE_SENSOR_MEMORY_USAGE
  // Report the RAM usage
  printMemoryUsage();
  // for (;;);   // optional hang.
#endif // PULSE_SENSOR_MEMORY_USAGE

  // Lastly, set up and turn on the interrupts.
  if (UsingHardwareTimer) {
    if (!setupInterrupt()) {
			Paused = true;
      return false;
    }
  }
/*
  Uncomment the next line, and the other two references to timingPin
  in this file as well as PulseSensorPlayground.h.
  Connect the pin to an osilloscope to measure the run time
  of the PulseSensor heartbeat finding algorithm.
  This is useful to know the time it takes every sample.
*/
  // pinMode(timingPin,OUTPUT);
  return true;
}

void PulseSensorPlayground::analogInput(int inputPin, int sensorIndex) {
  if (sensorIndex != constrain(sensorIndex, 0, SensorCount)) {
    return; // out of range.
  }
  Sensors[sensorIndex].analogInput(inputPin);
}

void PulseSensorPlayground::blinkOnPulse(int blinkPin, int sensorIndex) {
  if (sensorIndex != constrain(sensorIndex, 0, SensorCount)) {
    return; // out of range.
  }
  Sensors[sensorIndex].blinkOnPulse(blinkPin);
}

void PulseSensorPlayground::fadeOnPulse(int fadePin, int sensorIndex) {
  if (sensorIndex != constrain(sensorIndex, 0, SensorCount)) {
    return; // out of range.
  }
  Sensors[sensorIndex].fadeOnPulse(fadePin);
}

bool PulseSensorPlayground::sawNewSample() {
  /*
     If using hardware timer, this function reads and clears the
     'saw a sample' flag that is set by the ISR.

     When not using hardware timer, this function sees whether it's time
     to sample and, if so, reads the sample and processes it.

		 First, check to see if the sketch has paused the Pulse Sensor sampling
  */
  bool result = false;
  if(!Paused){
    if (UsingHardwareTimer) {
      // Disable interrupts to avoid a race with the ISR.
      // DISABLE_PULSE_SENSOR_INTERRUPTS;
      bool sawOne = SawNewSample;
      SawNewSample = false;
      // ENABLE_PULSE_SENSOR_INTERRUPTS;

      result = sawOne;
    } else { 
// Sample PulseSensor as close as you can to 500Hz when not using hardware timer
      unsigned long nowMicros = micros();
      if ((long) (NextSampleMicros - nowMicros) > 0L) {
        result = false;  // not time yet.
      }
      NextSampleMicros = nowMicros + MICROS_PER_READ;

    #if PULSE_SENSOR_TIMING_ANALYSIS
      if (pTiming->recordSampleTime() <= 0) {
        pTiming->outputStatistics(SerialOutput.getSerial());
        for (;;); // Hang because we've disturbed the timing.
      }
    #endif // PULSE_SENSOR_TIMING_ANALYSIS

      // time to call the sample processor
      onSampleTime();
      result = true;
  	}
  }
  return result;
}

void PulseSensorPlayground::onSampleTime() {
  // Typically called from the ISR at 500Hz
  // digitalWrite(timingPin,HIGH); // optionally connect timingPin to oscilloscope to time algorithm run time
  /*
     Read the voltage from each PulseSensor.
     We do this separately from processing the samples
     to minimize jitter in acquiring the signal.
  */
  for (int i = 0; i < SensorCount; ++i) {
    Sensors[i].readNextSample();
  }

  // Process those samples.
  for (int i = 0; i < SensorCount; ++i) {
    Sensors[i].processLatestSample();
    Sensors[i].updateLEDs();
  }

  // Set the flag that says we've read a sample since the Sketch checked.
  // digitalWrite(timingPin,LOW); // optionally connect timingPin to oscilloscope to time algorithm run time
 }

int PulseSensorPlayground::getLatestSample(int sensorIndex) {
  if (sensorIndex != constrain(sensorIndex, 0, SensorCount)) {
    return -1; // out of range.
  }
  return Sensors[sensorIndex].getLatestSample();
}

int PulseSensorPlayground::getBeatsPerMinute(int sensorIndex) {
  if (sensorIndex != constrain(sensorIndex, 0, SensorCount)) {
    return -1; // out of range.
  }
  return Sensors[sensorIndex].getBeatsPerMinute();
}

int PulseSensorPlayground::getInterBeatIntervalMs(int sensorIndex) {
  if (sensorIndex != constrain(sensorIndex, 0, SensorCount)) {
    return -1; // out of range.
  }
  return Sensors[sensorIndex].getInterBeatIntervalMs();
}

bool PulseSensorPlayground::sawStartOfBeat(int sensorIndex) {
  if (sensorIndex != constrain(sensorIndex, 0, SensorCount)) {
    return false; // out of range.
  }
  return Sensors[sensorIndex].sawStartOfBeat();
}

bool PulseSensorPlayground::isInsideBeat(int sensorIndex) {
  if (sensorIndex != constrain(sensorIndex, 0, SensorCount)) {
    return false; // out of range.
  }
  return Sensors[sensorIndex].isInsideBeat();
}

void PulseSensorPlayground::setThreshold(int threshold, int sensorIndex) {
  if (sensorIndex != constrain(sensorIndex, 0, SensorCount)) {
    return; // out of range.
  }
  Sensors[sensorIndex].setThreshold(threshold);
}

#if USE_SERIAL

  void PulseSensorPlayground::setSerial(Stream &output) {
    SerialOutput.setSerial(output);
  }

  void PulseSensorPlayground::setOutputType(byte outputType) {
    SerialOutput.setOutputType(outputType);
  }

  void PulseSensorPlayground::outputSample() {
    SerialOutput.outputSample(Sensors, SensorCount);
  }

  void PulseSensorPlayground::outputBeat(int sensorIndex) {
    SerialOutput.outputBeat(Sensors, SensorCount, sensorIndex);
  }

  void PulseSensorPlayground::outputToSerial(char s, int d) {
    SerialOutput.outputToSerial(s,d);
  }

#endif

int PulseSensorPlayground::getPulseAmplitude(int sensorIndex) {
  if (sensorIndex != constrain(sensorIndex, 0, SensorCount)) {
    return -1; // out of range.
  }
  return Sensors[sensorIndex].getPulseAmplitude();
}

unsigned long PulseSensorPlayground::getLastBeatTime(int sensorIndex) {
  if (sensorIndex != constrain(sensorIndex, 0, SensorCount)) {
    return -1; // out of range.
  }
  return Sensors[sensorIndex].getLastBeatTime();
}

bool PulseSensorPlayground::isPaused() {
	return Paused;
}

bool PulseSensorPlayground::pause() {
  bool result = true;
	if (UsingHardwareTimer) {
    if (!disableInterrupt()) {
      Paused = false;
      result = false;
    }else{
			// DOING THIS HERE BECAUSE IT COULD GET CHOMPED IF WE DO IN resume() BELOW
			for(int i=0; i<SensorCount; i++){
				Sensors[i].resetVariables();
			}
			Paused = true;
		}
	}else{
		// do something here?
		for(int i=0; i<SensorCount; i++){
			Sensors[i].resetVariables();
		}
		Paused = true;
	}
  return result;
}

bool PulseSensorPlayground::resume() {
  bool result = true;
	if (UsingHardwareTimer) {
    if (!enableInterrupt()) {
      Paused = true;
      result = false;
    }else{
			Paused = false;
		}
	}else{
		// do something here?
		Paused = false;
	}
  return result;
}

#if USE_SERIAL
  #if PULSE_SENSOR_MEMORY_USAGE
    void PulseSensorPlayground::printMemoryUsage() {
      char stack = 1;
      extern char *__data_start;
      extern char *__data_end;
      extern char *__bss_start;
      extern char *__bss_end;
      extern char *__heap_start;
      extern char *__heap_end;

      int	data_size	=	(int)&__data_end - (int)&__data_start;
      int	bss_size	=	(int)&__bss_end - (int)&__data_end;
      int	heap_end	=	(int)&stack - (int)&__malloc_margin;
      int	heap_size	=	heap_end - (int)&__bss_end;
      int	stack_size	=	RAMEND - (int)&stack + 1;
      int	available	=	(RAMEND - (int)&__data_start + 1);
      available	-=	data_size + bss_size + heap_size + stack_size;

      Stream *pOut = SerialOutput.getSerial();
      if (pOut) {
        pOut->print(F("data "));
        pOut->println(data_size);
        pOut->print(F("bss "));
        pOut->println(bss_size);
        pOut->print(F("heap "));
        pOut->println(heap_size);
        pOut->print(F("stack "));
        pOut->println(stack_size);
        pOut->print(F("total "));
        pOut->println(data_size + bss_size + heap_size + stack_size);
      }
    }
  #endif // PULSE_SENSOR_MEMORY_USAGE
#endif // USE_SERIAL


bool PulseSensorPlayground::setupInterrupt(){
    bool result = false;


#if !USE_HARDWARE_TIMER
  /*
     The Sketch doesn't want interrupts,
     so we won't waste Flash space and create complexity
     by adding interrupt-setup code.
  */
  return result;

#else
  // This code sets up the sample timer interrupt
  // based on the type of Arduino platform.

  #if defined(__AVR_ATmega328P__) || defined(__AVR_ATmega168__) || defined(__AVR_ATmega32U4__) || defined(__AVR_ATmega16U4__)

    // check to see if the Servo library is in use
    #if __has_include (<Servo.h>)
      // Initializes Timer2 to throw an interrupt every 2mS
      // Interferes with PWM on pins 3 and 11
            #if defined(__AVR_ATmega328P__) || defined(__AVR_ATmega168__)
          TCCR2A = 0x02;          // Disable PWM and go into CTC mode
          TCCR2B = 0x05;          // don't force compare, 128 prescaler
          #if F_CPU == 16000000L   // if using 16MHz crystal
            OCR2A = 0XF9;         // set count to 249 for 2mS interrupt
          #elif F_CPU == 8000000L // if using 8MHz crystal
            OCR2A = 0X7C;         // set count to 124 for 2mS interrupt
          #endif
          TIMSK2 = 0x02;          // Enable OCR2A match interrupt DISABLE BY SETTING TO 0x00
          ENABLE_PULSE_SENSOR_INTERRUPTS;
          // #define _useTimer2
          result = true;
            #elif defined(__AVR_ATmega32U4__) || defined(__AVR_ATmega16U4__)
                TCCR3A = 0x02;          // Disable PWM and go into CTC mode
                TCCR3B = 0x05;          // don't force compare, 128 prescaler
                #if F_CPU == 16000000L   // if using 16MHz crystal
                    OCR3A = 0XF9;         // set count to 249 for 2mS interrupt
                #elif F_CPU == 8000000L // if using 8MHz crystal
                    OCR3A = 0X7C;         // set count to 124 for 2mS interrupt
                #endif
                TIMSK3 = 0x02;          // Enable OCR2A match interrupt DISABLE BY SETTING TO 0x00
                ENABLE_PULSE_SENSOR_INTERRUPTS;
                // #define _useTimer2
                result = true;
            #endif
    #else
      // Initializes Timer1 to throw an interrupt every 2mS.
      // Interferes with PWM on pins 9 and 10
      TCCR1A = 0x00;            // Disable PWM and go into CTC mode
      TCCR1C = 0x00;            // don't force compare
      #if F_CPU == 16000000L    // if using 16MHz crystal
        TCCR1B = 0x0C;          // prescaler 256
        OCR1A = 0x007C;         // count to 124 for 2mS interrupt
      #elif F_CPU == 8000000L   // if using 8MHz crystal
        TCCR1B = 0x0B;          // prescaler = 64
        OCR1A = 0x00F9;         // count to 249 for 2mS interrupt
      #endif
      TIMSK1 = 0x02;            // Enable OCR1A match interrupt DISABLE BY SETTING TO 0x00
      ENABLE_PULSE_SENSOR_INTERRUPTS;
      result = true;
    #endif
  #endif

    #if defined(__AVR_ATmega1280__) || defined(__AVR_ATmega2560__)

    // check to see if the Servo library is in use
    #if __has_include (<Servo.h>) 
        // Initializes Timer1 to throw an interrupt every 2mS.
        // Interferes with PWM on pins 9 and 10
        TCCR1A = 0x00;            // Disable PWM and go into CTC mode
        TCCR1C = 0x00;            // don't force compare
        #if F_CPU == 16000000L    // if using 16MHz crystal
            TCCR1B = 0x0C;          // prescaler 256
            OCR1A = 0x007C;         // count to 124 for 2mS interrupt
        #elif F_CPU == 8000000L   // if using 8MHz crystal
            TCCR1B = 0x0B;          // prescaler = 64
            OCR1A = 0x00F9;         // count to 249 for 2mS interrupt
        #endif
        TIMSK1 = 0x02;            // Enable OCR1A match interrupt
        ENABLE_PULSE_SENSOR_INTERRUPTS;
        result = true;

    #else
        // Initializes Timer2 to throw an interrupt every 2mS
        // Interferes with PWM on pins 3 and 11
            TCCR2A = 0x02;          // Disable PWM and go into CTC mode
            TCCR2B = 0x05;          // don't force compare, 128 prescaler
            #if F_CPU == 16000000L   // if using 16MHz crystal
                OCR2A = 0XF9;         // set count to 249 for 2mS interrupt
            #elif F_CPU == 8000000L // if using 8MHz crystal
                OCR2A = 0X7C;         // set count to 124 for 2mS interrupt
            #endif
            TIMSK2 = 0x02;          // Enable OCR2A match interrupt
            ENABLE_PULSE_SENSOR_INTERRUPTS;
            // #define _useTimer2
            result = true;

    #endif
 #endif

  #if defined(__AVR_ATtiny85__)
    GTCCR = 0x00;     // Disable PWM, don't connect pins to events
        OCR1A = 0x7D;           // Set top of count to 125. Timer match throws the interrupt
    OCR1C = 0x7D;     // Set top of the count to 125. Timer match resets the counter
    #if F_CPU == 16000000L
      TCCR1 = 0x89;      // Clear Timer on Compare, Set Prescaler to 256
    #elif F_CPU == 8000000L
      TCCR1 = 0x88;      // Clear Timer on Compare, Set Prescaler to 128
        #elif F_CPU == 1000000L
            TCCR1 = 0x85            // Clear Timer on Compare, Set Prescaler to 16
    #endif
    bitSet(TIMSK,6);   // Enable interrupt on match between TCNT1 and OCR1A
    ENABLE_PULSE_SENSOR_INTERRUPTS;
    result = true;
  #endif

  #if defined(ARDUINO_ARCH_RENESAS)
    uint8_t timer_type = GPT_TIMER;
    int8_t tindex = FspTimer::get_available_timer(timer_type);
    if(tindex == 0){
        FspTimer::force_use_of_pwm_reserved_timer();
        tindex = FspTimer::get_available_timer(timer_type);  
    }
    sampleTimer.begin(TIMER_MODE_PERIODIC, timer_type, tindex, SAMPLE_RATE_500HZ, 0.0f, sampleTimerISR);
    sampleTimer.setup_overflow_irq();
    sampleTimer.open();
    sampleTimer.start();
    result = true;
  #endif

  #if defined(ARDUINO_SAM_DUE)
    sampleTimer.attachInterrupt(sampleTimer_ISR);
    sampleTimer.start(2000); // Calls every period microseconds
    result = true;
  #endif

  #if defined(ARDUINO_ARCH_RP2040)
    /*  This starts the sample timer interrupt
     *  Use pause() and resume() to start and stop sampling on the fly
     *  Check Resources folder in the library for more tools
     */
    sampleTimer.attachInterruptInterval(SAMPLE_INTERVAL_US, sampleTimer_ISR);
    result = true;
  #endif

  #if defined(ARDUINO_NRF52_ADAFRUIT)
    /*  This starts the sample timer interrupt
     *  Use pause() and resume() to start and stop sampling on the fly
     *  Check Resources folder in the library for more tools
     */
    sampleTimer.attachInterruptInterval(TIMER3_INTERVAL_US, Timer3_ISR);
    result = true;
  #endif

  #if defined(ARDUINO_ARCH_ESP32)
    /*
        This will set up and start the timer interrupt on ESP32.
        The interrupt will occur every 2000uS or 500Hz.
    */
    sampleTimer = timerBegin(1000000); // 1MHz ticker, 1uS tick period                
    timerAttachInterrupt(sampleTimer, &onInterrupt);  
    timerAlarm(sampleTimer, 2000, true, 0);    
    result = true;
  #endif

  #if defined(ARDUINO_ARCH_ESP8266)
        ESP8266Timer sampleTimer;
    sampleTimer.setFrequency(500,onInterrupt);
    sampleTimer.restartTimer();
  #endif

  #if defined(ARDUINO_SAMD_ZERO) || defined(ARDUINO_ARCH_SAMD)
    sampleTimer.attachInterrupt(SAMPLE_RATE_500HZ, onInterrupt);
    result = true;
  #endif

#endif // USE_HARDWARE_TIMER

  return result;
} // setup interrupt



bool PulseSensorPlayground::enableInterrupt(){
    bool result = false;
#if USE_HARDWARE_TIMER
    #if defined(__AVR_ATmega328P__) || defined(__AVR_ATmega168__) || defined(__AVR_ATmega32U4__) || defined(__AVR_ATmega16U4__)
    // check to see if the Servo library is in use
    #if __has_include (<Servo.h>)
            #if defined(__AVR_ATmega328P__) || defined(__AVR_ATmega168__)
              DISABLE_PULSE_SENSOR_INTERRUPTS;
          TIMSK2 = 0x02;          // Enable OCR2A match interrupt
          ENABLE_PULSE_SENSOR_INTERRUPTS;
          result = true;
            #elif defined(__AVR_ATmega32U4__) || defined(__AVR_ATmega16U4__)
                DISABLE_PULSE_SENSOR_INTERRUPTS;
                TIMSK3 = 0x02;          // Enable OCR2A match interrupt
                ENABLE_PULSE_SENSOR_INTERRUPTS;
                result = true;
            #endif
    #else
      DISABLE_PULSE_SENSOR_INTERRUPTS;
      TIMSK1 = 0x02;            // Enable OCR1A match interrupt
      ENABLE_PULSE_SENSOR_INTERRUPTS;
      result = true;
    #endif
  #endif

    #if defined(__AVR_ATmega1280__) || defined(__AVR_ATmega2560__)
        // check to see if the Servo library is in use
    #if __has_include (<Servo.h>)
        DISABLE_PULSE_SENSOR_INTERRUPTS;
            TIMSK1 = 0x02;            // Enable OCR1A match interrupt
            ENABLE_PULSE_SENSOR_INTERRUPTS;
            result = true;
    #else
            DISABLE_PULSE_SENSOR_INTERRUPTS;
            TIMSK2 = 0x02;          // Enable OCR2A match interrupt
            ENABLE_PULSE_SENSOR_INTERRUPTS;
            result = true;
    #endif
    #endif

    #if defined(__AVR_ATtiny85__)
        DISABLE_PULSE_SENSOR_INTERRUPTS;
        bitSet(TIMSK,6);   // Enable interrupt on match between TCNT1 and OCR1A
        ENABLE_PULSE_SENSOR_INTERRUPTS;
        result = true;
    #endif

  #if defined(ARDUINO_ARCH_ESP32)
    timerStart(sampleTimer);
    result = true;
  #endif

  #if defined(ARDUINO_ARCH_NRF52840)
    sampleTimer.restartTimer();
    result = true;
  #endif

  #if defined(ARDUINO_ARCH_RP2040)
    sampleTimer.restartTimer();
    result = true;
  #endif

  #if defined(ARDUINO_ARCH_RENESAS)
    sampleTimer.start();
    result = true;
  #endif

  #if defined(ARDUINO_ARCH_ESP8266)
    sampleTimer.restartTimer();
    result = true;
  #endif

  #if defined(ARDUINO_ARCH_SAM)
    sampleTimer.start(2000);
    result = true;
  #endif

#endif
    
return result; 
}


bool PulseSensorPlayground::disableInterrupt(){      
    bool result = false;
#if USE_HARDWARE_TIMER
    #if defined(__AVR_ATmega328P__) || defined(__AVR_ATmega168__) || defined(__AVR_ATmega32U4__) || defined(__AVR_ATmega16U4__)
    // check to see if the Servo library is in use
    #if __has_include (<Servo.h>)
            #if defined(__AVR_ATmega328P__) || defined(__AVR_ATmega168__)
              DISABLE_PULSE_SENSOR_INTERRUPTS;
          TIMSK2 = 0x00;          // Disable OCR2A match interrupt
          ENABLE_PULSE_SENSOR_INTERRUPTS;
          result = true;
            #elif defined(__AVR_ATmega32U4__) || defined(__AVR_ATmega16U4__)
                DISABLE_PULSE_SENSOR_INTERRUPTS;
                TIMSK3 = 0x00;          // Disable OCR2A match interrupt
                ENABLE_PULSE_SENSOR_INTERRUPTS;
                result = true;
            #endif
    #else
      DISABLE_PULSE_SENSOR_INTERRUPTS;
      TIMSK1 = 0x00;            // Disable OCR1A match interrupt
      ENABLE_PULSE_SENSOR_INTERRUPTS;
      result = true;
    #endif
  #endif

    #if defined(__AVR_ATmega1280__) || defined(__AVR_ATmega2560__)
        // check to see if the Servo library is in use
    #if __has_include (<Servo.h>)
        DISABLE_PULSE_SENSOR_INTERRUPTS;
        TIMSK1 = 0x00;            // Disable OCR1A match interrupt
        ENABLE_PULSE_SENSOR_INTERRUPTS;
        result = true;
    #else
        DISABLE_PULSE_SENSOR_INTERRUPTS;
        TIMSK3 = 0x00;          // Disable OCR2A match interrupt
        ENABLE_PULSE_SENSOR_INTERRUPTS;
        result = true;
    #endif
    #endif

    #if defined(__AVR_ATtiny85__)
        DISABLE_PULSE_SENSOR_INTERRUPTS;
        bitClear(TIMSK,6);   // Disable interrupt on match between TCNT1 and OCR1A
        ENABLE_PULSE_SENSOR_INTERRUPTS;
        result = true;
    #endif

  #if defined(ARDUINO_ARCH_ESP32)
    timerStop(sampleTimer);
  #endif

  #if defined(ARDUINO_ARCH_NRF52840)
    sampleTimer.stopTimer();
    result = true;
  #endif

  #if defined(ARDUINO_ARCH_RP2040)
    sampleTimer.stopTimer();
    result = true;
  #endif

  #if defined(ARDUINO_ARCH_RENESAS)
    sampleTimer.stop();
    result = true;
  #endif

  #if defined(ARDUINO_ARCH_ESP8266)
    sampleTimer.stopTimer();
    result = true;
  #endif

  #if defined(ARDUINO_ARCH_SAM)
    sampleTimer.stop();
    result = true;
  #endif

#endif



return result;
} // DisableInterrupt
