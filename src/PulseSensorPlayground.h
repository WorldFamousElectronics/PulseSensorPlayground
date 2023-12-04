/*
   A central Playground object to manage a set of PulseSensors.
   See https://www.pulsesensor.com to get started.

   Copyright World Famous Electronics LLC - see LICENSE -JM why not list the copyright here?
   Contributors:
     Joel Murphy, https://pulsesensor.com
     Yury Gitman, https://pulsesensor.com
     Bradford Needham, @bneedhamia, https://bluepapertech.com

   Licensed under the MIT License, a copy of which
   should have been included with this software.

   This software is not intended for medical use.
*/

/*
vvvvvvvvv  THIS CAN BE REMOVED FOR V2 vvvvvvvv
  NOTE: Every Sketch that uses the PulseSensor Playground
  must define the variable USE_HARDWARE_TIMER *before* including
  PulseSensorPlayground.h. If you don't, you will get a compiler error
  about "undefined reference to `PulseSensorPlayground::UsingHardwareTimer".

  In particular, if your Sketch wants the Playground to use interrupts
  to read and process PulseSensor data, your Sketch must contain the
  following two lines, in order:
    #define USE_HARDWARE_TIMER true
    #include <PulseSensorPlayground.h>

  If, instead, your Sketch does not use interrupts to read PulseSensor
  data,  your Sketch must instead contain the
  following two lines, in order:
    #define USE_HARDWARE_TIMER false
    #include <PulseSensorPlayground.h>

  See utility/interrupts.h for details.

  Internal, developer note: in the Playground code, don't use
  USE_HARDWARE_TIMER as a variable; instead, refer to
  PulseSensorPlayground::UsingHardwareTimer, which is a static variable
  that reflects what the Sketch defined USE_HARDWARE_TIMER to.
  Because USE_HARDWARE_TIMER is defined *only* in the user's Sketch,
  it doesn't exist when the various Playground modules are compiled.

  See further notes in interrupts.h
^^^^^^^^^  THIS CAN BE REMOVED FOR V2 ^^^^^^^^^^^
*/

#pragma once

#ifndef PULSE_SENSOR_PLAYGROUND_H
#define PULSE_SENSOR_PLAYGROUND_H

// #include "SelectTimer.h"


#define USE_HARDWARE_TIMER true

/*
   If you wish to perform timing statistics on your non-interrupt Sketch:

   Uncomment the line below: #define PULSE_SENSOR_TIMING_ANALYSIS true
   Compile and download your Sketch.
   Start the Arduino IDE Serial Monitor.
   Wait about 30 seconds. The Sketch should then print 3 numbers and hang.
   The three numbers are:
     Minimum variation (microseconds) from the 2 millisecond sample time.
     Average variation in that number.
     Maximum variation in that number.
   For example and output of -4 0 18 says that samples were made between
   4 microseconds short of 2 milliseconds, and 18 microseconds longer,
   with an average sample time right at 2 milliseconds (0 microseconds offset).

   If the average number is larger than, say, 50 microseconds, your Sketch
   is taking too much time per loop(), causing inaccuracies in the
   measured signal, heart rate, and inter-beat interval.

   You should aim for an average offset of under 50 microseconds.

   NOTES:

   1) This is an approximate measure, because interrupts can occur that
   the timing statistics cannot measure.

   2) These statistics compile only for non-interrupt Sketches. If your
   Sketch uses Interrupts to sample the PulseSensor signal, enabling
   this timing analysis will have no effect and will print nothing.

   3) Because timing analysis results are printed on Serial, you cannot
   use the Arduino IDE Serial Plotter or the Processing Visualizer to
   examine output when timing analysis is enabled.

   4) If the average is a negative number, your assumed Arduino clock
   speed may be incorrect. For example, if you compiled for an 8MHz clock
   and your Arduino actually runs at 16MHz, you would likely see an
   average offset of something like -1000.

*/
#define PULSE_SENSOR_TIMING_ANALYSIS false
//#define PULSE_SENSOR_TIMING_ANALYSIS true

/*
   If you wish to print the amount of memory used before your Sketch
   starts:

   Uncomment the line below: #define PULSE_SENSOR_MEMORY_USAGE true
   Compile and download your Sketch.
   Start the Arduino IDE Serial Monitor
   Your Sketch will start normally, then print memory usage, then hang.

   The memory usage consists of five numbers:
   data = bytes of global, uninitialized data storage (int x;)
   bss = bytes of global, initialized data storage (int x = 5;)
   heap = bytes of dynamically allocated memory (new Stream;)
   stack = bytes of local variables (those defined within a function)
   total = the total of data, bss, heap, and stack memory used.

   The amount of flash memory used is printed by the Arduino IDE
   when compilation finishes, with a printout such as:
     Sketch uses 5036 bytes (15%) of program storage space.

   NOTE: you must call pulse.setSerial(Serial) in your Sketch's setup().
*/
#define PULSE_SENSOR_MEMORY_USAGE false
//#define PULSE_SENSOR_MEMORY_USAGE true

