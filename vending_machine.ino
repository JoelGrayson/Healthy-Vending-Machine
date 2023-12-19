// # Global Constants
#define N_SNACKS 8
#define TAU 6.2831853071
#define NUM_REVS 1.03 //it is better for it to turn too much than too little

double durationsOfRevolution[]={ //how long it takes different springs to turn
    3350,
    3343,
    3342,
    3520,

    3071,
    3173,
    3070,
    3300
};

String snackNames[]={
    "Mushroom Jerky",
    "Honey Sticks",
    "Harvest Snaps",
    "Spicy Chickpeas",
    "Doritos",
    "Goldfish",
    "Ruffles",
    "Kit Kat"
};
double snackPrices[]={
    5.00,
    1.00,
    5.00,
    1.00,
    1.00,
    1.00,
    2.00,
    1.00
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
    Serial.print("Turning motor #"); Serial.println(motorI);
    turnInRadians(motorI, TAU);
}
void turnInRadians(uint8_t motorI, double radians) {
    double durationOfRevolution=durationsOfRevolution[motorI];
    double delayPerRadian=durationOfRevolution/TAU;
    turnInMS(motorI, delayPerRadian*radians);
}
void turnInMS(uint8_t motorI, double milliseconds) { //turn in milliseconds
    const int transistorPin=motorPins[motorI];
    
    digitalWrite(transistorPin, HIGH);
    delay(milliseconds);
    digitalWrite(transistorPin, LOW);
}
void startTurningMotor(uint8_t motorI) {
    const int transistorPin=motorPins[motorI];
    digitalWrite(transistorPin, HIGH);
}
void stopTurningMotor(uint8_t motorI) {
    const int transistorPin=motorPins[motorI];
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

    for (int i=0; i<8; i++) //better that it goes too much than too little so that a student gets 2 snacks instead of 0
        durationsOfRevolution[i]*=NUM_REVS;
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
        if (customKey=='#') { //press # for control panel
            #define password1 '1'
            #define password2 '2'
            #define password3 '3'
            #define password4 '4'

            Serial.println("Please enter the password to run an action");
            if (!waitForNextKey(password1)) return;
            if (!waitForNextKey(password2)) return;
            if (!waitForNextKey(password3)) return;
            if (!waitForNextKey(password4)) return;
            controlPanel();
            return;
        }
    
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
            lcd.print("Thank you. Enjoy the");
            lcd.setCursor(0, 1);
            lcd.print(snackNames[selectedSnackIndex]);
            lcd.print("!");
            moneyInserted=0;
            selectedSnackIndex=-1;
            delay(3000);
            lcd.clear();
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

bool waitForNextKey(char correctKey) { //2 seconds to press the key
    char pressedKey;
    for (uint8_t i=0; i<20; i++) {
        delay(100);
        pressedKey=keypad.getKey();
        if (pressedKey==correctKey) {
            Serial.print("Correct key: ");
            Serial.println(correctKey);
            return true;
        }
    }
    
    Serial.println("Aborting because pressed wrong key");
    return false;
}

void controlPanel() {
    // 0 for print log
    // 1–8 for turn a specific motor
        // one rotation in NORMAL mode
        // 1/10 rotation in MILLI mode
        // 1/100 rotation in MICRO mode
    // * for turn all motors
    // # for exiting control panel
    // 9 for change mode
    // # for exit
    
    displayMode("Normal");
    
    enum Mode {
        NORMAL,
        DECI,
        MILLI,
        MICRO,
        TEST_DURATION
    };

    Mode mode=NORMAL;
    bool shiftPressed=false; //when pressed, small movements to motors
    
    while (true) {
        char pressedKey=keypad.getKey(); //get value if keypad pressed
        delay(50);

        if (mode==TEST_DURATION) {
            int motorI=keyToMotorI(pressedKey);

            if (motorI!=-1) {
                Serial.println("Testing");
                
                lcd.setCursor(0, 1);
                lcd.print("Turned for          ");
                const int incrementAmount=100;
                int milliseconds=0;

                startTurningMotor(motorI);
                long long startedTurningAt=millis();
                
                lcd.setCursor(0, 2);
                lcd.print("Press any key");
                lcd.setCursor(0, 3);
                lcd.print("to stop");

                while (true) {
                    // Stopped when any key pressed
                    char pressedKey=keypad.getKey();
                    if (pressedKey) {
                        long long stoppedTurningAt=millis();
                        int duration=(int)(stoppedTurningAt-startedTurningAt);

                        displayMode("Test Duration");
                        lcd.setCursor(0, 2);
                        lcd.print("Finished");
                        lcd.setCursor(0, 3);
                        lcd.print(duration);
                        lcd.print(" ms");
                        stopTurningMotor(motorI);
                        break;
                    }

                    delay(20);
                }
                continue;
            }
        }
        
        // Set move amount
        double moveAmount;
        switch (mode) {
            case NORMAL: moveAmount=TAU;     break;
            case DECI:   moveAmount=TAU/4;   break;
            case MILLI:  moveAmount=TAU/10;  break;
            case MICRO:  moveAmount=TAU/100; break;
            default:     moveAmount=TAU;     break;
        }
        
        // Perform action
        switch (pressedKey) {
            // Turn motor
            case '1': turnInRadians(0, moveAmount); break;
            case '2': turnInRadians(1, moveAmount); break;
            case '3': turnInRadians(2, moveAmount); break;
            case '4': turnInRadians(3, moveAmount); break;
            case '5': turnInRadians(4, moveAmount); break;
            case '6': turnInRadians(5, moveAmount); break;
            case '7': turnInRadians(6, moveAmount); break;
            case '8': turnInRadians(7, moveAmount); break;

            // Print log to serial port if computer is connected
            case '0':
                Serial.println("Print log");
                printLog();
                break;
            
            // Turn all springs
            case '*':
                Serial.print("Turning all springs from 1 to ");
                Serial.println(N_SNACKS);
                for (uint8_t i=0; i<N_SNACKS; i++)
                    turnOnce(i);
                break;
            
            // Change mode
            case '9':
                switch (mode) {
                    case NORMAL:         mode=DECI;          displayMode("Deci");          break;
                    case DECI:           mode=MILLI;         displayMode("Milli");         break;
                    case MILLI:          mode=MICRO;         displayMode("Micro");         break;
                    case MICRO:          mode=TEST_DURATION; displayMode("Test Duration"); break;
                    case TEST_DURATION:  mode=NORMAL;        displayMode("Normal");        break;
                }
                break;
            
            case '#':
                lcd.clear();
                lcd.setCursor(0, 0);
                lcd.print("Exiting");
                delay(600);
                lcd.clear();
                lcd.setCursor(0, 0);
                return;
        }
    }

}

void displayMode(char modeName[]) {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("---Control Panel---");
    lcd.setCursor(0, 1);
    lcd.print(modeName);
    lcd.print(" Mode");
}

int keyToMotorI(char key) {
    switch (key) {
        case '1': return 0;
        case '2': return 1;
        case '3': return 2;
        case '4': return 3;
        case '5': return 4;
        case '6': return 5;
        case '7': return 6;
        case '8': return 7;
        default:  return -1;
    }
}
