// # Global Constants
#define N_SNACKS 8
String snackNames[]={
    "Mushroom Jerky",
    "Honey Sticks",
    "Harvest Snaps",
    "Spicy Chickpeas",
    "Blue Doritos",
    "Waffle",
    "Twix Bar",
    "Ruffles"
};
double snackPrices[]={
    5.00,
    1.00,
    5.00,
    1.00,
    1.00,
    2.00,
    1.00,
    2.00
};

const unsigned long idleDurationBeforeReset=600000; //10 minutes


// # Logic
double moneyInserted=0;
int selectedSnackIndex=-1;
unsigned long buttonLastPressed=0;



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


// # DC Motor
const uint8_t motorPins[N_SNACKS]={ 23, 25, 27, 29, 31, 33, 35, 37 }; //23–37 odd numbers
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

    Serial.println(idleDurationBeforeReset);
}

void loop() {
    // Keypad
    char customKey=keypad.getKey(); //get value if keypad pressed
    
    if (customKey) { //key pressed
        buttonLastPressed=millis();

        if (customKey=='0') { //press 0 to reset
            reset();
            displayBalance();
            return;
        }
        if (customKey=='#') //print log to Serial port if computer is connected
            return printLog();
    
        int tempSelectedSnackIndex=int(customKey)-int('1'); //offset from 1
        // customKey - one-based indexing
        // selectedSnackIndex - zero-based indexing

        // Show message
        lcd.clear();
        lcd.setCursor(0, 0);
        if (tempSelectedSnackIndex>N_SNACKS-1 || tempSelectedSnackIndex<0) {
            Serial.print(tempSelectedSnackIndex);
            Serial.println(" is out of range");
            addLog("out of range");
            lcd.print("Out of range"); //validate customKey in range
            return;
        }

        selectedSnackIndex=tempSelectedSnackIndex;

        Serial.print("snackNames[selectedSnackIndex] and selectedSnackIndex: ");
        Serial.print(snackNames[selectedSnackIndex]);
        Serial.println(selectedSnackIndex);

        String snackName=snackNames[selectedSnackIndex];
        char message[50]; //buffer
        sprintf(message, "%s (#%c)", snackName.c_str(), customKey);
        lcd.print(message);
        lcd.setCursor(0, 1);
        lcd.print("costs $");
        lcd.print(snackPrices[selectedSnackIndex]);
        displayBalance();
        Serial.println(message);
        addLog("Selected ", snackName);
        if (moneyInserted==0) {
            lcd.setCursor(0, 2);
            double snackPrice=snackPrices[selectedSnackIndex];
            if (snackPrice==1.0)
                lcd.print("Insert $1 bill");
            else if (snackPrice==2.0 || snackPrice==3.0 || snackPrice==4.0)
                lcd.print("Insert $1 bills");
            else if (snackPrice>=5.0)
                lcd.print("Insert $1 or $5 bill");
        }
    }

    // Bill Acceptor
    prevPulse=pulse;
    pulse=digitalRead(dollarPin);
    if (pulse==HIGH && prevPulse==LOW) {
        buttonLastPressed=millis();
        moneyInserted++;

        displayBalance();
        addLog("Inserted", String(moneyInserted));
    }

    if (moneyInserted!=0 && selectedSnackIndex!=-1) { //snack selected and money in
        if (moneyInserted>=snackPrices[selectedSnackIndex]) { //check if your balance affords the price of the snack
            turnOnce(selectedSnackIndex);
            addLog("Bought", String(selectedSnackIndex));
            lcd.clear();
            lcd.setCursor(0, 0);
            lcd.print("Thank you");
            lcd.setCursor(0, 1);
            lcd.print("Enjoy your");
            lcd.setCursor(0, 2);
            lcd.print(snackNames[selectedSnackIndex]);
            lcd.print("!");
            moneyInserted=0;
            selectedSnackIndex=-1;
        }
    }

    if (moneyInserted!=0 || selectedSnackIndex!=-1) { //snack selected or money in
        if (millis()-buttonLastPressed>idleDurationBeforeReset) { //idle duration
            Serial.println("Been inactive for too long");
            moneyInserted=0;
            reset();
        }
    }
}

void reset() {
    selectedSnackIndex=-1;
    lcd.clear();
    lcd.setCursor(0, 0);
}

void displayBalance() {
    if (moneyInserted==0) return; //don't show a balance of 0
    
    char message[50];
    sprintf(message, "You inserted $%d", (int)moneyInserted);
    lcd.setCursor(0, 2);
    lcd.print(message);
}
