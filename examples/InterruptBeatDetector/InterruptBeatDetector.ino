/*
 * Code to detect pulses from a pulse Oximeter sensor,
 * using an ARM-based interrupt service routine.
 * Not compatible with non-ARM boards such as Arduino 101.
 * 
 * Based on Joel Murphy and Yury Gitman's Pulse Sensor code, at
 * https://github.com/WorldFamousElectronics/PulseSensor_Amped_Arduino
 *
 * Example Pulse Sensors:
 *   Pulse Sensor Amped (https://www.pulsesensor.com)
 *     available through SparkFun, Adafruit, and others.
 *   
 * This code has been tested with the Pulse Sensor Amped
 * and Arduino Uno.
 * 
 * Copyright (c) 2016, 2017 Bradford Needham, North Plains, Oregon, USA
 * @bneedhamia, https://bluepapertech.com
 * Licensed under the MIT License, a copy of which
 * should have been included with this software.
 * 
 * This software is not intended for medical use.
 */
 
#include <PulseSensorPlayground.h>

/*
 * Pinout:
 *   HAS_A_REF = set to true if you have connected
 *     the pulse sensor power pin to A-REF.
 *     If you are powering the pulse sensor below 5V,
 *     you must set HAS_A_REF to true.
 *   PIN_INPUT = Analog Input. Connected to the pulse sensor
 *    purple (signal) wire.
 *   PIN_BLINK = digital Output. Connected to an LED (and 220 ohm resistor)
 *    that will flash on each detected pulse.
 *   PIN_FADE = digital Output. Connected to an LED (and resistor)
 *    that will smoothly fade with each pulse.
 */
const boolean HAS_A_REF = false;
const int PIN_INPUT = A0;
const int PIN_BLINK = 13;    // Pin 13 is the on-board LED
const int PIN_FADE = 5;      // must be a pin that supports PWM. Can't be pin 3 or 11 (see ISR()).

/*
 * Planned time (microseconds) between calls to readSensor().
 * Note: interruptSetup() assumes this value is 2 milliseconds (2000 microseconds).
 */
const unsigned long MICROS_PER_READ = 2 * 1000L;

// PWM steps per fade step.  More fades faster; less fades slower.
const int PWM_STEPS_PER_FADE = 12;

/* 
 * fadePWM = Pulse Width Modulation value for our fading LED
 * (0 = off; 255 = full on)
 */
int fadePWM;

/*
 * The per-sample processing code.
 */
PulseSensorPlayground pulseSensor;

/*
 * If true, we've seen a beat that hasn't yet been printed.
 * Set in the ISR.
 * Cleared in loop().
 */
volatile boolean QS;

void setup() {
  /*
   * Use 115200 baud because that's what the Processing Sketch expects to read,
   * and because that speed provides about 11 bytes per millisecond.
   * 
   * If we used a slower baud rate, we'd likely write bytes faster than
   * they can be transmitted, which would mess up the timing
   * of readSensor() calls, which would make the pulse measurement
   * not work properly.
   */
  Serial.begin(115200);

  // Set up the I/O pins
  
  if (HAS_A_REF) {
    analogReference(EXTERNAL);
  }
  // PIN_INPUT is set up by the pulseDetector constructor.
  pinMode(PIN_BLINK, OUTPUT);
  digitalWrite(PIN_BLINK, LOW);
  pinMode(PIN_FADE, OUTPUT);
  fadePWM = 0;
  analogWrite(PIN_FADE, fadePWM);   // sets PWM duty cycle

  // Create the beat detector
  pulseSensor.beginBeatDetection(PIN_INPUT, MICROS_PER_READ / 1000L);

  QS = false;
  interruptSetup(); // start the interrupt timer.
}

void loop() {
  
  /*
   * Every so often, send the latest Sample to the Processing Sketch.
   * We don't print every sample, because our baud rate
   * won't support that much I/O.
   */
  delay(20);
  
  Serial.print('S');
  Serial.println(pulseSensor.getBeatSignal());

  // Coincidentally, fade the LED a bit.
  fadePWM -= PWM_STEPS_PER_FADE;
  if (fadePWM < 0) {
    fadePWM = 0;
  }
  analogWrite(PIN_FADE, fadePWM);

  // Blink the non-fading LED when the start of a pulse is detected.
  if (pulseSensor.isBeat()) {
    digitalWrite(PIN_BLINK, HIGH);
  } else {
    digitalWrite(PIN_BLINK, LOW);
  }

  // If the ISR has seen a beat, print the per-beat information.
  if (QS) {
    fadePWM = 255;  // start fading on the start of each beat.
    analogWrite(PIN_FADE, fadePWM);
    
    Serial.print('B');
    Serial.println(pulseSensor.getBeatsPerMinute());
    Serial.print('Q');
    Serial.println(pulseSensor.getInterBeatIntervalMs());
    
    QS = false;
  }

}

/*
 * Sets up a Timer Interrupt for every 2ms
 */
void interruptSetup(){     
  // Initializes Timer2 to throw an interrupt every 2mS.
  TCCR2A = 0x02;     // DISABLE PWM ON DIGITAL PINS 3 AND 11, AND GO INTO CTC MODE
  TCCR2B = 0x06;     // DON'T FORCE COMPARE, 256 PRESCALER 
  OCR2A = 0X7C;      // SET THE TOP OF THE COUNT TO 124 FOR 500Hz SAMPLE RATE
  TIMSK2 = 0x02;     // ENABLE INTERRUPT ON MATCH BETWEEN TIMER2 AND OCR2A
  sei();             // MAKE SURE GLOBAL INTERRUPTS ARE ENABLED      
}

// THIS IS THE TIMER 2 INTERRUPT SERVICE ROUTINE. 
// Timer 2 makes sure that we take a reading every 2 miliseconds
ISR(TIMER2_COMPA_vect){                    // triggered when Timer2 counts to 124
  cli();                                   // disable interrupts while we do this
  if (pulseSensor.readPulseSensor()) {
    QS = true;
  }
  sei();                                   // enable interrupts when youre done!
}// end isr


