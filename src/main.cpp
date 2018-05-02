#include <Arduino.h>
#include <Stepper.h>
#include <LiquidCrystal.h>

// Steps per Revolution for Big Stepper
#define bigStepperStepsRev 200

// LCD Buttons
#define btnRIGHT  0
#define btnUP     1
#define btnDOWN   2
#define btnLEFT   3
#define btnSELECT 4
#define btnNONE   5

unsigned int lowSpeed  = 6000; // Notabene: nicht über 16000
unsigned int highSpeed =  1000;

// setup for 'LCD Keypad Shield'
LiquidCrystal lcd(8,9,4,5,6,7);

// Then the pins are entered here in the sequence 1-3-2-4 for proper sequencing
Stepper bigStepper(bigStepperStepsRev, 30, 31);

// Setup small Stepper
const int motorPin1 = 32;  // Blue   - In 1
const int motorPin2 = 34;  // Pink   - In 2
const int motorPin3 = 36; // Yellow - In 3
const int motorPin4 = 38; // Orange - In 4

// Setup Endstop
const int endStop = 33;

/*-----( Declare Variables )-----*/
// Big Stepper 1 Revolution = 6400 Steps
// Small Stepper 1 Revolution = 2048 Steps
int bigRev      = 6400; // Schritte für eine Umdrehung
int revCounter  = 0;
int bigStepCounter = 0;
int lcd_key     = 0;
int lcd_key_prev = 0;
int adc_key_in  = 0;
bool startWind = false;
int whenJump = 1; // Voreinstellung nach wie viel Umdrehungen gesprungen wird
int wideJump = 4; // Voreinstellung wie weit gesprungen wird
float servoPos = 0;
bool goLeft = true;
int leftSteps = 0;
int farToJump = 1; // set Stepweite auf dem Display

void setup() {
  Serial.begin(115200);
  Serial.print("Program Start");

  pinMode(motorPin1, OUTPUT);
  pinMode(motorPin2, OUTPUT);
  pinMode(motorPin3, OUTPUT);
  pinMode(motorPin4, OUTPUT);

  pinMode(endStop, INPUT);

  // set up the LCD //////////////////////
  lcd.begin(2, 16); // Set the size of the LCD
  lcd.clear(); // Clear the screen
  lcd.setCursor(0,0); // Set cursor for next line
  lcd.print("Spool Winder"); // Print this line
  lcd.setCursor(0,1); // Set cursor for next line
  lcd.print("by ManuelW"); // Print this line
  delay(3000); // Wait 3 seconds

  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Waiting...");
  lcd.setCursor(0,1);
  lcd.print("Turn:" + String(whenJump));
  lcd.setCursor(8,1);
  lcd.print("Wide:" + String(wideJump));
}

int read_LCD_buttons() {
    adc_key_in = analogRead(0); // read the value from the sensor

    if (adc_key_in > 1000) return btnNONE;

    if (adc_key_in < 50)   return btnRIGHT;
    if (adc_key_in < 195)  return btnUP;
    if (adc_key_in < 380)  return btnDOWN;
    if (adc_key_in < 555)  return btnLEFT;
    if (adc_key_in < 790)  return btnSELECT;

    return btnNONE; // when all others fail, return this.
}

void moveBigStepper(int Steps2Take, int StepsSpeed) {
  bigStepper.setSpeed(StepsSpeed);
  bigStepper.step(Steps2Take);
}

void rechtsrum(unsigned int motorSpeed) {
  // 1
  digitalWrite(motorPin4, HIGH);
  digitalWrite(motorPin3, LOW);
  digitalWrite(motorPin2, LOW);
  digitalWrite(motorPin1, LOW);
  delayMicroseconds(motorSpeed);

  // 2
  digitalWrite(motorPin4, HIGH);
  digitalWrite(motorPin3, HIGH);
  digitalWrite(motorPin2, LOW);
  digitalWrite(motorPin1, LOW);
  delayMicroseconds(motorSpeed);

  // 3
  digitalWrite(motorPin4, LOW);
  digitalWrite(motorPin3, HIGH);
  digitalWrite(motorPin2, LOW);
  digitalWrite(motorPin1, LOW);
  delayMicroseconds(motorSpeed);

  // 4
  digitalWrite(motorPin4, LOW);
  digitalWrite(motorPin3, HIGH);
  digitalWrite(motorPin2, HIGH);
  digitalWrite(motorPin1, LOW);
  delayMicroseconds(motorSpeed);

  // 5
  digitalWrite(motorPin4, LOW);
  digitalWrite(motorPin3, LOW);
  digitalWrite(motorPin2, HIGH);
  digitalWrite(motorPin1, LOW);
  delayMicroseconds(motorSpeed);

  // 6
  digitalWrite(motorPin4, LOW);
  digitalWrite(motorPin3, LOW);
  digitalWrite(motorPin2, HIGH);
  digitalWrite(motorPin1, HIGH);
  delayMicroseconds(motorSpeed);

  // 7
  digitalWrite(motorPin4, LOW);
  digitalWrite(motorPin3, LOW);
  digitalWrite(motorPin2, LOW);
  digitalWrite(motorPin1, HIGH);
  delayMicroseconds(motorSpeed);

  // 8
  digitalWrite(motorPin4, HIGH);
  digitalWrite(motorPin3, LOW);
  digitalWrite(motorPin2, LOW);
  digitalWrite(motorPin1, HIGH);
  delayMicroseconds(motorSpeed);
}

