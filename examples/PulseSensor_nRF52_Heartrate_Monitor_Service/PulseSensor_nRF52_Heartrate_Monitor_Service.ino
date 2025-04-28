/*
   This code is written to target the following hardrware:
      Adafruit nRF52 boards
      Seeed Studio nRF52 boards (NO EMBED)
   Embed board architecture will be supported in the future.

    Check out the PulseSensor Playground Tools for explaination
    of all user functions and directives.
    https://github.com/WorldFamousElectronics/PulseSensorPlayground/blob/master/resources/PulseSensor%20Playground%20Tools.md

    This example code is fully described in our new book:
    Heartbeat Sensor Projects with PulseSensor: Prototyping Biofeedback Devices with PulseSensor!
    https://www.amazon.com/Heartbeat-Sensor-Projects-PulseSensor-Prototyping/dp/148429324X
    There is also a tutorial online here:
    https://pulsesensor.com/pages/nrf52-bluetooth-low-energy

    For more information on the PulseSensor methods and functions
    go to our Resources page
    https://github.com/WorldFamousElectronics/PulseSensorPlayground/blob/master/resources/PulseSensor%20Playground%20Tools.md
*/

/*
    Include the Adafruit bluefruit library
    The nRF52 will play the role of Server
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
   Include the PulseSensor Playground library to get all the good stuff!
   The PulseSensor Playground library will decide whether to use
   a hardware timer to get accurate sample readings by checking
   what target hardware is being used and adjust accordingly.
   You may see a "warning" come up in red during compilation
   if a hardware timer is not being used.
*/
#include <PulseSensorPlayground.h>

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

