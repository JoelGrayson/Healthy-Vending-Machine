// # Global Constants
// #define N_SNACKS 8
#define N_SNACKS 2


// # Liquid Crystal Display
#include <LiquidCrystal_I2C.h> //from github.com/johnrickman/LiquidCrystal_I2C

LiquidCrystal_I2C lcd(0x27, 20, 4); //20x4 display from amazon.com/gp/product/B01DKETWO2
    // Pin 9–13

// # Keypad
// Mark Stanley's Keypad library
#include <Keypad.h>

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




// # Stepper Motor
#include <Stepper.h>
Stepper* motors=malloc(sizeof(Stepper)*N_SNACKS);

uint8_t pins[N_SNACKS][4]={
    {46, 48, 50, 52}, //Pin mapping: 1N1 -> 46, 1N2 -> 48, 1N3 -> 50, 1N4 -> 52
    {47, 49, 51, 53},
    // {0, 0, 0, 0},
    // {0, 0, 0, 0},
    // {0, 0, 0, 0},
    // {0, 0, 0, 0},
    // {0, 0, 0, 0},
    // {0, 0, 0, 0},
};

#define STEPS_PER_REVOLUTION 32
#define GEAR_REDUCTION 64
const float STEPS_PER_OUTPUT_REVOLUTION=STEPS_PER_REVOLUTION*GEAR_REDUCTION;
const float tau=STEPS_PER_OUTPUT_REVOLUTION; //one full revolution


void setup() {
    Serial.begin(9600);
    Serial.println("Serial began");
    
    lcd.init();
    lcd.backlight();
    lcd.clear();
    lcd.setCursor(0, 0);

    for (int i=0; i<N_SNACKS; i++)
        motors[i]=Stepper(STEPS_PER_REVOLUTION, pins[0][0], pins[0][2], pins[0][1], pins[0][3]);
}


void loop() {
    char customKey=keypad.getKey(); //get value if keypad pressed
    
    if (customKey) { //key pressed
        uint8_t motorI=int(customKey)-int('1'); //offset from 1
        lcd.clear();
        lcd.setCursor(0, 0);

        if (motorI>N_SNACKS-1) { //validate customKey in range
            return lcd.print("Out of range");
        }
        
        char message[50]; //buffer
        sprintf(message, "Selected %c.", customKey);
        Serial.println(message);

        lcd.print(message);

        Serial.print("motorI"); Serial.println(motorI);
        
        motors[motorI].setSpeed(1000);
        motors[motorI].step(tau);
    }
}
