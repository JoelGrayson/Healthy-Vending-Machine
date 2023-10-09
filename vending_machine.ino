// # Global Constants
#define N_SNACKS 8


// # Liquid Crystal Display
#include <LiquidCrystal_I2C.h> //from github.com/johnrickman/LiquidCrystal_I2C

LiquidCrystal_I2C lcd(0x27, 20, 4); //20x4 display from amazon.com/gp/product/B01DKETWO2
    // Pin 9–13


// # Keypad
#include <Keypad.h> //Mark Stanley's Keypad library

const uint8_t N_ROWS=4;
const uint8_t N_COLS=3;

char keys[N_ROWS][N_COLS]={
    { '1', '2', '3' },
    { '4', '5', '6' },
    { '7', '8', '9' },
    { '*', '0', '#' }
};

uint8_t rowPins[N_ROWS]={ 9, 8, 7, 6 };
uint8_t colPins[N_COLS]={ 5, 4, 3 };

Keypad keypad(makeKeymap(keys), rowPins, colPins, N_ROWS, N_COLS);


// # Dollar Acceptor
// Each pulse is configured to be one dollar. From https://github.com/mudmin/AnotherMaker/blob/master/arduino-powered-cash-reader/cash-reader-with-lcd.c
#define dollarPin 15 //blue pin. Purple pin is connected to ground
// It is HIGH when normal. When $ inserted, it beeps LOW for ~100 millis per dollar
uint8_t pulse=HIGH;
uint8_t prevPulse=HIGH;
double moneyInserted=0;


// # DC Motor
const uint8_t motorPins[N_SNACKS]={ 23, 25, 27, 29, 31, 33, 35, 37 }; //23–37 even numbers
void turnOnce(uint8_t motorI) { //turns the transistor controlling the spring motor just long enough for one revolution
    const int transistorPin=motorPins[motorI];
    Serial.print("Turning "); Serial.println(transistorPin);
    digitalWrite(transistorPin, HIGH);
    delay(3300);
    digitalWrite(transistorPin, LOW);
}


// # Log
#include "log.hpp"


void setup() {
    Serial.begin(9600);
    while (!Serial);
    Serial.println("Serial began");
    
    lcd.init();
    lcd.backlight();
    lcd.clear();
    lcd.setCursor(0, 0);


    // Bill Acceptor
    pinMode(dollarPin, INPUT_PULLUP);

    // DC Motor
    for (uint8_t motorPin : motorPins)
        pinMode(motorPin, OUTPUT);

    // Log
    initializeLog();
    addLog("turned on");
}

void loop() {
    // Keypad
    char customKey=keypad.getKey(); //get value if keypad pressed
    
    if (customKey) { //key pressed
        uint8_t motorI=(uint8_t) (int(customKey)-int('1')); //offset from 1

        // Show message
        lcd.clear();
        lcd.setCursor(0, 0);
        if (motorI>N_SNACKS-1) {
            addLog("out of range");
            return lcd.print("Out of range"); //validate customKey in range
        }

        char message[50]; //buffer
        sprintf(message, "Selected %c.", customKey);
        lcd.print(message);
        Serial.println(message);
        addLog(message);

        // Turn
        turnOnce(motorI);
    }

    // Bill Acceptor
    prevPulse=pulse;
    pulse=digitalRead(dollarPin);
    if (pulse==HIGH && prevPulse==LOW) {
        moneyInserted++;
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("You have $");
        lcd.print(moneyInserted);
        addLog("inserted $", String(moneyInserted));
    }
}

void log() {

}
