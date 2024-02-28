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


#ifndef PULSE_SENSOR_PLAYGROUND_H
#define PULSE_SENSOR_PLAYGROUND_H

#include <Arduino.h>
/*
    SelectTimer.h will determine if the library supports
    hardware timer interrupts or not. If so, it will set
    up a hardware timer to sample at 500Hz. If not,
    it will set up a software timer that the main program
    will have to check often called 

*/
#include "utility/SelectTimer.h"


/*
    Library version number
*/
#define PULSESENSOR_PLAYGROUND_VERSION_STRING "v2.1.0"
#define PULSESENSOR_PLAYGROUND_VERSION_NUMBER 210

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
       Start reading and processing data from the PulseSensor(s).
       Your Sketch should make all necessary PulseSensor configuration calls
       before calling begin().
    */
    bool begin();

    /*

vvvvvvvv  THIS NEEDS MODIFICATION FOR V2 vvvvvvvv
       Returns true if a new sample has been read from each PulseSensor.
       You'll likely want to add this call to your Sketch's loop()
       only if you either 1) want to do something with each sample of the
       PulseSensor signals, or 2) your Sketch doesn't use a hardware timer
       to read from the PulseSensors.

       NOTE: If your Sketch uses a software timer,
       you must call sawNewSample() frequently (at least
       once every 2 milliseconds) to assure that PulseSensor signals
       are read accurately.
       A typical loop() that uses a software timer should not have 
       any delay() statements in it.  
    */
    bool sawNewSample();

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
    bool sawStartOfBeat(int sensorIndex = 0);

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
    bool isInsideBeat(int sensorIndex = 0);

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
#else
    #warning "PulseSensor Playground internal Serial commands not used"
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
	bool isPaused();

    /*
	   Pause PulseSensor sampling in order to do other stuff.
       This will effect all PulseSensors if you are using more than one.
	   
    */
	bool pause();

	/*
        Resume sampling the PulseSensor after a call to pause().
        This will effect all PulseSensors if you are using more than one.
    */
	bool resume();


    // (internal to the library) "this" pointer for the ISR.
#if USE_HARDWARE_TIMER
    static PulseSensorPlayground *OurThis;
#endif

    byte samplesUntilReport = SAMPLES_PER_SERIAL_SAMPLE;
    bool UsingHardwareTimer;

  private:

/*
   Optionally use this (or a different) pin to toggle high
   while the beat finding algorithm is running.
   Uncomment this line and the other 3 timingPin lines
   in PulseSensorPlayground.cpp, then connect the pin
   to an oscilloscope to measure algorithm run time.
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

   Sets up the sample timer interrupt for this Arduino Platform
   and the pause and resume routines.
*/
bool setupInterrupt();
bool disableInterrupt();
bool enableInterrupt();

/*
   Varialbles
*/
	bool Paused;                // keeps track of whether the algorithm is running
    byte SensorCount;              // number of PulseSensors in Sensors[].
    PulseSensor *Sensors;          // use Sensors[idx] to access a sensor.
    volatile unsigned long NextSampleMicros; // Desired time to sample next.
    volatile bool SawNewSample; // "A sample has arrived from the ISR"
#if USE_SERIAL
    PulseSensorSerialOutput SerialOutput; // Serial Output manager.
#endif // USE_SERIAL
#if PULSE_SENSOR_TIMING_ANALYSIS   // Don't use ram and flash we don't need.
    PulseSensorTimingStatistics *pTiming;
#endif // PULSE_SENSOR_TIMING_ANALYSIS

};


#endif // PULSE_SENSOR_PLAYGROUND_H