/*
    Tell the compiler not to include Serial related code.
    If you are coming up against issues with the Serial class,
    or have no need for the library's Serial output,
    you can remove the related code here.
    
    When true, the library will mangage Serial output,
    and messages from the library will be sent to the Serial specified in the .ino file.
    
    When false, the library will not have any access to the Serial
    class. All the other functionality remains. 
*/
#define USE_SERIAL true
// #define USE_SERIAL false 


#if defined(ARDUINO_NRF52_ADAFRUIT)
#include "Adafruit_TinyUSB.h"
#endif
#include <Arduino.h>
#include "utility/PulseSensor.h"
#if USE_SERIAL
#include "utility/PulseSensorSerialOutput.h"
#endif
#include "utility/PulseSensorTimingStatistics.h"

#define SAMPLE_RATE_500HZ 500
#define SAMPLES_PER_SERIAL_SAMPLE 10

class PulseSensorPlayground {
  public:
    /*
       The number of microseconds per sample of data from the PulseSensor.
       1 millisecond is 1,000 microseconds.

       Refer to this value as PulseSensorPlayground::MICROS_PER_READ
    */
    static const unsigned long MICROS_PER_READ = (2 * 1000L); // usecs per sample.

    //---------- PulseSensor Manager functions

    /*
       Construct the one PulseSensor Playground manager,
       that manages the given number of PulseSensors.
       Your Sketch should declare either PulseSensorPlayground() for one sensor
       or PulseSensorPlayground(n) for n PulseSensors.

       For example:
         PulseSensorPlayground pulse();
       or
         PulseSensorPlayground pulse(2); // for 2 PulseSensors.
    */
    PulseSensorPlayground(int numberOfSensors = 1);

    /*
       Start reading and processing data from the PulseSensors.

       Your Sketch should make all necessary PulseSensor configuration calls
       before calling begin().

vvvvvvvv  THIS CAN BE REMOVED FOR V2 vvvvvvvv
       If the Sketch defined USE_HARDWARE_TIMER as true, this function
       sets up and turns on interrupts for the PulseSensor.

       If instead the Sketch defined USE_HARDWARE_TIMER as false,
       it initializes what's necessary for the Sketch to process
       PulsSensor signals. See sawNewSample(), below.

       Returns true if successful, false if unsuccessful.
       Returns false if PulseSensorPlayground doesn't yet support
       interrupts on this Arduino platform and the user's Sketch
       did a #define USE_HARDWARE_TIMER true.

       If begin() returns false, you can either use a different
       type of Arduino platform, or you can change your Sketch's
       definition of USE_HARDWARE_TIMER to false:
         #define USE_HARDWARE_TIMER false
^^^^^^^^  THIS CAN BE REMOVED FOR V2 ^^^^^^^^
    */
    boolean begin();

    /*

vvvvvvvv  THIS NEEDS MODIFICATION FOR V2 vvvvvvvv
       Returns true if a new sample has been read from each PulseSensor.
       You'll likely want to add this call to your Sketch's loop()
       only if you either 1) want to do something with each sample of the
       PulseSensor signals, or 2) your Sketch doesn't use interrupts
       to read from the PulseSensors.

       NOTE: If your Sketch defined USE_HARDWARE_TIMER as false,
       you must call pulse.sawNewSample() frequently (at least
       once every 2 milliseconds) to assure that PulseSensor signals
       are read accurately.
       A typical loop() that doesn't use interrupts will contain:
         if (pulse.sawNewSample()) {
           int latest = pulse.getLatestSample();
           ...do whatever you want with the sample read from the PulseSensor.
         }

^^^^^^^^  THIS NEEDS MODIFICATION FOR V2 ^^^^^^^^
    */
    boolean sawNewSample();

    //---------- Per-PulseSensor functions

    /*
       By default, the Playground assumes the PulseSensor is connected to A0.
       If your PulseSensor is connected to a different analog input pin,
       call pulse.analogInput(pin) or pulse.analogInput(pin, sensorIndex).

       inputPin = the analog input this PulseSensor is connected to.
       sensorIndex = optional, index (0..numberOfSensors - 1).
    */
    void analogInput(int inputPin, int sensorIndex = 0);

