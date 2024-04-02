#include <SPI.h> 
#include <RTClib.h>
#include <Wire.h>
#include <ZMPT101B.h>
#include <SD.h>
#include "HD44780_LCD_PCF8574.h"

#define DISPLAY_DELAY_INIT 50
#define SENSITIVITY 500.0f
#define ACTectionRange 20
#define VREF 5

HD44780LCD myLCD(4, 16, 0x27, &Wire);
ZMPT101B voltageSensor(A0, 50.0);
RTC_DS1307 rtc;

float correctionfactor = -10;
float Current_Value = 0;
float peakVoltage = 0;
float voltageVirtualValue = 0;
float Wh = 0;
const int ACPin = A1;

char watt[5];
char buf1[20];

unsigned long last_time = 0;
unsigned long current_time = 0;

File myFile;

void setup() {
  Serial.begin(9600);

  rtc.begin();

  voltageSensor.setSensitivity(SENSITIVITY);

  while (!Serial);

  pinMode(ACPin, INPUT);

  delay(DISPLAY_DELAY_INIT);
  myLCD.PCF8574_LCDInit(myLCD.LCDCursorTypeOn);
  myLCD.PCF8574_LCDClearScreen();
  myLCD.PCF8574_LCDBackLightSet(true);
  myLCD.PCF8574_LCDGOTO(myLCD.LCDLineNumberOne, 0);
}

void loop() {

  float ACcurrent = readACCurrentValue();
  float acvoltage = voltageSensor.getRmsVoltage();
  float P = acvoltage * ACcurrent;

  DateTime now = rtc.now();

  sprintf(buf1, "%02d:%02d:%02d %02d/%02d/%02d",  now.hour(), now.minute(), now.second(), now.day(), now.month(), now.year());  

  last_time = current_time;
  current_time = millis();
  Wh = Wh + P * ((current_time - last_time) / 3600000.0);
  dtostrf(Wh, 4, 2, watt);

  String titleString = "SPELVA";
  String datetimeString = "Date/Time: " + String(buf1);
  String voltageString = "Voltage: " + String(acvoltage) + " V";
  String currentString = "Current: " + String(ACcurrent) + " A";

  myLCD.PCF8574_LCDGOTO(myLCD.LCDLineNumberOne, 0);
  myLCD.PCF8574_LCDSendString(titleString.c_str());
  myLCD.PCF8574_LCDSendChar(' ');

  myLCD.PCF8574_LCDGOTO(myLCD.LCDLineNumberTwo, 0);
  myLCD.PCF8574_LCDSendString(datetimeString.c_str());
  myLCD.PCF8574_LCDSendChar(' ');

  myLCD.PCF8574_LCDGOTO(myLCD.LCDLineNumberThree, 4);
  myLCD.PCF8574_LCDSendString(voltageString.c_str());
  myLCD.PCF8574_LCDSendChar(' ');

  myLCD.PCF8574_LCDGOTO(myLCD.LCDLineNumberFour, 4);
  myLCD.PCF8574_LCDSendString(currentString.c_str());
  myLCD.PCF8574_LCDSendChar(' ');

  Serial.println("Voltage: " + String(acvoltage));
  Serial.println("Current: " + String(ACcurrent));

  writeToFile(acvoltage, ACcurrent, datetimeString);
  delay(1000);
}

float readACCurrentValue() {
  float ACCurrtntValue = 0;
  float peakVoltage = 0;
  float voltageVirtualValue = 0; //Vrms
  for (int i = 0; i < 5; i++) {
    peakVoltage += analogRead(ACPin); //read peak voltage
    peakVoltage = peakVoltage + correctionfactor;
    delay(1);
  }
  peakVoltage = peakVoltage / 5;
  voltageVirtualValue = peakVoltage * 0.707; //change the peak voltage to the Virtual Value of voltage

  /*The circuit is amplified by 2 times, so it is divided by 2.*/
  voltageVirtualValue = (voltageVirtualValue / 1024 * VREF) / 2;
  ACCurrtntValue = voltageVirtualValue * ACTectionRange;

  Serial.println("Vm: " + String(peakVoltage));
  Serial.println("Vrms: " + String(voltageVirtualValue));

  return ACCurrtntValue;
}
  
void writeToFile(float acvoltage, float ACcurrent, String datetimeString) {
  Serial.print("Initializing SD card...");

  if (!SD.begin(4)) {
    Serial.println("Initialization failed!");
    while (true);
  }

  myFile = SD.open("test.txt", FILE_WRITE);

  if (myFile) {
    myFile.println("=========== SPELVA DATA ============");
    myFile.println(String(datetimeString));
    myFile.println("Voltage: " + String(acvoltage) + " V");
    myFile.println("Current: " + String(ACcurrent) + " C");
    myFile.println(" ");
    myFile.close();
    Serial.println("Data written to file.");
  } else {
    Serial.println("Error opening file");
  }
}