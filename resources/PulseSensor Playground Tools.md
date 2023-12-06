# PulseSensor Playground Tools

We created the PulseSensor Playground library to make it easy to use and customize. This page will show you how to get the most out of the PulseSensor Playground. Let's go!

## Useful Functions

The PulseSensor Playground library provides all the tools you need to get data into your sketch and make magic with. The descriptions below are brief for brevity. There's much more detail commented in the Playground code files.

---
### PulseSensorPlayground()
You have to create an instance that can access all the PulseSensor Playground functions

	PulseSensorPlayground pulseSensor;

---
### begin()
Start reading and processing data from the PulseSensor! Returns `true` when successfull and `false` if there is a problem. In our examples, if this function returns false, the program will hang, blink the LED and send '!' over the serial port.

---
### pause()
Turn off timer and stop reading and processing PulseSensor data.
Useful if you need to do other time sensitive things. Returns `true` when successful.

---
### resume()
Turn on timer and start reading and processing PUlseSensor data.
Use this after calling pause() to resume sampling PulseSensor. Returns `true` when successful.

---
### isPaused()
Retruns `true` while PulseSensor algorithgm is paused, `false` while it is running.

---
### sawNewSample()
Will return `true` if a new sample has been read. This function is used to ensure software sample time
when not using a hardware timer. If a hardware timer is not being used, this function needs to be called often enough to ensure 500Hz sample rate (every 2mS).

---
### analogInput(int)
Set the pin your PulseSensor is connected to.

---
### blinkOnPulse(int)
Set the pin that will blink to your pulse.

---
### fadeOnPulse(int)
Set the pin to fade with you're heartbeat. Make sure the pin can do PWM!

---
### setSerial(Serial)
The Playground doesn't output serial data automatically. If you want the library to output serial pulse data, include this. If not, don't, and see the explication of the pre-processor directive below.

---
### setOutputType(int)
By default, Playground will output Serial data in `SERIAL_PLOTTER` format. Change this to `PROCESSING_VISUALIZER` if you are going to use one of our Processing Visualizer Sketch examples.

---
### setThreshold(int)
Set a value that the PulseSensor signal has to cross when going up. Adjusting this can be useful to combat noise. We set the default value at 550

---
### getLatestSample()
Returns the most recently read analog value from the PulseSensor. Type = int.

---
### getBeatsPerMinute()
Returns the latest beats-per-minute. Type = int.

---
### getInterBeatIntervalMs()
Returns the latest inter-beat interval (IBI) in milliseconds. Type = int.

---
### getPulseAmplitude()
Returns the amplitude of the latest pulse wave. Value is computed from analog value of latest pulse wave peak minus latest pulse wave trough. Type = int.

---
### getLastBeatTime()
Returns the sample number when the latest beat was found. The sample number has a 2mS resolution. Type = unsigned long.

---
### sawStartOfBeat()
Returns `true` if a new heartbeat pulse has been detected. Type = bool.

---
### isInsideBeat()
Returns `true` for the time when a measured heartbeat wave is above the value set by the setThreshold() function, and `false` when it's not.

---
### outputSample()
Output the latest sample over the Serial port. The samples will either be formatted for the Arduino Serial Plotter, or one for our Processing Visualizer Sketches depending on the parameter set in setOutputType() above. In the case of `SERIAL_PLOTTER`, the library will print BPM, IBI, and PulseSensor raw signal. In the case of `PROCESSING_VISUALIZER`, the library will print just the raw PulseSensor value formatted for our Processing Visualizer Sketches.

---
### outputBeat()
Output the latest BPM and IBI values over the Serial port. This function is only run when outputType is set to `PROCESSING_VISUALIZER`, and the output is formatted for our Processing Visualizer Sketches.

---
### outputToSerial(char, int)
Output Data with a character prefix. Used exclusively with the PulseSensor Processing Visualizer. Processing Visualizer needs to know what the prefix means in order to parse data from the serial stream. The characters we use are`S` for raw PulseSensor data, `B` for beats per minute, and `Q` for interbeat interval.

---
## Notes On Sample Timing

Version 2.0.0 and up of the PulseSensor Playground library uses preprocessor directives to determine support for using a hardware timer or a software timer. We wan to use hardware timer if we can, but software timers work OK when you need to. If a software timer is used, the compiler will print a warning to the output terminal in Arduino IDE. In this case, the function `sawNewSample()` needs to be called often to ensure accurate sample timing. For a full list of supported and unsupported Arduino and Arduino compatible boards, please review this link

---
## Selecting Your Serial Output

The sketch examples in PulseSensor Playground will output different kinds of serial info for visualizing your pulse waveform and Beats Per Minute data. Here's how you can choose which serial to use.

* Our basic example sketch called [GettingStartedProject.ino](https://github.com/WorldFamousElectronics/PulseSensorPlayground/tree/master/examples/GettingStartedProject) will output a serial stream of PulseSensor signal data only. The signal is read in the loop function about every 10mS. This works perfectly with the Arduino Serial Plotter. Follow this [Tutorial](https://pulsesensor.com/pages/code-and-guide) to give it a go.

* All the other example sketches that we have give you the option to output to the Arduino Serial Plotter, or to our [PulseSensor Visualizer](https://github.com/WorldFamousElectronics/PulseSensor_Amped_Processing_Visualizer) program. To select which one you want to output to, you will need to tell Arduino by setting the value of a variable called `OUTPUT_TYPE` at the top of the sketch. Here's an example that shows how to set the output to work with our Visualizer software.

---
## Preprocessor Directive Use

The PulseSensor Playground library has a lot of functionality. It can handle serial messages, for example. But sometimes you might not want the library to manage the Serial port. If you don't want us to manage the Serial port, then you can tell the library that by changing one line in the PulseSensorPlayground.h file. When the library uses the Serial port, this line is commented out:

`#define USE_SERIAL true`

If you don't want us to use the Serial port, simply comment out the line above and uncomment the line:

`#define USE_SERIAL false`

You also will need to discard the `setSerial()` and `setOutputType()` functions. You can access the raw PulseSensor, BPM and IBI data, etc, by using calls to functions that return those values above, and print them on your own as you like.

Other preprocessor directives that we have for users to adjust are

`PULSE_SENSOR_TIMING_ANALYSIS`
and 
`PULSE_SENSOR_MEMORY_USAGE`

Please read all about them in the PulseSensorPlayground.h file!