    /*
       By default, the Playground doesn't blink LEDs automatically.

       If you wish the Playground to automatically blink an LED
       during each detected pulse,
       call pulse.blinkOnPulse(blinkPin) or
       pulse.blinkOnPulse(blinkPin, sensorIndex).

       blinkPin = the pin to blink on each pulse, which you've connected
         to an LED and 1K ohm resistor, or the built in LED pin
         on your Arduino (for example, pin 13 on Arduino Uno).
       sensorIndex = optional, index (0..numberOfSensors - 1).
    */
    void blinkOnPulse(int blinkPin, int sensorIndex = 0);

    /*
       By default, the Playground doesn't fade LEDs automatically.

       If you wish the Playground to automatically fade an LED
       during each detected pulse, call fadeOnPulse(fadePin)
       or fadeOnPulse(fadePin, sensorIndex).

       NOTE: the fade pin must be able to output PWM (Pulse-Width Modulation).

       fadePin = the PWM pin to blink and fade on each pulse,
         which is connected to an LED and a current-limit resistor.
       sensorIndex = optional, index (0..numberOfSensors - 1).
    */
    void fadeOnPulse(int fadePin, int sensorIndex = 0);

    /*
       Perform all the processing necessary when it's time to
       read from all the PulseSensors and process their signals.
       This function is not called by the user, but in some cases
       the sketch needs to associate it with other code above the setup.
    */
    void onSampleTime();

    /*
       Returns the most recently read analog value from the given PulseSensor
       (range: 0..1023).

       sensorIndex = optional, index (0..numberOfSensors - 1).
    */
    int getLatestSample(int sensorIndex = 0);

    /*
       Returns the latest beats-per-minute measure for the given PulseSensor.

       The internal beats-per-minute measure is updated per-PulseSensor,
       when a beat is detected from that PulseSensor.

       sensorIndex = optional, index (0..numberOfSensors - 1).
    */
    int getBeatsPerMinute(int sensorIndex = 0);

    /*
       Returns the latest IBI (inter-beat interval, in milliseconds) measure
       for the given PulseSensor.

       The internal IBI measure is updated per-PulseSensor,
       when a beat is detected from that PulseSensor.

       sensorIndex = optional, index (0..numberOfSensors - 1).
    */
    int getInterBeatIntervalMs(int sensorIndex = 0);

    /*
       Returns true if the moment of a new heartbeat (pulse) has been detected
       since the last time sawStartOfBeat() was called on this PulseSensor.

       Typical use in loop():
         if (pulse.sawStartOfBeat()) {
           ...do what you want to do when there is a heartbeat.
         }

       sensorIndex = optional, index (0..numberOfSensors - 1).
    */
    boolean sawStartOfBeat(int sensorIndex = 0);

    /*
       Returns true if the given PulseSensor signal is currently
       inside a heartbeat. Inside a heartbeat means, the signal is above
       the dynamically-set threshold of a beat. When the signal goes below 
       the threshold, isInsiceBeat will return false.

       Typical use in loop():
         if (pulse.isInsideBeat()) {
           ...do what you want while in the beat.
         } else {
           ...do what you want while between beats.
         }

       sensorIndex = optional, index (0..numberOfSensors - 1).
    */
    boolean isInsideBeat(int sensorIndex = 0);

    /*
       By default, the threshold value is 530.  JM- is this true?
       threshold value is used to find the heartbeat.
       PulseSensor signal idles at V/2 (512 analog value on a 10 bit ADC)
       It is recommended to set this value above the idle threshold.
       adjust this value in the setup function to avoid noise.
    */
    void setThreshold(int threshold, int sensorIndex = 0);


    //---------- Serial Output functions
#if USE_SERIAL
    /*
       By default, the Playround doesn't output serial data automatically.

       If you want to output serial pulse data, call pulse.setSerial(Serial),
       pulse.setSerial(Serial1), or whatever Serial stream you like.

       output = the Stream to write data to. Serial, Serial1, Serial2,
       etc., and a SoftwareSerial are valid parameters to pass.
    */
    void setSerial(Stream &output);

    /*
       By default, Playground output is in SERIAL_PLOTTER format.
       In SERIAL_PLOTTER format, data will be sent to the Serial port
       in CSV format. Data include the PulseSensor raw signal,
       Interbeat Interval, and BPM will print with every heartbeat.

       If you want output in a different format, call this function once
       sometime before calling pulse.begin().

       Remember to call pulse.setSerial() if you want serial output.

       outputType = SERIAL_PLOTTER to output to the Arduino Serial Plotter,
       PROCESSSING_VISUALIZER to output formatted to our data visualization 
       software written in Processing. See www.pulsesensor.com for tutorials.
    */
    void setOutputType(byte outputType);

    /*
       Output the current raw signal data for each PulseSensor,
       in the previously-set outputType.

       If your Sketch wants to plot raw sample data, it should call this function
       every so often.
    */
    void outputSample();

