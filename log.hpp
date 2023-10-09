#ifndef LOG_HPP
#define LOG_HPP

#include <Arduino.h>
#include <SD.h>
#include <SPI.h>
#define csPin 10 // Pin 53 on Arduino Mega

void initializeLog(void);
void addLog(String message, String secondPartOfMessage="");

#endif