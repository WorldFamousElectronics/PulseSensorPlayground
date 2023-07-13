/*
   Code to detect pulses from the PulseSensor,
   using an interrupt service routine.

   This example is designed to target the Adafruit nRF52840 Feather Express.

    Install the dependent library. Go to Sketch > Include Library > Mange Libraries.
    When the Library Manager loads, search for NRF52_TimerInterrupt.
    Install the latest version.

    Download the nRF Toobox App from Nordic Semiconductor to your phone or tablet.
    Open the app and select Heart Rate Service, then connect to PulseSensor to view BPM graph.

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
    The NRF52TimerInterrupt library will set up a hardware timer to trigger
    sampling of the PulseSensor Playground library.
    Define the sample interval in microseconds.
*/
#include "NRF52TimerInterrupt.h"
#define SAMPLE_INTERVAL_US        2000 

/*
   Create an instance of the timer class called Sample_Timer.
   NRF_TIMER_0 and NRF_TIMER_1 can be used by the core
   and the bluetool library, so avoid them.
*/
NRF52Timer Sample_Timer(NRF_TIMER_3);

/*
    Include the Adafruit bluefruit library
    The target hardware will play the role of Server
    The phone or tablet will play the role of Client
*/
#include <bluefruit.h>

/*
    Heartrate Monitor Service 
    These are defined in the bluefruit library
    The service exposes characteristics that allow for different data
    to be sent by the Server as heartrate specific information. 
    BLEService is defined as heartrate monitor service
    The heartrate measurement characteristing (hrmc) is used to send the BPM
    The body sensor location (bslc) is used to send the PulseSensor location
    the byte variable bpm will be used to transmit PulseSensor heartrate to the client
*/
BLEService        hrms = BLEService(UUID16_SVC_HEART_RATE);
BLECharacteristic hrmc = BLECharacteristic(UUID16_CHR_HEART_RATE_MEASUREMENT);
BLECharacteristic bslc = BLECharacteristic(UUID16_CHR_BODY_SENSOR_LOCATION);
uint8_t  bpm = 0;

/*
    Device Information Service 
    This service allows us to add characteristics like 
    > Manufacturer
    > Model
    > Serial Number
    > Hardware Version
    > Frimware Version
    > etc.
*/
BLEDis bledis;   

/*
   Every Sketch that uses the PulseSensor Playground must
   define USE_ARDUINO_INTERRUPTS before including PulseSensorPlayground.h.
   Here, #define USE_ARDUINO_INTERRUPTS true tells the library to use
   interrupts to automatically read and process PulseSensor data.

   See PulseSensor_BPM_Alternative.ino for an example of not using interrupts.
*/
#define USE_ARDUINO_INTERRUPTS true
#include <PulseSensorPlayground.h>

void Timer3_ISR(){
  PulseSensorPlayground::OurThis->onSampleTime();
}
/*
   The format of our output.

   Set this to PROCESSING_VISUALIZER if you're going to run
    the Processing Visualizer Sketch.
    See https://github.com/WorldFamousElectronics/PulseSensor_Amped_Processing_Visualizer

   Set this to SERIAL_PLOTTER if you're going to run
    the Arduino IDE's Serial Plotter.
*/
const int OUTPUT_TYPE = SERIAL_PLOTTER;

/*
   Pinout:
     PULSE_INPUT = Analog Input. Connected to the pulse sensor
      purple (signal) wire.
     PULSE_BLINK = digital Output. Connected to an LED (and 1K series resistor)
      that will flash on each detected pulse.
     PULSE_FADE = digital Output. PWM pin onnected to an LED (and 1K series resistor)
      that will smoothly fade with each pulse.
      NOTE: PULSE_FADE must be a pin that supports PWM. Do not use
      pin 9 or 10, because those pins' PWM interferes with the sample timer.
     THRESHOLD should be set higher than the PulseSensor signal idles
      at when there is nothing touching it. The expected idle value
      should be 512, which is 1/2 of the ADC range. To check the idle value
      open a serial monitor and make note of the PulseSensor signal values
      with nothing touching the sensor. THRESHOLD should be a value higher
      than the range of idle noise by 25 to 50 or so. When the library
      is finding heartbeats, the value is adjusted based on the pulse signal
      waveform. THRESHOLD sets the default when there is no pulse present.
      Adjust as neccesary.
*/
const int PULSE_INPUT = A0;
const int PULSE_BLINK = 13;
const int PULSE_FADE = 12;
const int THRESHOLD = 550;   // Adjust this number to avoid noise when idle

/*
   All the PulseSensor Playground functions.
*/
PulseSensorPlayground pulseSensor;

