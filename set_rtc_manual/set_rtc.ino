#include "RTClib.h"

RTC_DS1307 rtc;

char daysOfTheWeek[7][12] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};

void setup() {
  Serial.begin(57600);

#ifndef ESP8266
  while (!Serial); // wait for serial port to connect. Needed for native USB
#endif

  if (!rtc.begin()) {
    Serial.println("Couldn't find RTC");
    Serial.flush();
    while (1) delay(10);
  }

  if (!rtc.isrunning()) {
    Serial.println("RTC is NOT running, let's set the time!");
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  }

  Serial.println("Ready");
  Serial.println("Send 'S' to set new time, or any other key to continue");
  while (!Serial.available()) delay(10);
  
  if (Serial.read() == 'S') {
    setTimeViaSerial();
  }
}

void loop() {
  DateTime now = rtc.now();

  Serial.print(now.year(), DEC);
  Serial.print('/');
  Serial.print(now.month(), DEC);
  Serial.print('/');
  Serial.print(now.day(), DEC);
  Serial.print(" (");
  Serial.print(daysOfTheWeek[now.dayOfTheWeek()]);
  Serial.print(") ");
  Serial.print(now.hour(), DEC);
  Serial.print(':');
  Serial.print(now.minute(), DEC);
  Serial.print(':');
  Serial.print(now.second(), DEC);
  Serial.println();

  Serial.print("Unix time: ");
  Serial.print(now.unixtime());
  Serial.print("s = ");
  Serial.print(now.unixtime() / 86400L);
  Serial.println("d");

  Serial.println();
  delay(3000);
}

void setTimeViaSerial() {
  Serial.println("\nEnter new date and time (YYYY MM DD HH MM SS):");
  
  while (Serial.available() < 12) { // Wait for all numbers to be entered
    if (Serial.available()) {
      char c = Serial.peek();
      if (!(isdigit(c) || isspace(c))) {
        Serial.read(); // Discard non-digit, non-space characters
      }
    }
    delay(10);
  }

  // Read the numbers
  int year = Serial.parseInt();
  int month = Serial.parseInt();
  int day = Serial.parseInt();
  int hour = Serial.parseInt();
  int minute = Serial.parseInt();
  int second = Serial.parseInt();

  // Clear any remaining input
  while (Serial.available()) Serial.read();

  // Validate input
  if (year < 2000 || year > 2099 || month < 1 || month > 12 || 
      day < 1 || day > 31 || hour < 0 || hour > 23 || 
      minute < 0 || minute > 59 || second < 0 || second > 59) {
    Serial.println("Invalid date/time entered. Using current time.");
    return;
  }

  // Set the RTC
  DateTime newTime(year, month, day, hour, minute, second);
  rtc.adjust(newTime);
  
  Serial.println("RTC set to:");
  Serial.print(year); Serial.print('/');
  Serial.print(month); Serial.print('/');
  Serial.print(day); Serial.print(' ');
  Serial.print(hour); Serial.print(':');
  Serial.print(minute); Serial.print(':');
  Serial.println(second);
}