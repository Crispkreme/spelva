#include <Wire.h>
#include <SD.h>
#include <SSD1306Ascii.h>
#include <SSD1306AsciiWire.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define I2C_ADDRESS 0x3C 

SSD1306AsciiWire oled;

const int ACPin = A0;
const float VREF = 5.0; // Reference voltage
const float ACTectionRange = 30.0; // Range of current sensor
float adc_max = 0;
float adc_min = 1023; 
float current_Value = 0;   
long tiempo_init; 

boolean serialPrint = true; 
char *logFile = "datalog.txt"; 

File myFile;

void setup() {
    Serial.begin(9600);
    Wire.begin();
    oled.begin(&Adafruit128x64, I2C_ADDRESS); // Initialize OLED display

    tiempo_init = millis(); 

    if (!SD.begin()) { // Initialize SD card
        Serial.println("SD initialization failed!");
        return;
    }
    Serial.println("SD initialization done.");
    SD.remove(logFile);

    myFile = SD.open(logFile, FILE_WRITE); 
    if (!myFile) {
        Serial.println("Error opening log file!");
        while (1);
    }
    myFile.close();
    pinMode(ACPin, INPUT);
    oled.clear();
}

void loop() {

    int sensorValue = analogRead(A1); 
    float current = 0;
    float current_value = readACCurrentValue();

    if ((millis() - tiempo_init) > 500) { 
        adc_max = 0;     
        adc_min = 1023;  
        tiempo_init = millis(); 
    }

    // Update max ADC value
    if (current_value > adc_max) {
        adc_max = current_value; 
    }

    Serial.print(current_value);
    Serial.println(" A");

    // Display current value on OLED
    oled.clear();
    oled.setFont(System5x7);
    oled.println("Current:");
    oled.print(current_value);
    oled.println("A");

    oled.setFont(System5x7);
    oled.println("Voltage:");
    oled.print(adc_max);
    oled.println("V");

    // Write data to SD card
    myFile = SD.open(logFile, FILE_WRITE);
    if (myFile) {
        myFile.print("Current: ");
        myFile.print(current_value);
        myFile.println("A");

        myFile.print("Voltage: ");
        myFile.print(adc_max);
        myFile.println("V");
        myFile.close();
    } else {
        Serial.println("Error opening log file for writing");
    }

    // Display data from SD card
    myFile = SD.open(logFile);
    if (myFile) {
        Serial.println("Log file contents:");
        while (myFile.available()) {
            Serial.write(myFile.read());
        }
        myFile.close();
    } else {
        Serial.println("Error opening log file for reading");
    }

    delay(5000); // Delay between readings
}

float readACCurrentValue() {
    float peakVoltage = 0;
    float voltageVirtualValue = 0; 

    for (int i = 0; i < 5; i++) {
        peakVoltage += analogRead(ACPin);  
        delay(1);
    }

    peakVoltage = peakVoltage / 5;
    voltageVirtualValue = peakVoltage * 0.707; 

    voltageVirtualValue = (voltageVirtualValue / 1024 * VREF ) / 2;
    float current_value = voltageVirtualValue * ACTectionRange;

    return current_value;
}
