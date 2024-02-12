#include <SPI.h>
#include <SD.h>

//serial monitor print while writing log
boolean serialPrint = true;//true or false
char *logFile ="datalog.txt";//file name. must be max 8 character excluding the extension (book.txt is 4 char)
int readPushButtonPin = 9;

File myFile;
void writeLog(float);//prototype of writLog funciton at the bottom of this code
void readLog(void);//prototype of readLog funciton at the bottom of this code

void setup() {

    //Watch the instruction to this code on youTube https://youtu.be/TduSOX6CMr4
    // Open serial communications and wait for port to open:
    Serial.begin(9600);

    while (!Serial) {
        ; // wait for serial port to connect. Needed for native USB port only
    }

    pinMode(readPushButtonPin, INPUT_PULLUP);

    Serial.print("Initializing SD card...");

    if (!SD.begin(4)) {
        Serial.println("initialization failed!");
        while (1);
    }

    Serial.println("initialization done.");
    SD.remove(logFile);

    // open the file. note that only one file can be open at a time,
    // so you have to close this one before opening another.

    myFile = SD.open(logFile, FILE_WRITE);

    if (!myFile) {
        Serial.println("log file missing");
        while(1);
    }
}

void loop() {
    int sensor = analogRead(A0);//read A0 pin

    //convert value of 0 to 1023 to voltage. 
    //Arduino UNO is 5V so we use 5.0
    float voltage = sensor * (5.0 / 1023.0); //5.0 it depend in your input value

    if(digitalRead(readPushButtonPin) == LOW)
    {
        delay(100);//
        readLog();
    }

    writeLog(voltage);   
        //Watch the instruction to this code on youTube https://youtu.be/TduSOX6CMr4
    }

void writeLog(float voltage)
{
    myFile = SD.open(logFile, FILE_WRITE);
    myFile.print("Time: ");//writing the text: Time:
    myFile.print(millis()/1000); //recod time  
    myFile.print("\tVoltage: ");//writing the text Volage after a tab 	
    myFile.print(voltage);//Writing voltage
    myFile.println("V");//adding V at the end
    
    if(serialPrint) {
        Serial.print ("Time:");
        Serial.print(millis()/1000);  
        Serial.print ("	Voltage: ");
        Serial.print(voltage);       
    } 

    // close the file:
    myFile.close();
    
    if(serialPrint){
        Serial.println(" done.");
    }
}

/*
* readLog
* Reads the datas from MicroSD Card and prints it on serial monitor
*/
void readLog()
{

    // re-open the file for reading:
    myFile = SD.open(logFile);

    if (myFile) {
        Serial.print("Reading ========");
        Serial.println(logFile);
        
        // read from the file until there's nothing else in it:
        while (myFile.available()) {
            Serial.write(myFile.read());
        }

        Serial.print("Reading END ==========");

        // close the file:
        myFile.close();
        
    } else {
        // if the file didn't open, print an error:
        Serial.print("readLog() error opening ");
        Serial.println(logFile);
    }  
}