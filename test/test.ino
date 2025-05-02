#include <Wire.h>

const int ledPin = LED_BUILTIN;  // Use the built-in LED
int counter = 0;

void setup() {
  // Initialize serial communication
  Serial.begin(9600);
  while (!Serial);  // Wait for serial port to connect (needed for some boards)
  
  // Initialize I2C
  Wire.begin();
  
  // Initialize LED pin
  pinMode(ledPin, OUTPUT);
  
  Serial.println("\nI2C Scanner starting...");
  
  // Scan I2C devices
  scanI2C();
}

void loop() {
  // Blink the LED
  digitalWrite(ledPin, HIGH);
  delay(500);
  digitalWrite(ledPin, LOW);
  delay(500);
  
  // Print incrementing number to serial
  Serial.print("Counter: ");
  Serial.println(counter++);
}

void scanI2C() {
  byte error, address;
  int nDevices = 0;
  
  Serial.println("Scanning I2C devices...");
  
  for(address = 1; address < 127; address++) {
    Wire.beginTransmission(address);
    error = Wire.endTransmission();
    
    if (error == 0) {
      Serial.print("I2C device found at address 0x");
      if (address < 16) {
        Serial.print("0");
      }
      Serial.print(address, HEX);
      Serial.println();
      nDevices++;
    }
    else if (error == 4) {
      Serial.print("Unknown error at address 0x");
      if (address < 16) {
        Serial.print("0");
      }
      Serial.println(address, HEX);
    }
  }
  
  if (nDevices == 0) {
    Serial.println("No I2C devices found");
  }
  else {
    Serial.println("Scan completed");
  }
}