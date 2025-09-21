//USB to Hardware Serial Bridge for nRF52840 based boards by StrawbEmi
#define VERSION "0.2.0" //LOL why does this basic ass program need a versioning system? Because I said so, that's why. :D

#define MANUFACTURER "Seeed Studio"
#define MODEL "Seeed Xiao nRF52840"

#include <Arduino.h>
#include <Adafruit_TinyUSB.h>
#include <bluefruit.h>


//Define desired HW serial port here
//Options depending on board
#define HWSERIAL Serial1


// BLE Service
BLEDfu  bledfu;  // OTA DFU service
BLEDis  bledis;  // device information
BLEUart bleuart; // uart over ble
BLEBas  blebas;  // battery

static void startAdv(void);
static void connect_callback(uint16_t conn_handle);
static void disconnect_callback(uint16_t conn_handle, uint8_t reason);

unsigned long btMillis = 0;

unsigned long hbMillis = 0;
unsigned long usbActivityMillis = 0;
unsigned long hwActivityMillis = 0;

void setup() {
  //set the pinmodes for the on-board LEDs
  pinMode(LED_RED, OUTPUT);
  pinMode(LED_GREEN, OUTPUT);
  pinMode(LED_BLUE, OUTPUT);
  //set the LEDs off except for red
  digitalWrite(LED_RED, LOW);
  digitalWrite(LED_GREEN, HIGH);
  digitalWrite(LED_BLUE, HIGH);

  //start the serial communication
  Serial.begin(38400);
  HWSERIAL.begin(38400);

  Serial.println("nRF: Waiting for USB serial to initialize...");
  unsigned long startMillis = millis();
  while(!Serial) {
    if(millis() - startMillis > 5000) { //if it's been over 5 seconds, break the loop and continue
      break;
    }
    delay(10);
  }
  //if USB serial initialized successfully, print success message
  if(Serial) { 
    Serial.println("nRF: If you can see this then USB serial initialized successfully! :)");
  }
  //while(!HWSERIAL) delay(10); //wait for HW serial to initialize

  //Print a start message with version number
  Serial.println("nRF: USB Serial to Hardware Serial Bridge - Version: ");
  Serial.println(VERSION);
  Serial.print("nRF: Starting");
  delay(500);

  // Setup the BLE LED to be enabled on CONNECT
  // Note: This is actually the default behaviour, but provided
  // here in case you want to control this LED manually via PIN 19
  Bluefruit.autoConnLed(true);

  Serial.print(".");
  // Config the peripheral connection with maximum bandwidth 
  // more SRAM required by SoftDevice
  // Note: All config***() function must be called before begin()
  Bluefruit.configPrphBandwidth(BANDWIDTH_MAX);

  Serial.print(".");

  Bluefruit.begin();
  Bluefruit.setTxPower(4);    // Check bluefruit.h for supported values
  Bluefruit.setName(MODEL);
  //Bluefruit.setName(getMcuUniqueID()); // useful testing with multiple central connections
  Bluefruit.Periph.setConnectCallback(connect_callback);
  Bluefruit.Periph.setDisconnectCallback(disconnect_callback);

  // To be consistent OTA DFU should be added first if it exists
  bledfu.begin();
  
  Serial.print(".");
  Serial.println();

  Serial.println("nRF: Starting Device Information Service...");
  // Configure and Start Device Information Service
  bledis.setManufacturer(MANUFACTURER);
  bledis.setModel(MODEL);
  bledis.begin();

  Serial.println("nRF: Starting BLE UART service...");
  // Configure and Start BLE Uart Service
  bleuart.begin();

  // Start BLE Battery Service
  blebas.begin();
  blebas.write(100);

  Serial.println("nRF: Starting BLE Advertising...");
  // Set up and start advertising
  startAdv();

  Serial.println("nRF: Please use Adafruit's Bluefruit LE app or nRF Connect to connect in UART mode");
  Serial.println("nRF: Once connected, enter character(s) that you wish to send");
  Serial.println("-----------------------------------------------------");
}


