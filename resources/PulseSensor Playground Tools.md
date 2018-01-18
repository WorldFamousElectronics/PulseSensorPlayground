# PulseSensor Playground Tools

We created the PulseSensor Playground library to make it easy to use and customize. This page will show you how to get the most out of the PulseSensor Playground. Let's go!
	
## Useful Functions

The PulseSensor Playground library provides all the tools you need to get data into your sketch and make magic with. The descriptions below are brief for brevity. There's much more detail commented in the Playground code files.

### PulseSensorPlayground()
You have to create an instance that can access all the PulseSensor Playground functions

	PulseSensorPlayground pulseSensor;

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
The Playground doesn't output serial data automatically. If you want to output serial pulse data, include this. If not, don't.

### setOutputType(int)
By default, Playground will output Serial data in `SERIAL_PLOTTER` format. You can change this to `PROCESSING_VISUALIZER` if you like. 

### setThreshold(int)
Set a value that the Pulse Sensor signal has to cross when going up. Adjusting this can be useful to combat noise.

### getLatestSample()
Returns the most recently read analog value from the Pulse Sensor.

### getBeatsPerMinute()
Returns the latest beats-per-minute. 

### getInterBeatIntervalMs()
Returns the latest inter-beat interval (IBI) in milliseconds.

### sawStartOfBeat()
Returns true if a new heartbeat pulse has been detected.

### isInsideBeat()
Returns true for the time when a measured heartbeat wave is above the value set in setThreshold() above. 

### outputSample()
Output the latest sample over the Serial port. If your Sketch wants to plot samples, it should call this function every so often.

### outputBeat()
Output the latest BPM and IBI values over the Serial port.

## Using Pulse Sensor Interrupts

We want to use interrupts if we can in our Pulse Sensor code in order to get super accurate hear rate data: Beats Per Minute (BPM), and Inter-beat Interval (IBI). In the example sketch [PulseSensor_BPM.ino](https://github.com/biomurph/PulseSensorPlayground/tree/master/examples/PulseSensor_BPM) there are a couple of lines of code at the very top of the sketch that set this up:

	#define USE_ARDUINO_INTERRUPTS true
	#include <PulseSensorPlayground.h>
	
While using interrupts is super cool and useful, it won't work with all of the myriad Arduino boards out there. If your board does not support interrupts yet, not to worry! We have code that works just as well without employing interrupts. It's called [PulseSensor_BPM_Alternative.ino](https://github.com/biomurph/PulseSensorPlayground/tree/master/examples/PulseSensor_BPM_Alternative).

## Selecting Your Serial Output

The sketch examples in PulseSensor Playground will output different kinds of serial info for visualizing your pulse waveform and Beats Per Minute data. Here's how you can choose which serial to use.

* Our basic example sketch called [A_StarterProject_and_SignalTester.ino](https://github.com/biomurph/PulseSensorPlayground/tree/master/examples/A_StarterProject_and_SignalTester) will output a serial stream of Pulse Sensor signal data only. The signal is read in the loop function about every 10mS. This works perfectly with the Arduino Serial Plotter. Follow this [Tutorial](https://pulsesensor.com/pages/code-and-guide) to give it a go.

* All the other example sketches that we have give you the option to output to the Arduino Serial Plotter, or to our [Pulse Sensor Visualizer](https://github.com/WorldFamousElectronics/PulseSensor_Amped_Processing_Visualizer) program. To select which one you want to output to, you will need to tell Arduino by setting the value of a variable called `OUTPUT_TYPE` at the top of the sketch. Here's an example that shows how to set the output to work with our Visualizer software.
  
![outputType](https://github.com/biomurph/PulseSensorPlayground/blob/master/Images/outputType.png)
