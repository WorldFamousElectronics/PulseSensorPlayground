# PulseSensor Playground Tools

We Created the PulseSensor Playground Library to make it easier to use and customize. This page will show you how to get the most out of the PulseSensor Playground. Let's go!

## Pulse Sensor Interrupts

We use interrups in our Pulse Sensor code so that we can get super accurate hear rate data: Beats Per Minute (BPM), and Interbeat Interval (IBI). In order to do that with our Playground library, we have to tell that to Arduino. In the example Sketch

	PulseSensor_BPM

there are a couple of lines of code at the very top of the sketch that do this:

	#define USE_ARDUINO_INTERRUPTS true
	#include <PulseSensorPlayground.h>
	
While using interrupts is super cool and useful, it won't work with all of the myriad Arduino boards out there. If your board does not support interrupts yet, not to worry! We have code that works just as well without employing interrupts. It's called 

	PulseSensor_BPM_Alternative
	
and you can find it in our examples folder inside our library. 

## Selecting Your Serial Output

The sketch examples in PulseSensor Playground will output different kinds of serial info for visualizing your pulse waveform and Beats Per Minute data. Here's how you can choose which serial to use.

* Our basic example sketch called `A_StarterProject_and_SignalTester.ino` will output a serial stream of Pusle Sensor signal data only. The signal is read in the loop function about every 10mS. This works perfectly with the Arduino Serial Plotter. Follow this [Tutorial](https://pulsesensor.com/pages/code-and-guide) to give it a go.

* All the other example sketches that we have give you the option to output to the Arduino Serial Plotter, or to our [Pulse Sensor Visualizer](https://github.com/WorldFamousElectronics/PulseSensor_Amped_Processing_Visualizer) program. To select which one you want to output to, you will need to tell Arduino by setting the value of a variable called `outputType` at the top of the sketch. Here's an example of how to set the output to work with our Visualizer software.

![outputType](images/outputType.png)
	
## Useful Functions

The PulseSensor Playground library exposes some very useful functions for you to use for finding the heartbeat, and getting data into your sketch to make magic with. The descriptions below are brief for brevity. There's much more detail commented in the Playground code files.

### PulseSensorPlayground()
Creates an instance that can access all the PulseSensor Playground functions

	PulseSensorPlayground pulseSensor;
	
Alternately can set up to read multiple Pulse Sensors

	const int PULSE_SENSOR_COUNT = 2;
	PulseSensorPlayground pulseSensor(PULSE_SENSOR_COUNT);


### begin()
Start reading and processing data from the Pulse Sensor!

### sawNewSample()
Will return `true` if a new sample has been read.

### analogInput(int)
Set the pin your Pulse Sensor is connected to.

### blinkOnPulse(int)
Set the pin that will blink to your pulse.

### fadeOnPulse(int)
Set the pin to fade with you're heartbeat. Make sure the pin can do PWM!

### setSerial(Serial)
The Playround doesn't output serial data automatically. If you want to output serial pulse data, call this. If not, don't.

### setOutputType(int)
By default, Playground output is in `SERIAL_PLOTTER` format. You can change this to `PROCESSING_VISUALIZER` if you like. 

### setThreshold(int)
Set a value that the Pulse Sensor signal has to cross. This can be useful to combat noise.

### getLatestSample()
Returns the most recently read analog value from the Pulse Sensor (range: 0..1023).

### getBeatsPerMinute()
Returns the latest beats-per-minute. 

### getInterBeatIntervalMs()
Returns the latest interbeat interval (IBI) in milliseconds.

### sawStartOfBeat()
Returns true if a new heartbeat (pulse) has been detected.

### isInsideBeat()
Returns true for the time when a measured heartbeat wave above the `THRESHOLD` value. 

### outputSample()
Output the latest sample. If your Sketch wants to plot samples, it should call this function every so often.

###outputBeat()
Output over Serial the latest BPM and IBI values
