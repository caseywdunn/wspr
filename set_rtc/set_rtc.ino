#include <Wire.h>
#include <RTClib.h> // Make sure to install "RTClib by Adafruit"

RTC_DS1307 rtc;
// RTC_DS3231 rtc; 
// RTC_PCF8523 rtc;

const long SERIAL_BAUD = 9600; // Must match Python script

void setup() {
  Serial.begin(SERIAL_BAUD);
  while (!Serial); 

  Serial.println("RTC Set Time Sketch (DS1307)");
  Serial.println("Waiting for time sync message (e.g., T<YYYY>,<MM>,<DD>,<hh>,<mm>,<ss>)");

  if (!rtc.begin()) {
    Serial.println("Couldn't find RTC!");
    Serial.flush();
    while (1) delay(10); // Halt if RTC not found
  }

  // Check if the clock is running (DS1307 equivalent of checking power loss)
  if (!rtc.isrunning()) {
    Serial.println("RTC was not running (e.g., powered down without battery). Please set time.");
    // Note: Time will be set by the Python script when it runs.
    // If you wanted it to start counting from a default time immediately, you could add:
    // rtc.adjust(DateTime(F(__DATE__), F(__TIME__))); 
    // But it's better to wait for the accurate time from Python.
  }

  // Print current RTC time on startup
  printTime(); 
}

void loop() {
  // Check if data is available from Serial
  if (Serial.available() > 0) {
    String input = Serial.readStringUntil('\n');
    input.trim(); // Remove any leading/trailing whitespace

    Serial.print("Received: ");
    Serial.println(input);

    // Check if the input starts with the time sync prefix 'T'
    if (input.startsWith("T")) {
      // Remove the 'T' prefix
      String timeData = input.substring(1); 

      // Parse the comma-separated values
      int year, month, day, hour, minute, second;
      
      // Use sscanf for robust parsing (requires converting String to char array)
      char buffer[30]; 
      timeData.toCharArray(buffer, sizeof(buffer));

      int parsedFields = sscanf(buffer, "%d,%d,%d,%d,%d,%d", 
                                &year, &month, &day, &hour, &minute, &second);

      if (parsedFields == 6) {
        // Check basic validity (optional, add more checks if needed)
        if (year >= 2000 && year < 2100 && month >= 1 && month <= 12 && day >= 1 && day <= 31 &&
            hour >= 0 && hour <= 23 && minute >= 0 && minute <= 59 && second >= 0 && second <= 59) 
        {
            // Create a DateTime object and set the RTC
            DateTime newTime(year, month, day, hour, minute, second);
            rtc.adjust(newTime);
            
            Serial.println("RTC time set successfully!");
            printTime(); // Print the newly set time
            Serial.println("OK"); // Send confirmation back to Python
        } else {
            Serial.println("Error: Parsed date/time values out of range.");
            Serial.println("FAIL: Invalid data");
        }
      } else {
        Serial.print("Error: Could not parse all 6 time fields. Found: ");
        Serial.println(parsedFields);
        Serial.println("FAIL: Parse error");
      }
    } else {
      Serial.println("Ignoring input: Missing 'T' prefix.");
    }
  }

  // Optional: Print RTC time periodically for debugging
  // delay(5000);
  // printTime(); 
}

// Helper function to print the current RTC time
void printTime() {
  DateTime now = rtc.now();
  Serial.print("Current RTC Time: ");
  Serial.print(now.year(), DEC);
  Serial.print('/');
  Serial.print(now.month(), DEC);
  Serial.print('/');
  Serial.print(now.day(), DEC);
  Serial.print(" ");
  Serial.print(now.hour(), DEC);
  Serial.print(':');
  Serial.print(now.minute(), DEC);
  Serial.print(':');
  Serial.print(now.second(), DEC);
  Serial.println();
}