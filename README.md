
![logo](https://avatars0.githubusercontent.com/u/7002937?v=3&s=200) 
# [PulseSensor.com](https://pulsesensor.com) Playground
```
This Playground is a collection of code for the most popular uses of PulseSensor and Arduino.  

- The playgroud includes a number of projects, with the code already written-out and commented! 🤘 
- Use this code to get started quickly, or do advanced stuff. 👍
- Once you download the playground, you can simply access each project's code inside the Arduino IDE (software). 💻
- Contritube your project code back to the GitHub hive-mind. 🐝
```
___
## Playground Project Descriptions: 


### Start Here:  
  Plug your sensor in for the first time!  Blink an LED with your pulse, live. 
- Project Page: https://pulsesensor.com/pages/code-and-guide
  
  <img src="https://cdn.shopify.com/s/files/1/0100/6632/files/PulseSensor_GettingStarted_bb_1024x1024.png?v=1511986616" width="400">
---

### Calculate BPM:  
  Focus-in on the code that calculates a user's HeartRate Beats Per Minute, "BPM".    
  Plug your sensor in for the first time!  See the best practises to get the best signal.  
- Project Page: https://pulsesensor.com/pages/getting-advanced
  
  <img src="https://cdn.shopify.com/s/files/1/0100/6632/files/PulseSensor_GettingAdvanced_bb_1024x1024.png?v=1511986194" width="400">
---
  
### Make A Sound to a live Heartbeat:  
  Transform the heartbeat into a live "beep" with a speaker and piezo.   
- Project Page: https://pulsesensor.com/pages/pulse-sensor-speaker-tutorial
  
  <img src="https://cdn.shopify.com/s/files/1/0100/6632/files/PulseSensor_Speaker_bb_61a0333f-e868-4123-961d-7456a31fa928_1024x1024.png?v=1510863829" width="400">
---  
  
  ### Move a Motor to a live Heartbeat:  
  Get detailed visulization of the heart's pulse and behavior. Take the PulseSensor date into Processing.  
- Project Page:  https://pulsesensor.com/pages/pulse-sensor-servo-tutorial

  <img src="https://cdn.shopify.com/s/files/1/0100/6632/files/PulseSensor_Servo_bb_87fce9fc-dc47-4208-b708-a7edb6df58a2_1024x1024.png?v=1510863990" width="400">
 ---

 ### Processing Visualizer:  
  Get detailed visulization of the heart's pulse and behavior. Take the PulseSensor date into Processing.  
- Project Page: https://pulsesensor.com/pages/getting-advanced
  
  <img src="https://cdn.shopify.com/s/files/1/0100/6632/files/ScreenShot_1024x1024.png?v=1491857113" width="400">
---  

### TroubleShoot Your Signal:  
  Plug your sensor in for the first time!  Blink an LED with your pulse, live. 
- Project Page: https://pulsesensor.com/pages/code-and-guide
  
  <img src="https://github.com/WorldFamousElectronics/PulseSensorStarterProject/blob/master/screenshot-threshold-arrows.png" width="400">


## Installing the Software 🤓 
 
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


## Connecting the Harware 😎
  <img src="https://github.com/WorldFamousElectronics/PulseSensorStarterProject/raw/master/Arduino-LEDonPin13-PulseSensor-Pic.jpg" width="400">

 ## Physical Connections 👩‍💻👨‍💻 
 
 1. A computer that can run the: [Arduino IDE](https://www.arduino.cc/en/Main/Software) and 
 2. An Arduino Board
 3. A PulseSensor from PulseSensor.com 
 

## Need Help?
The Issues Tab will get you the quickest response. You might even see answers to questions there.


## Thanks

This list was inspired by [awesome-ios](https://github.com/vsouza/awesome-ios) and [awesome-swift](https://github.com/matteocrippa/awesome-swift). Thanks to all the [contributors](https://github.com/dkhamsing/open-source-ios-apps/graphs/contributors) 🎉 


------------------------------------------------------
#####  Legal:  PulseSensor.com™ World Famous Electronics llc. in Brooklyn, NY. USA
------------------------------------------------------
Made Something Awesome With the PulseSensor Code?   Send Us Some PayPal Love. ♥︎  
[![Donate](https://img.shields.io/badge/Donate-PayPal-green.svg?style=plastic)](https://www.paypal.com/cgi-bin/webscr?cmd=_s-xclick&hosted_button_id=KE4DZA5E9AJQ4) 


