// #define N_SNACKS 8
#define N_SNACKS 2

// # Liquid Crystal Display
#include <LiquidCrystal_I2C.h> //from github.com/johnrickman/LiquidCrystal_I2C

LiquidCrystal_I2C lcd(0x27, 20, 4); //20x4 display from amazon.com/gp/product/B01DKETWO2


// # Keypad
// Mark Stanley's Keypad library
#include <Keypad.h>

const byte N_ROWS=4;
const byte N_COLS=3;

char keys[N_ROWS][N_COLS]={
    { '1', '2', '3' },
    { '4', '5', '6' },
    { '7', '8', '9' },
    { '*', '0', '#' }
};

byte rowPins[N_ROWS]={ 9, 8, 7, 6 };
byte colPins[N_COLS]={ 5, 4, 3 };

Keypad keypad=Keypad(makeKeymap(keys), rowPins, colPins, N_ROWS, N_COLS);


// # Stepper Motor
#include <Stepper.h>
#define STEPS_PER_REVOLUTION 32
#define GEAR_REDUCTION 64
const float STEPS_PER_OUTPUT_REVOLUTION=STEPS_PER_REVOLUTION*GEAR_REDUCTION;
const float tau=STEPS_PER_OUTPUT_REVOLUTION; //one full revolution
// byte pins[4]={13, 12, 11, 10};
byte pins[N_SNACKS][4]={
    {46, 48, 50, 52}, //Pin mapping: 1N1 -> 46, 1N2 -> 48, 1N3 -> 50, 1N4 -> 52
    {53, 51, 49, 47},
    // {0, 0, 0, 0},
    // {0, 0, 0, 0},
    // {0, 0, 0, 0},
    // {0, 0, 0, 0},
    // {0, 0, 0, 0},
    // {0, 0, 0, 0},
};
Stepper motors[2]={
    Stepper(STEPS_PER_REVOLUTION, pins[0][0], pins[0][2], pins[0][1], pins[0][3]),
    Stepper(STEPS_PER_REVOLUTION, pins[1][0], pins[1][2], pins[1][1], pins[1][3])
};
// for (byte i=0; i<N_SNACKS; i++)
//     motors[i]=motor(STEPS_PER_REVOLUTION, pins[i][0], pins[i][2], pins[i][1], pins[i][3]);


void setup() {
    Serial.begin(9600);
    Serial.println("Serial began");
    
    lcd.init();
    lcd.backlight();
    lcd.clear();
    lcd.setCursor(0, 0);
}


void loop() {
    // Get value if keypad pressed
    char customKey=keypad.getKey();
    
    if (customKey) { //pressed key
        byte motorI=int(customKey)-int('1'); //offset from 1
        lcd.clear();
        lcd.setCursor(0, 0);

        // Validate customKey in range
        if (motorI>N_SNACKS-1) {
            lcd.print("Out of range");
            return;
        }
        
        char message[50]; //buffer
        sprintf(message, "Selected %c.", customKey);
        Serial.println(message);

        lcd.print(message);

        
        Serial.println("motorI");
        Serial.println(motorI);
        motors[motorI].setSpeed(1000);
        motors[motorI].step(tau);
    }
}
