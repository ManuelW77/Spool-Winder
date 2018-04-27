#include <Arduino.h>
#include <Stepper.h>
#include <Servo.h>
#include <LiquidCrystal.h>

int servoPin = 50;

Servo servo;

int servoAngle = 0;

// Steps per Revolution for Small Stepper
#define smallStepperStepsRev 32

// Steps per Revolution for Big Stepper
#define bigStepperStepsRev 200

// LCD Buttons
#define btnRIGHT  0
#define btnUP     1
#define btnDOWN   2
#define btnLEFT   3
#define btnSELECT 4
#define btnNONE   5

// Then the pins are entered here in the sequence 1-3-2-4 for proper sequencing
Stepper smallStepper(smallStepperStepsRev, 30, 31, 32, 33);
Stepper bigStepper(bigStepperStepsRev, 52, 53);

// setup for 'LCD Keypad Shield'
LiquidCrystal lcd(8,9,4,5,6,7);

/*-----( Declare Variables )-----*/
// Big Stepper 1 Revolution = 6400 Steps
// Small Stepper 1 Revolution = 2048 Steps
int smallRev    = 2048;
int bigRev      = 6400;
int revCounter  = 0;
int bigStepCounter = 0;
int lcd_key     = 0;
int lcd_key_prev = 0;
int adc_key_in  = 0;
bool startWind = false;
int whenJump = 3;
int wideJump = 10;
float servoPos = 0;

void setup()
{
  Serial.begin(115200);
  Serial.print("Program Start");

  servo.attach(servoPin);
  servo.write(servoPos);

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

int read_LCD_buttons(){               // read the buttons
    adc_key_in = analogRead(0);       // read the value from the sensor

    if (adc_key_in > 1000) return btnNONE;

    if (adc_key_in < 50)   return btnRIGHT;
    if (adc_key_in < 195)  return btnUP;
    if (adc_key_in < 380)  return btnDOWN;
    if (adc_key_in < 555)  return btnLEFT;
    if (adc_key_in < 790)  return btnSELECT;

    return btnNONE;                // when all others fail, return this.
}

void moveBigStepper(int Steps2Take, int StepsSpeed) {
  bigStepper.setSpeed(StepsSpeed);
  bigStepper.step(Steps2Take);
}

void moveSmallStepper(int Steps2Take, int StepsSpeed) {
  smallStepper.setSpeed(StepsSpeed);
  smallStepper.step(Steps2Take);
}

void loop()
{
  lcd_key_prev = lcd_key;
  lcd_key = read_LCD_buttons();   // read the buttons

  if (lcd_key != lcd_key_prev) {
    switch (lcd_key){
        case btnRIGHT:{
              wideJump += 10;
              lcd.setCursor(8,1); lcd.print("         ");
              lcd.setCursor(8,1); lcd.print("Wide:" + String(wideJump));
              break;
        }
        case btnLEFT:{
              if (wideJump > 1) wideJump -= 10;
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
                lcd.print("Stopping...");
              }
              break;
        }
      }
  }

  if (startWind == true) {
    if (revCounter < whenJump) {
      moveBigStepper(1, 1000);

      bigStepCounter++;
      if (bigStepCounter == bigRev) {
        revCounter++;
        bigStepCounter = 0;
      }
    }
    else {
      for (int i=0; i<=wideJump; i++) {
        moveBigStepper(1, 1000);

        if (servoPos >= 170.00) {
          servoPos = 0;
        }
        else {
          servoPos +=0.1;
        }
        servo.write(servoPos);
        Serial.println(servoPos);
      }
      revCounter = 0;
    }
  }
}