void startAdv(void)
{
  // Advertising packet
  Bluefruit.Advertising.addFlags(BLE_GAP_ADV_FLAGS_LE_ONLY_GENERAL_DISC_MODE);
  Bluefruit.Advertising.addTxPower();

  // Include bleuart 128-bit uuid
  Bluefruit.Advertising.addService(bleuart);

  // Secondary Scan Response packet (optional)
  // Since there is no room for 'Name' in Advertising packet
  Bluefruit.ScanResponse.addName();
  
  /* Start Advertising
   * - Enable auto advertising if disconnected
   * - Interval:  fast mode = 20 ms, slow mode = 152.5 ms
   * - Timeout for fast mode is 30 seconds
   * - Start(timeout) with timeout = 0 will advertise forever (until connected)
   * 
   * For recommended advertising interval
   * https://developer.apple.com/library/content/qa/qa1931/_index.html   
   */
  Bluefruit.Advertising.restartOnDisconnect(true);
  Bluefruit.Advertising.setInterval(32, 244);    // in unit of 0.625 ms
  Bluefruit.Advertising.setFastTimeout(30);      // number of seconds in fast mode
  Bluefruit.Advertising.start(0);                // 0 = Don't stop advertising after n seconds  
}


void heartbeat() {
  //blink red LED every second to show the program is running
  if (millis() - hbMillis > 1000) { //if it's been over 1 second, turn the LED on
    digitalWrite(LED_RED, LOW);
  }
  if (millis() - hbMillis > 2000) { //if it's been 2 seconds, reset the timer and turn the LED back off
    hbMillis = millis();
    digitalWrite(LED_RED, HIGH);
  }
}

void activityLeds() {
  //keep activity LEDs on for 1ms after last activity
  if (millis() - usbActivityMillis > 1) {
    digitalWrite(LED_GREEN, HIGH);
  }
  if (millis() - hwActivityMillis > 1) {
    digitalWrite(LED_BLUE, HIGH);
  }
}
void usbSerial() {  
  uint8_t incomingByte;

  //Read from USB serial and send to HW serial
  if (Serial.available() > 0) {
    digitalWrite(LED_GREEN, LOW); //blink green LED when USB serial data received
    usbActivityMillis = millis(); //reset USB LED activity timer
    incomingByte = Serial.read();
    HWSERIAL.write(incomingByte);
  }

  //Read from HW serial and send to USB serial
  if (HWSERIAL.available() > 0) {
    digitalWrite(LED_BLUE, LOW); //blink blue LED when HW serial data received  
    hwActivityMillis = millis(); //reset HW LED activity timer
    incomingByte = HWSERIAL.read();
    Serial.write(incomingByte);
  }


}

void bluetoothSerial() {

  // Forward from BLEUART to HW Serial
  if ( bleuart.available() > 0 ) {
    uint8_t ch;
    ch = (uint8_t) bleuart.read();
    HWSERIAL.write(ch);
  }

  // Forward data from HW Serial to BLEUART
  if ((HWSERIAL.available() > 0) && (Bluefruit.connected())) {
    // Delay to wait for enough input, since we have a limited transmission buffer
    if (millis() - btMillis > 2) { 
      btMillis = millis();
      uint8_t buf[64];
      int count = HWSERIAL.readBytes(buf, sizeof(buf));
      bleuart.write( buf, count );
    }

  }
}

void timerReset() {
  if (btMillis > 4294967000) { //reset the bluetooth timer if it's about to overflow
    btMillis = 0;
  }
  if (hbMillis > 4294967000) { //reset the heartbeat timer if it's about to overflow
    hbMillis = 0;
  }
  if(hwActivityMillis > 4294967000) { //reset the HW activity timer if it's about to overflow
    hwActivityMillis = 0;
  }
  if(usbActivityMillis > 4294967000) { //reset the USB activity timer if it's about to overflow
    usbActivityMillis = 0;
  }
}

void loop() {
  heartbeat();
  usbSerial();
  bluetoothSerial();
  activityLeds();
  timerReset();
}


// callback invoked when central connects
void connect_callback(uint16_t conn_handle)
{
  // Get the reference to current connection
  BLEConnection* connection = Bluefruit.Connection(conn_handle);

  char central_name[32] = { 0 };
  connection->getPeerName(central_name, sizeof(central_name));

  Serial.print("nRF: Connected to ");
  Serial.println(central_name);
}

/**
 * Callback invoked when a connection is dropped
 * @param conn_handle connection where this event happens
 * @param reason is a BLE_HCI_STATUS_CODE which can be found in ble_hci.h
 */
void disconnect_callback(uint16_t conn_handle, uint8_t reason)
{
  (void) conn_handle;
  (void) reason;

  Serial.println();
  Serial.println("nRF: Disconnected");
}
