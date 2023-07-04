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
byte pins[4]={46, 48, 50, 52};
Stepper motor(STEPS_PER_REVOLUTION, pins[0], pins[2], pins[1], pins[3]);  //Pin mapping: 1N1 -> 13, 1N2 -> 12, 1N3 -> 11, 1N4 -> 10


void setup() {
    Serial.begin(9600);
    Serial.println("Serial began");
    
    lcd.init();
    lcd.backlight();
    lcd.clear();
    lcd.setCursor(0, 0);
}


void loop() {
    // // Get value if keypad pressed
    // char customKey=keypad.getKey();
    
    // if (customKey) { //pressed key
    //     char message[50]; //buffer
    //     sprintf(message, "Selected %c.", customKey);
    //     Serial.println(message);

    //     lcd.clear();
    //     lcd.setCursor(0, 0);
    //     lcd.print(message);
    // }

    Serial.println("Stepping");
    motor.setSpeed(1000);
    motor.step(tau);
    Serial.println("Finished stepping");
    delay(1500);
}