void linksrum(unsigned int motorSpeed) {
  // 1
  digitalWrite(motorPin1, HIGH);
  digitalWrite(motorPin2, LOW);
  digitalWrite(motorPin3, LOW);
  digitalWrite(motorPin4, LOW);
  delayMicroseconds(motorSpeed);

  // 2
  digitalWrite(motorPin1, HIGH);
  digitalWrite(motorPin2, HIGH);
  digitalWrite(motorPin3, LOW);
  digitalWrite(motorPin4, LOW);
  delayMicroseconds(motorSpeed);

  // 3
  digitalWrite(motorPin1, LOW);
  digitalWrite(motorPin2, HIGH);
  digitalWrite(motorPin3, LOW);
  digitalWrite(motorPin4, LOW);
  delayMicroseconds(motorSpeed);

  // 4
  digitalWrite(motorPin1, LOW);
  digitalWrite(motorPin2, HIGH);
  digitalWrite(motorPin3, HIGH);
  digitalWrite(motorPin4, LOW);
  delayMicroseconds(motorSpeed);

  // 5
  digitalWrite(motorPin1, LOW);
  digitalWrite(motorPin2, LOW);
  digitalWrite(motorPin3, HIGH);
  digitalWrite(motorPin4, LOW);
  delayMicroseconds(motorSpeed);

  // 6
  digitalWrite(motorPin1, LOW);
  digitalWrite(motorPin2, LOW);
  digitalWrite(motorPin3, HIGH);
  digitalWrite(motorPin4, HIGH);
  delayMicroseconds(motorSpeed);

  // 7
  digitalWrite(motorPin1, LOW);
  digitalWrite(motorPin2, LOW);
  digitalWrite(motorPin3, LOW);
  digitalWrite(motorPin4, HIGH);
  delayMicroseconds(motorSpeed);

  // 8
  digitalWrite(motorPin1, HIGH);
  digitalWrite(motorPin2, LOW);
  digitalWrite(motorPin3, LOW);
  digitalWrite(motorPin4, HIGH);
  delayMicroseconds(motorSpeed);
}

void stop() {
  digitalWrite(motorPin4, LOW);
  digitalWrite(motorPin3, LOW);
  digitalWrite(motorPin2, LOW);
  digitalWrite(motorPin1, LOW);
}

void loop()
{
  lcd_key_prev = lcd_key;
  lcd_key = read_LCD_buttons();   // read the buttons

  if (lcd_key != lcd_key_prev) {
    switch (lcd_key){
        case btnRIGHT:{
              wideJump += farToJump;
              lcd.setCursor(8,1); lcd.print("         ");
              lcd.setCursor(8,1); lcd.print("Wide:" + String(wideJump));
              break;
        }
        case btnLEFT:{
              if (wideJump > 1) wideJump -= farToJump;
              lcd.setCursor(8,1); lcd.print("         ");
              lcd.setCursor(8,1); lcd.print("Wide:" + String(wideJump));
              break;
        }
        case btnUP:{
              whenJump += 1;
              lcd.setCursor(0,1); lcd.print("       ");
              lcd.setCursor(0,1); lcd.print("Turn:" + String(whenJump));
              break;
        }
        case btnDOWN:{
              if (whenJump > 1) whenJump -= 1;
              lcd.setCursor(0,1); lcd.print("       ");
              lcd.setCursor(0,1); lcd.print("Turn:" + String(whenJump));
              break;
        }
        case btnSELECT:{
              //lcd.print("SELECT");  //  push button "SELECT" and show the word on the screen
              lcd.setCursor(0,0);
              lcd.print("                ");
              lcd.setCursor(0,0);
              if (startWind == false) {
                startWind = true;
                lcd.print("Running...");
              }
              else {
                startWind = false;
                stop();
                lcd.print("Stopping...");
              }
              break;
        }
      }
  }

  if (startWind == true) {
    if (revCounter < whenJump) {
      moveBigStepper(1, 500);

      bigStepCounter++;
      if (bigStepCounter == bigRev) {
        revCounter++;
        bigStepCounter = 0;
      }
    }
    else {
      // Endstop auslesen und Richtung bestimmen
      if (digitalRead(endStop) == false) {
        goLeft = false;
      }
      else if (goLeft == true) {
        leftSteps++;
      }

      for (int i=0; i<=wideJump; i++) {
        moveBigStepper(1, 500);

        if (goLeft == false && leftSteps > 0) {
          rechtsrum(highSpeed);
        }
        else {
          linksrum(highSpeed);
        }
      }

      if (goLeft == false && leftSteps > 0) {
        leftSteps--;
      }
      if (goLeft == false && leftSteps == 0) {
        goLeft = true;
      }

      Serial.println(leftSteps);
      revCounter = 0;
    }
  }
}
