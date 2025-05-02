/*
WSPR transmitter with RTC timing control
Transmits at :01 seconds past every minute ending with 8
*/

#include <si5351.h>
#include <JTEncode.h>
#include <Wire.h>
#include "RTClib.h"

Si5351 si5351;
JTEncode jtencode;
RTC_DS1307 rtc;

// Global variables
#define WSPR_FREQ       14096900UL  // 20M, 14.094,60
#define LED_PIN         13
char call[] = "AC1GQ";
char loc[] = "FN31";
uint8_t dbm = 27;
uint8_t tx_buffer[255];

void encode() {
    uint8_t i;
    DateTime now = rtc.now();

    Serial.print("Starting transmission at ");
    Serial.print(now.hour());
    Serial.print(":");
    if(now.minute() < 10) Serial.print("0");
    Serial.print(now.minute());
    Serial.print(":");
    if(now.second() < 10) Serial.print("0");
    Serial.println(now.second());
    
    // Reset the tone to the base frequency and turn on the output
    si5351.output_enable(SI5351_CLK0, 1);
    digitalWrite(LED_PIN, HIGH);

    for(i = 0; i < WSPR_SYMBOL_COUNT; i++) {
        si5351.set_freq((WSPR_FREQ * 100) + (tx_buffer[i] * 146), SI5351_CLK0);
        delay(683);
    }

    // Turn off the output
    si5351.output_enable(SI5351_CLK0, 0);
    digitalWrite(LED_PIN, LOW);
    Serial.println("Transmission complete");
}

void set_tx_buffer() {
    memset(tx_buffer, 0, 255);
    jtencode.wspr_encode(call, loc, dbm, tx_buffer);
}

void setup() {
    Serial.begin(57600);
    while(!Serial); // Wait for serial port to connect

    // Initialize RTC
    if (!rtc.begin()) {
        Serial.println("Couldn't find RTC");
        while(1);
    }

    if (!rtc.isrunning()) {
        Serial.println("RTC is NOT running, setting to compile time");
        rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
    }

    // Initialize Si5351
    if(!si5351.init(SI5351_CRYSTAL_LOAD_8PF, 0, 0)) {
        Serial.println("Si5351 initialization failed!");
        while(1);
    }

    // Configure pins and outputs
    pinMode(LED_PIN, OUTPUT);
    digitalWrite(LED_PIN, LOW);
    si5351.drive_strength(SI5351_CLK0, SI5351_DRIVE_8MA);
    si5351.output_enable(SI5351_CLK0, 0);

    // Encode the message
    set_tx_buffer();

    Serial.println("\nWSPR Transmitter Ready");
    Serial.print("Callsign: "); Serial.println(call);
    Serial.print("Location: "); Serial.println(loc);
    Serial.print("Power: "); Serial.print(dbm); Serial.println(" dBm");
    Serial.print("Frequency: "); Serial.print(WSPR_FREQ/1000000.0, 2); Serial.println(" MHz");
    Serial.println("Waiting for transmission time (minutes ending with 8, at 01 seconds)...");
}

void loop() {
    DateTime now = rtc.now();
    
    // Check if we're at :01 of any minute ending with 8
    if(now.minute() % 10 == 8 && now.second() == 1) {
        encode();
        
        // Wait until we're past the transmission time to avoid repeats
        while(rtc.now().second() == 1) {
            delay(100);
        }
    }

    // Print status every minute
    static uint8_t last_minute = 255;
    if(now.minute() != last_minute) {
        last_minute = now.minute();
        Serial.print("Time: ");
        Serial.print(now.hour());
        Serial.print(":");
        if(now.minute() < 10) Serial.print("0");
        Serial.print(now.minute());
        Serial.print(":");
        if(now.second() < 10) Serial.print("0");
        Serial.print(now.second());
        Serial.print(" - Next transmission at ");
        Serial.print(now.hour());
        Serial.print(":");
        // Calculate next minute ending with 8
        int next_min = ((now.minute()/10)*10) + 8;
        if(next_min <= now.minute()) next_min += 10;
        if(next_min >= 60) next_min -= 60;
        if(next_min < 10) Serial.print("0");
        Serial.print(next_min);
        Serial.println(":01");
    }

    delay(500); // Short delay to reduce CPU usage
}