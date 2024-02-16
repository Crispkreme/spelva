#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128 
#define SCREEN_HEIGHT 64 
 
#define OLED_RESET   -1 
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

boolean serialPrint = true; 
char *logFile = "datalog.txt"; 
 
float Current_Value = 0;
const int ACPin = A0;         
int adc_max = 0;    
int adc_min = 1023; 
long tiempo_init; 

File myFile;

void setup() {
  Serial.begin(9600);
  delay(2000); // Added delay for serial monitor to initialize

  // Initialize OLED display
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("SSD1306 allocation failed"));
    for (;;);
  }

  tiempo_init = millis(); 

  display.clearDisplay();
  display.setTextColor(WHITE);
  display.display(); // Make sure to display content after clearing

  Serial.print("Initializing SD card...");

  if (!SD.begin(4)) {
    Serial.println("initialization failed!");
    while (1);
  }

  Serial.println("initialization done.");

  SD.remove(logFile);

  myFile = SD.open(logFile, FILE_WRITE);

  if (!myFile) {
    Serial.println("log file missing");
    while(1);
  }
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

  if (sensorValue > adc_max) {
    adc_max = sensorValue; 
  } else if (sensorValue < adc_min) {
    adc_min = sensorValue; 
  }

  Serial.print(current_value);
  Serial.println(" A");

  current += current_value;
  
  writeLog(adc_max, current); 

  delay(1000);

  Serial.print(current);
  Serial.println("Current: ");
  display.clearDisplay();

  display.setTextSize(1);
  display.setCursor(0,0);
  display.print("Current: ");
  display.print(current);

  display.setTextSize(1);
  display.setCursor(0, 8);
  display.print("Voltage: ");
  display.print(adc_max);

  display.display(); 
}

float readACCurrentValue() {
  float ACCurrtntValue = 0;
  float peakVoltage = 0;
  float voltageVirtualValue = 0; 

  for (int i = 0; i < 5; i++) {
    peakVoltage += analogRead(ACPin);  
    delay(1);
  }

  peakVoltage = peakVoltage / 5;
  voltageVirtualValue = peakVoltage * 0.707; 

  voltageVirtualValue = (voltageVirtualValue / 1024 * VREF ) / 2;
  ACCurrtntValue = voltageVirtualValue * ACTectionRange;

  return ACCurrtntValue;
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
    Serial.print ("Time:");
    Serial.print(millis()/1000);  
    Serial.print (" Voltage: ");
    Serial.print(voltage);       
    Serial.print (" Current: ");
    Serial.print(current);  
  } 

  myFile.close();
  
  if (serialPrint) {
    Serial.println(" done.");
  }
}

void readLog() {
  myFile = SD.open(logFile);

  if (myFile) {
    Serial.print("Reading ========");
    Serial.println(logFile);
    
    while (myFile.available()) {
      Serial.write(myFile.read());
    }

    Serial.print("Reading END ==========");

    myFile.close();
      
  } else {

    Serial.print("readLog() error opening ");
    Serial.println(logFile);
  }  
}
