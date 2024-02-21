#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <SSD1306Ascii.h>
#include <SSD1306AsciiWire.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define I2C_ADDRESS 0x3C

SSD1306AsciiWire oled;

const int ACPin = A0;
const float VREF = 5.0;
const float ACTectionRange = 30.0;
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
  oled.begin(&Adafruit128x64, I2C_ADDRESS);

  tiempo_init = millis();

  pinMode(ACPin, INPUT);

  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }

  Serial.print("Initializing SD card...");

  if (!SD.begin(4)) {
    Serial.println("initialization failed!");
    while (1);
  }
  Serial.println("initialization done.");

  // open  the file. note that only one file can be open at a time,
  // so you have to close this one before opening another.
  myFile = SD.open("datalog.txt", FILE_WRITE);

  // if the file opened okay, write to it:
  if (myFile) {
    Serial.print("Writing to datalog.txt...");
    myFile.println("testing 1, 2, 3.");
    // close the file:
    myFile.close();
    Serial.println("done.");
  } else {
    // if the file didn't open, print an error:
    Serial.println("error opening datalog.txt");
  }

  // re-open the file for reading:
  myFile = SD.open("datalog.txt");
  if (myFile) {
    Serial.println("test.txt:");

    // read from the file until there's nothing else in it:
    while (myFile.available()) {
      Serial.write(myFile.read());
    }
    // close the file:
    myFile.close();
  } else {
    // if the file didn't open, print an error:
    Serial.println("error opening test.txt");
  }

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
  Serial.print(adc_max);
  Serial.println(" V");

  oled.clear();
  oled.setFont(System5x7);
  oled.println("ELECTRIC SUPPLY MONITORING");
  oled.println("_____________________________");
  oled.println("CURRENT: " + String(current_value) + " A");
  oled.println("VOLTAGE:  " + String(adc_max) + " V");

  // Write data to SD card
  myFile = SD.open(logFile, FILE_WRITE);
  if (myFile) {
    myFile.println("CURRENT: " + String(current_value) + " A");
    myFile.println("VOLTAGE:  " + String(adc_max) + " V");
    myFile.close();
  } else {
    Serial.println("Error opening log file for writing");
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
