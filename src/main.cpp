//USB to Hardware Serial Bridge for nRF52840 based boards by StrawbEmi
#define VERSION "0.1.6" //LOL why does this basic ass program need a versioning system? Because I said so, that's why. :D

#include <Arduino.h>
#include <Adafruit_TinyUSB.h>

//Define desired HW serial port here
//Options depending on board
#define HWSERIAL Serial1

unsigned long hbMilis = 0;
unsigned long usbActivityMilis = 0;
unsigned long hwActivityMilis = 0;

void setup() {
  //set the pinmodes for the on-board LEDs
  pinMode(LED_RED, OUTPUT);
  pinMode(LED_GREEN, OUTPUT);
  pinMode(LED_BLUE, OUTPUT);
  //set the LEDs off
  digitalWrite(LED_RED, HIGH);
  digitalWrite(LED_GREEN, HIGH);
  digitalWrite(LED_BLUE, HIGH);
  //start the serial communication
  Serial.begin(38400);
  HWSERIAL.begin(38400);
  delay(2000);
  digitalWrite(LED_RED, LOW);
  Serial.println("Foobar"); //random print to tell serial terminal that device is connected
  delay(2000); // wait for serial monitor to open
  Serial.print("Starting");
  delay(500);
  Serial.print(".");
  delay(500);
  Serial.print(".");
  delay(500);
  Serial.println(".");
  delay(1000);
  //Print a start message
  Serial.println("USB Serial to Hardware Serial Bridge - Version: ");
  Serial.println(VERSION);
  Serial.println("If you can see this then USB serial initialized successfully! :)");
}

void loop() {
  //blink red LED every second to show the program is running
  if (millis() - hbMilis > 1000) { //if it's been over 1 second, turn the LED on
    digitalWrite(LED_RED, LOW);
  }
  if (millis() - hbMilis > 2000) { //if it's been 2 seconds, reset the timer and turn the LED back off
    hbMilis = millis();
    digitalWrite(LED_RED, HIGH);
  }
  
  //keep activity LEDs on for 1ms after last activity
  if (millis() - usbActivityMilis > 1) {
    digitalWrite(LED_GREEN, HIGH);
  }
  if (millis() - hwActivityMilis > 1) {
    digitalWrite(LED_BLUE, HIGH);
  }

  int incomingByte;

  //Read from USB serial and send to HW serial
  if (Serial.available() > 0) {
    digitalWrite(LED_GREEN, LOW); //blink green LED when USB serial data received
    usbActivityMilis = millis(); //reset USB LED activity timer
    incomingByte = Serial.read();
    HWSERIAL.write(incomingByte);
  }

  //Read from HW serial and send to USB serial
  if (HWSERIAL.available() > 0) {
    digitalWrite(LED_BLUE, LOW); //blink blue LED when HW serial data received  
    hwActivityMilis = millis(); //reset HW LED activity timer
    incomingByte = HWSERIAL.read();
    Serial.write(incomingByte);
  }



  if (hbMilis > 4294967000) { //reset the heartbeat timer if it's about to overflow
    hbMilis = 0;
  }
  if(hwActivityMilis > 4294967000) { //reset the HW activity timer if it's about to overflow
    hwActivityMilis = 0;
  }
  if(usbActivityMilis > 4294967000) { //reset the USB activity timer if it's about to overflow
    usbActivityMilis = 0;
  }
}
