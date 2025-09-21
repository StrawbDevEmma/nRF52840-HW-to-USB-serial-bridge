//USB to Hardware Serial Bridge for nRF52840 based boards by StrawbEmi
#define VERSION "0.1.4" //LOL why does this basic ass program need a versioning system? Because I said so, that's why. :D

#include <Arduino.h>
#include <Adafruit_TinyUSB.h>

//Define desired HW serial port here
//Options depending on board
#define HWSERIAL Serial1

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
  //set the LEDs to show we're alive
  digitalWrite(LED_RED, LOW);
  digitalWrite(LED_GREEN, HIGH);
  digitalWrite(LED_BLUE, HIGH);
  int incomingByte;
  if (Serial.available() > 0) {
    digitalWrite(LED_GREEN, LOW); //blink green LED when USB serial data received
    incomingByte = Serial.read();
    HWSERIAL.write(incomingByte);
  }
  if (HWSERIAL.available() > 0) {
    digitalWrite(LED_BLUE, LOW); //blink blue LED when HW serial data received  
    incomingByte = HWSERIAL.read();
    Serial.write(incomingByte);
  }
}