void setup() {
  /*
     115200 provides about 11 bytes per millisecond.
     If we used a slower baud rate, we'd likely write bytes faster than
     they can be transmitted,.
  */
  Serial.begin(115200);
  while (!Serial && millis() < 5000);

  // Configure the PulseSensor manager.

  pulseSensor.analogInput(PULSE_INPUT);
  pulseSensor.blinkOnPulse(PULSE_BLINK);
  pulseSensor.fadeOnPulse(PULSE_FADE);
  pulseSensor.setThreshold(THRESHOLD);

  pulseSensor.setSerial(Serial);
  pulseSensor.setOutputType(OUTPUT_TYPE);

  // Now that everything is ready, start reading the PulseSensor signal.
  if (!pulseSensor.begin()) {
    /*
       PulseSensor initialization failed,
       likely because our particular Arduino platform interrupts
       aren't supported yet.

       If your Sketch hangs here, try PulseSensor_BPM_Alternative.ino,
       which doesn't use interrupts.
    */
    for(;;) {
      // Flash the led to show things didn't work.
      digitalWrite(PULSE_BLINK, LOW);
      delay(50);
      digitalWrite(PULSE_BLINK, HIGH);
      delay(50);
    }
  }

/*
    Begin the Bluetooth
    Define callback routines to run on connect and disconnect
    Set Device Information Characteristic attributes
    Set up the advertising packet
    Start advertising
*/
  Serial.println("Starting BLE");
  Bluefruit.begin();
  Bluefruit.setName("PulseSensor");
  Bluefruit.Periph.setConnectCallback(connect_callback);
  Bluefruit.Periph.setDisconnectCallback(disconnect_callback);
  bledis.begin();
  Serial.println("Begin Heartrate Monitor Service");
  setupHeartrateMonitor();
  startAdvertising();
  Serial.println("Advertising\nConnect via Bluetooth to PulseSensor to view BPM");

/*
    Start the interrupt timer at the end of setup
    so it does not interfere with other setup stuff
*/
  if (Sample_Timer.attachInterruptInterval(SAMPLE_INTERVAL_US, Timer3_ISR)){
    Serial.println(F("Starting Timer 3"));
  } else {
    Serial.println(F("Timer 3 Startup failed!"));
  }

} // end of setup()

void loop() {
  /*
     When a heartbeat happens, print the BPM to serial
     and notify connected client of the updated BPM value
   */
  if (pulseSensor.sawStartOfBeat()) {
    bpm = uint8_t(pulseSensor.getBeatsPerMinute());
    if ( Bluefruit.connected() ) {
        uint8_t heartRateData[2] = { 0b00000110, bpm };           // Sensor connected, BPM value
        if (hrmc.notify(heartRateData, sizeof(heartRateData))){
          Serial.print("Heart Rate Updated: "); 
        } else {
          Serial.println("error: Notify not set or not connected!");
        }
    }
    Serial.print(bpm,DEC); Serial.println(" BPM");
  }
}

/*
    startAdvertising()
    This function builds the packet that is sent
    to potential Clients. This lets Clients know what kind 
    of data they can get from us.
    Tell the bluetooth to restart advertising on disconnect from client
    setInterval and setFastTimeout tell the Bluetooth how often and how long
    to advertise. Setting start(timeout) = 0 will continue to advertise until connected
*/
void startAdvertising(void){
  Bluefruit.Advertising.addFlags(BLE_GAP_ADV_FLAGS_LE_ONLY_GENERAL_DISC_MODE);
  Bluefruit.Advertising.addTxPower();
  Bluefruit.Advertising.addService(hrms);
  Bluefruit.Advertising.addName();
  Bluefruit.Advertising.restartOnDisconnect(true);
  Bluefruit.Advertising.setInterval(32, 244);    // in unit of 0.625 ms
  Bluefruit.Advertising.setFastTimeout(30);      // number of seconds in fast mode
  Bluefruit.Advertising.start(0);                // 0 = Don't stop advertising after n seconds  
}

/*
    setupHeartrateMonitor()
    This function initializes the heartrate monitor service.
    hrms.begin() must be called before any characteristics can be set.
    hrmc.setProperties() defines the way that we will send the data
    hrmc.setPermission() defines the security access. First parameter is read, second is write.
    hrmc.setFixedLen() defines how many bytes the characteristic has
        first byte is metadata
        second byte is heartrate
    Now, the characteristic can begin, and once started we can write to it
    hrmc.write() tells the characteristic to expect 8 bit values and that the PulseSensor is connected
    The Sensor Location characteristic setup is similar.
    After is it begun, the bslc.write() sends a byte variable to set the physical location of the PulseSensor
        0 = OTHER
        1 = CHEST
        2 = WRIST
        3 = FINGER
        4 = HAND
        5 = EARLOBE
        6 = FOOT
        7:255 = RESERVED
*/
void setupHeartrateMonitor(void){
  hrms.begin();
  hrmc.setProperties(CHR_PROPS_NOTIFY);
  hrmc.setPermission(SECMODE_OPEN, SECMODE_NO_ACCESS);
  hrmc.setFixedLen(2);  
  hrmc.begin();
  uint8_t heartRateCharacteristicData[2] = { 0b00000110, 0x40 }; // Set the characteristic to use 8-bit values, with the sensor connected and detected
  hrmc.notify(heartRateCharacteristicData, 2);
  bslc.setProperties(CHR_PROPS_READ);
  bslc.setPermission(SECMODE_OPEN, SECMODE_NO_ACCESS);
  bslc.setFixedLen(1);
  bslc.begin();
  bslc.write8(3);    // Set the characteristic to 'Finger' (3)
}

/*
    connect_callback function is run when a Client connects
    It gets a reference to the current connection,
    then prints feedback to the serial port
*/
void connect_callback(uint16_t conn_handle){
  BLEConnection* connection = Bluefruit.Connection(conn_handle);
  char central_name[32] = { 0 };
  connection->getPeerName(central_name, sizeof(central_name));
  Serial.print("Connected to ");
  Serial.println(central_name);
}

/*
    disconnect_callback is run when a Client disconnects
    conn_handle is the connection that got disconnected
    reason is a status code describing the reason for the disconnect
    for example the code 0x13 corresponds to User Terminated Connection.
*/
void disconnect_callback(uint16_t conn_handle, uint8_t reason){
  (void) conn_handle;
  (void) reason;
  Serial.print("Disconnected, reason = 0x"); Serial.println(reason, HEX);
  Serial.println("Advertising!");
}

