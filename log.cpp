#include "SD.h"
#include "log.hpp"

void initializeLog(void) {
    pinMode(csPin, OUTPUT);
    // SD Card Initialization
    if (!SD.begin())
        Serial.println("SD card initialization failed");
}

void addLog(String message, String secondPartOfMessage="") {
    File logFile=SD.open("log.txt", FILE_WRITE);
    if (!logFile)
        return Serial.println("could not open log.txt");
    if (secondPartOfMessage=="") {
        logFile.println(message);
    } else {
        logFile.print(message);
        logFile.println(secondPartOfMessage);
    }
    logFile.close();
    
    Serial.print("Printed message: ");
    Serial.println(message);
}

void printLog(void) {
    File logFile=SD.open("log.txt", FILE_READ);
    if (!logFile)
        return Serial.println("could not open log.txt");
    
    Serial.println("-----Printing log.txt-----");

    while (logFile.available())
        Serial.write(logFile.read());

    Serial.println("---------------");

    logFile.close();
}
