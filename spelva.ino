#include <Wire.h>
#include <SSD1306Ascii.h>
#include <SSD1306AsciiWire.h>
#include <SPI.h>
#include <SD.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define I2C_ADDRESS 0x3C

SSD1306AsciiWire oled;

const int ACPin = A0;
const int chipSelect = 10;

boolean serialPrint = true;
char *logFile = "datalog.txt";

float Current_Value = 0;
int adc_max = 0;
long tiempo_init;

File myFile;

void setup() {
    Serial.begin(9600);
    Wire.begin();
    oled.begin(&Adafruit128x64, I2C_ADDRESS);

    if (!SD.begin(chipSelect)) {
        Serial.println("initialization failed!");
        return;
    }
    Serial.println("initialization done.");

    SD.remove(logFile);

    myFile = SD.open(logFile, FILE_WRITE);

    if (!myFile) {
        Serial.println("log file missing");
        while(1);
    }

    tiempo_init = millis();

    delay(2000);
    oled.clear();
}

void loop() {
    int sensorValue = analogRead(ACPin);
    float current_value = readACCurrentValue();

    if ((millis() - tiempo_init) > 500) {
        adc_max = 0;
        tiempo_init = millis();
    }

    if (sensorValue > adc_max) {
        adc_max = sensorValue;
    }

    writeLog(adc_max, current_value);

    delay(1000);

    oled.clear();
    oled.println("Current:");
    oled.println(current_value, 3);
    oled.println("A");
    oled.println("Voltage:");
    oled.println(adc_max);
    oled.display();
}

float readACCurrentValue() {
    float ACCurrentValue = 0;
    float peakVoltage = 0;
    float voltageVirtualValue = 0;

    for (int i = 0; i < 5; i++) {
        peakVoltage += analogRead(ACPin);
        delay(1);
    }

    peakVoltage = peakVoltage / 5;
    voltageVirtualValue = peakVoltage * 0.707;

    voltageVirtualValue = (voltageVirtualValue / 1024 * VREF ) / 2;
    ACCurrentValue = voltageVirtualValue * ACTectionRange;

    return ACCurrentValue;
}

void writeLog(float voltage, float current) {
    myFile = SD.open(logFile, FILE_WRITE);
    if (!myFile) {
        Serial.println("Error opening log file!");
        return;
    }

    myFile.print("Time: ");
    myFile.print(millis()/1000);
    myFile.print("\tVoltage: ");
    myFile.print(voltage);
    myFile.println("V");
    myFile.print("\tCurrent: ");
    myFile.print(current);
    myFile.println("amps");

    if (serialPrint) {
        Serial.print("Time:");
        Serial.print(millis()/1000);
        Serial.print(" Voltage: ");
        Serial.print(voltage);
        Serial.print(" Current: ");
        Serial.println(current);
    }

    myFile.close();
}
