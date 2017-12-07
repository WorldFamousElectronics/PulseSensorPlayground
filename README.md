
![logo](https://avatars0.githubusercontent.com/u/7002937?v=3&s=200) 
# [PulseSensor.com](https://pulsesensor.com) Playground

This Playground is a collection of code for the most popular uses of PulseSensor and Arduino.  

- Once you download the playground, you can simply access each project's code inside the Arduino IDE (software). 💻
- The playgroud includes a number of projects, with the code already written out and commented! 🤘 
- Use this code to get started quickly. 👍
- Contritube your project code back into the GitHub hive-mind. 🐝


## Playground Projects   
 

 <img height='300' alt='Artsy Shows image 1' src='https://raw.githubusercontent.com/artsy/emergence/master/docs/screenshots/featured.png'> <img height='300' alt='Artsy Shows image 2' src='https://raw.githubusercontent.com/artsy/emergence/master/docs/screenshots/show.png'> 


- EX Player: Watch videos from EX.UA file sharing  
<img height='300' alt='EX Player image 1' src='https://raw.githubusercontent.com/IGRSoft/exTVPlayer/master/Screenshots/4.png'> 


### "Start Here" Project:  
  Plug your sensor in for the first time!  See the best practises to get the best signal.  
  [![Alt text](video-play.png)](https://www.youtube.com/watch?v=82T_zBZQkOE)
![ScreenShot](screenshot-threshold-arrows.png)
  Project Page: https://pulsesensor.com/pages/code-and-guide
  
 <img height='300' alt='GrandCentralBoard image 1' src='http://cdn.macoscope.com/blog/wp-content/uploads/2016/03/zegar_anim_1.gif'> 

### Calculate BPM:  
  Need to get the user's heartrate BPM (Beats Per Minute)?  Use this project.   
  ![ScreenShot](screenshot-threshold-arrows.png)
  Project Page: https://pulsesensor.com/pages/code-and-guide
  
 <img height='300' alt='GrandCentralBoard image 1' src='http://cdn.macoscope.com/blog/wp-content/uploads/2016/03/zegar_anim_1.gif'> 
 
 ### Processing Visualizer:  
  Plug your sensor in for the first time!  See the best practises to get the best signal.  
  [![Alt text](video-play.png)](https://www.youtube.com/watch?v=82T_zBZQkOE)
![ScreenShot](screenshot-threshold-arrows.png)
  Project Page: https://pulsesensor.com/pages/code-and-guide
  
 <img height='300' alt='GrandCentralBoard image 1' src='http://cdn.macoscope.com/blog/wp-content/uploads/2016/03/zegar_anim_1.gif'> 


## How to Download and Install 🤨 
 
 1. Install [Arduino IDE](https://www.arduino.cc/en/Main/Software) and [Processing](https://processing.org/)
2. Download the .ZIP file of this library.
2. Start the Arduino IDE
3. In the IDE, Select Sketch / Include Library / Add .ZIP Library...
4. In the resulting dialog, navigate to the downloaded .ZIP of this library.
5. Follow the instructions to install this library.
6. Once this library is installed...
3. In the IDE, Select one of the examples under File / Examples / PulseSensor Playground.
4. Wire your circuit: Connect the Pulse Sensor signal to the Arduino's A0; connect an LED and 220 ohm resister in series between pin 5 and ground (GND).
5. Follow the rest of the instructions in [the original Processing example](https://github.com/WorldFamousElectronics/PulseSensor_Amped_Processing_Visualizer)
  * **Note**: In the Processing code, the line "println(Serial.list());" isn't compatible with Processing 3.0.  You'll need to change that line to "printArray(Serial.list());"
  * **Note2**: The [Arduino 101 specs page](https://www.arduino.cc/en/Main/ArduinoBoard101) says the analog input is "...from ground to 3.3 volts", so experiment with powering the PulseSensor from 3.3V vs. 5V and see which works better for you.

 
 ## Physical Connections 👩‍💻👨‍💻 
 
 1. A computer that can run the: [Arduino IDE](https://www.arduino.cc/en/Main/Software) and 
 2. An Arduino Board
 3. A PulseSensor from PulseSensor.com 
 

## Need Help?
The Issues Tab will get you the quickest response. You might even see answers to questions there.


## Thanks

This list was inspired by [awesome-ios](https://github.com/vsouza/awesome-ios) and [awesome-swift](https://github.com/matteocrippa/awesome-swift). Thanks to all the [contributors](https://github.com/dkhamsing/open-source-ios-apps/graphs/contributors) 🎉 