    /*
       Output the current per-beat information for each PulseSensor,
       in the previously-set outputType.

       If your Sketch wants to plot beat information, it should call this
       function every time a beat is detected.

       Typical use:
         if (pulse.sawStartOfBeat()) {
           pulse.outputBeat();
         }
    */
    void outputBeat(int sensorIndex = 0);

    /*
       Serial print data with prefix.
       Used exclusively with the Pulse Sensor Processing sketch.
    */
    void outputToSerial(char symbol, int data);
#endif

    /*
        Returns the current amplitude of the pulse waveform.
        Amplitude is calculated after a heartbeat is detected.
        the value is the ADC counts from the previous trough
        to the following peak in the raw sensor data.
    */
    int getPulseAmplitude(int sensorIndex = 0);

    /*
       Returns the sample number when the last beat was found. 2mS resolution.
       The sample number will count up continually.
       As an unsigned long variable, it will roll-over in approx 100 days free running.
    */
    unsigned long getLastBeatTime(int sensorIndex = 0);

    

	/*
        Returns true if PulseSensor sampling is paused, false if it is sampling 

    */
	boolean isPaused();

    /*
	   Pause PulseSensor sampling in order to do other stuff.
       This will effect all PulseSensors if you are using more than one.
	   
    */
	boolean pause();

	/*
        Resume sampling the PulseSensor after a call to pause().
        This will effect all PulseSensors if you are using more than one.
    */
	boolean resume();


    // (internal to the library) "this" pointer for the ISR.
    static PulseSensorPlayground *OurThis;

    byte samplesUntilReport = SAMPLES_PER_SERIAL_SAMPLE;

  private:

/*
   Optionally use this (or a different) pin to toggle high
   while the beat finding algorithm is running.
   Uncomment this line and connect the pin to an oscilloscope
   to measure algorithm run time.
*/
    // int timingPin = 10;

/*
    Print the memory usage of the compiled program.
    Only do this if the Serial class is enabled.
*/
#if USE_SERIAL
    #if PULSE_SENSOR_MEMORY_USAGE
        /*
           Print our RAM usage. See PULSE_SENSOR_MEMORY_USAGE
        */
        void printMemoryUsage();
    #endif // PULSE_SENSOR_MEMORY_USAGE
#endif

/*
   (internal to the library)
   Sets up the sample timer interrupt for this Arduino Platform.

   Returns true if successful, false if we don't yet support
   the timer interrupt on this Arduino.

   This could be included, or not, based on USE_HARDWARE_TIMER
*/
boolean setupInterrupt();
boolean disableInterrupt();
boolean enableInterrupt();

/*
   Varialbles used
*/
    boolean UsingHardwareTimer;
	boolean Paused;
    byte SensorCount;              // number of PulseSensors in Sensors[].
    PulseSensor *Sensors;          // use Sensors[idx] to access a sensor.
    volatile unsigned long NextSampleMicros; // Desired time to sample next.
    volatile boolean SawNewSample; // "A sample has arrived from the ISR"
#if USE_SERIAL
    PulseSensorSerialOutput SerialOutput; // Serial Output manager.
#endif // USE_SERIAL
#if PULSE_SENSOR_TIMING_ANALYSIS   // Don't use ram and flash we don't need.
    PulseSensorTimingStatistics *pTiming;
#endif // PULSE_SENSOR_TIMING_ANALYSIS

#if defined(ARDUINO_ARCH_RENESAS)
    uint8_t timer_type;
    int8_t tindex;
#endif

};


// Macros to link to interrupt disable/enable only if they exist
// The name is long to avoid collisions with Sketch and Library symbols.
#if defined(__arc__)||(ARDUINO_SAMD_MKR1000)||(ARDUINO_SAMD_MKRZERO)||(ARDUINO_SAMD_ZERO)\
||(ARDUINO_ARCH_SAMD)||(ARDUINO_ARCH_STM32)||(ARDUINO_STM32_STAR_OTTO)||(ARDUINO_ARCH_NRF52)\
||(ARDUINO_NANO33BLE)||(ARDUINO_ARCH_RP2040)||(ARDUINO_ARCH_ESP32)||(ARDUINO_ARCH_MBED_NANO)\
||(ARDUINO_ARCH_NRF52840)||(ARDUINO_ARCH_SAM)||(ARDUINO_ARCH_RENESAS)

#define DISABLE_PULSE_SENSOR_INTERRUPTS
#define ENABLE_PULSE_SENSOR_INTERRUPTS
#else
#define DISABLE_PULSE_SENSOR_INTERRUPTS cli()
#define ENABLE_PULSE_SENSOR_INTERRUPTS sei()
#endif

/*

*/
#endif // PULSE_SENSOR_PLAYGROUND_H
