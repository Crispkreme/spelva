#include <DHT.h>
#include <Wire.h>
#include <SSD1306Ascii.h>
#include <SSD1306AsciiWire.h>
#include <SPI.h>
#include <SD.h>

#define DHTPIN 2
#define DHTTYPE DHT22

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

#define I2C_ADDRESS 0x3C  // Change this to your display's I2C address
SSD1306AsciiWire oled;   // Initialize with I2C constructor

DHT dht(DHTPIN, DHTTYPE);

float hum;
float temp;
const int chipSelect = 10;

File myFile;

void setup()
{
    Serial.begin(9600);

    Wire.begin();
    oled.begin(&Adafruit128x64, I2C_ADDRESS); // Initialize display with the specified width and height

    if (!SD.begin(chipSelect)) {
        Serial.println("initialization failed!");
        return;
    }
    Serial.println("initialization done.");

    delay(2000);
    oled.clear();

    dht.begin();
}

void loop()
{
    hum = dht.readHumidity();
    temp = dht.readTemperature();

    // Display Humidity
    oled.setFont(System5x7); // Set font
    oled.println("Humidity:");
    oled.print(hum);
    oled.println("%");

    // Display Temperature
    oled.println("Temp:");
    oled.print(temp);
    oled.println("C");

    // Write data to SD card
    myFile = SD.open("data.txt", FILE_WRITE);

    if (myFile) {
        myFile.print("Humidity: ");
        myFile.print(hum);
        myFile.print("%, Temp: ");
        myFile.print(temp);
        myFile.println("C");
        myFile.close();
    } else {
        Serial.println("error opening data.txt");
    }

    // Display data from SD card
    myFile = SD.open("data.txt");
    if (myFile) {
        Serial.println("data.txt:");

        while (myFile.available()) {
            Serial.write(myFile.read());
        }
        myFile.close();
    } else {
        Serial.println("error opening data.txt");
    }

    delay(5000); // Delay between readings
}
