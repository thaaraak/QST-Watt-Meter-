#include <Wire.h> 
#include <LiquidCrystal_I2C.h>
#include <Bounce2.h>
#include "MenuItem.h"

LiquidCrystal_I2C lcd(0x27, 16, 2);

#define START_MENU_POS 5
#define CURSOR_MAX 5
#define MAX_MENU 3
int menuDepth = 0;
int menuEdit = 0;
int menuNumber = 0;

bool editing = false;
int currentCursorPos = 0;

int referenceVoltagePin = A0;
int forwardVoltagePin = A1;
int reflectedVoltagePin = A3;
int sampleHoldPin = A2;
int editButton = 7;
Bounce dbEdit = Bounce();

static char menu1[]  = "Input Atten";
static char menu2[]  = "Slope";
static char menu3[]  = "Y Intercept";

float yIntercept = 72.413;
float slope = 0.165;
float inputAttenuation = 50.0;
float currentMenuVal = 0;

MenuItem menu[3] =
{
  MenuItem( menu1, &inputAttenuation, 3, 3 ),
  MenuItem( menu2, &slope, 3, 3 ),
  MenuItem( menu3, &yIntercept, 3, 3 )
};


static int pinA = 2; // Our first hardware interrupt pin is digital pin 2
static int pinB = 3; // Our second hardware interrupt pin is digital pin 3
static int pinEncoderSwitch = 8;
Bounce dbEncoder = Bounce();

volatile byte aFlag = 0; // let's us know when we're expecting a rising edge on pinA to signal that the encoder has arrived at a detent
volatile byte bFlag = 0; // let's us know when we're expecting a rising edge on pinB to signal that the encoder has arrived at a detent (opposite direction to when aFlag is set)
volatile byte reading = 0; //somewhere to store the direct values we read from our interrupt pins before checking to see if we have moved a whole detent

volatile int encoderPos = 0;
volatile int oldEncoderPos = 0;


void setup() 
{
  pinMode( sampleHoldPin, OUTPUT );
  digitalWrite( sampleHoldPin, HIGH );

  pinMode( editButton, INPUT_PULLUP );
  dbEdit.attach(editButton);
  dbEdit.interval(5);

  pinMode(pinEncoderSwitch, INPUT_PULLUP);
  dbEncoder.attach(pinEncoderSwitch);
  dbEncoder.interval(5);
  
  pinMode(pinA, INPUT_PULLUP); // set pinA as an input, pulled HIGH to the logic voltage (5V or 3.3V for most cases)
  pinMode(pinB, INPUT_PULLUP); // set pinB as an input, pulled HIGH to the logic voltage (5V or 3.3V for most cases)
  attachInterrupt(0,PinA,RISING); // set an interrupt on PinA, looking for a rising edge signal and executing the "PinA" Interrupt Service Routine (below)
  attachInterrupt(1,PinB,RISING); // set an interrupt on PinB, looking for a rising edge signal and executing the "PinB" Interrupt Service Routine (below)

  lcd.begin();
  lcd.backlight();
  lcd.clear();
}


void loop() 
{
    char buf[20];
    bool somethingChanged = false;

    dbEdit.update();
    if ( dbEdit.fell() )
    {
      editPressed();
      somethingChanged = true;
    }
    
    dbEncoder.update();
    if ( dbEncoder.fell() )
    {
      encoderPressed();
      somethingChanged = true;
    }
    
    if ( checkEncoderChanged() )
      somethingChanged = true;

    if ( menuDepth == 0 )
        displayWattmeter();
    else if ( somethingChanged )
        displayMenu();
    
    //delay( 50 );
}

bool checkEncoderChanged()
{
    if ( oldEncoderPos == encoderPos )
      return false;
      
    if ( menuDepth == 0 )
      changeWattmeter();
    else if ( menuDepth == 1 )
    {
      if ( !editing )
        changeMenu();
      else
        changeMenuItem();
    }
    
    oldEncoderPos = encoderPos;
    return true;
}

void changeWattmeter()
{
}

void changeMenu()
{
    if ( encoderPos > oldEncoderPos )
      menuNumber++;
    else if ( encoderPos < oldEncoderPos )
      menuNumber--;

    if ( menuNumber < 0 )
      menuNumber = MAX_MENU-1;
    else if ( menuNumber >= MAX_MENU )
      menuNumber = 0;
}

void changeMenuItem()
{
  char buf[20];
  
  int intPortion = (int) currentMenuVal;
  int floatPortion = abs( ( currentMenuVal - (float)intPortion ) * 1000 );
  sprintf( buf, "%03d.%03d", intPortion, floatPortion );

  int dir = encoderPos > oldEncoderPos ? 1 : -1;

  char c;
  int bufpos = currentCursorPos;
  
  if ( currentCursorPos < 3 )
  {
    buf[bufpos] += dir;
  }
  else
  {
    ++bufpos;
    buf[bufpos] += dir;
  }

  if ( buf[bufpos] < '0' )
    buf[bufpos] = '9';
  else if ( buf[bufpos] > '9' )
    buf[bufpos] = '0';
  
  currentMenuVal = String( buf ).toFloat() + .0005;
  
}


void editPressed()
{
  if ( menuDepth == 0 )
    return;
    
  if (!editing )
  {
    editing = true;
    currentCursorPos = 0;
  }
  else if ( currentCursorPos < CURSOR_MAX )
    currentCursorPos++; 
  else
  {
    editing = false;
   if ( menuNumber == 0 )
      inputAttenuation = currentMenuVal;
   else if ( menuNumber == 1 )
      slope = currentMenuVal;
   else
      yIntercept = currentMenuVal;
  } 
}

void encoderPressed()
{  
  lcd.clear();
  
  if ( menuDepth == 0 )
    menuDepth = 1;
  else
  {
    menuDepth = 0;
    currentCursorPos = 0;
    menuNumber = 0;
    lcd.noBlink();
  }
}

void displayMenu()
{
   if ( menuNumber == 0 )
      displayAttenuation();
   else if ( menuNumber == 1 )
      displayParam1();
   else
      displayParam2();
}

void displayAttenuation()
{
  lcd.setCursor( 0, 0 );
  lcd.print( "Input Atten     " );

  if ( !editing )
    currentMenuVal = inputAttenuation;
  displayMenuValue();
}

void displayParam1()
{
  lcd.setCursor( 0, 0 );
  lcd.print( "Slope           " );
  if ( !editing )
    currentMenuVal = slope;
  displayMenuValue();
}

void displayParam2()
{
  lcd.setCursor( 0, 0 );
  lcd.print( "Y Intercept     " );
  if ( !editing )
    currentMenuVal = yIntercept;
  displayMenuValue();
}

void displayMenuValue()
{
  char buf[20];
  
  int intPortion = (int) currentMenuVal;
  int floatPortion = abs( ( currentMenuVal - (float)intPortion ) * 1000 );
  sprintf( buf, "%03d.%03d", intPortion, floatPortion );
  lcd.setCursor( START_MENU_POS, 1 );
  lcd.print( buf );

  if ( editing )
  {
    int pos = currentCursorPos;
    if ( pos > 2 )
      pos++;
    lcd.setCursor( START_MENU_POS + pos, 1 );
    lcd.blink();
  }
  else
    lcd.noBlink();
}


void displayWattmeter()
{
  char buf[20];


  int referenceVoltage = analogRead(referenceVoltagePin);
/*
  sprintf( buf, "R: %3d", referenceVoltage );
  lcd.setCursor(7,1);
  lcd.print( buf );
*/
  int reflectedVoltage = analogRead(reflectedVoltagePin) - referenceVoltage + 512;
  sprintf( buf, "R: %3d", reflectedVoltage );
  lcd.setCursor(0,0);
  lcd.print( buf );

  int forwardVoltage = analogRead(forwardVoltagePin) - referenceVoltage + 512;
  sprintf( buf, "F: %3d", forwardVoltage );
  lcd.setCursor(0,1);
  lcd.print( buf );

  printdbm( forwardVoltage, 7, 1 );
  printdbm( reflectedVoltage, 7, 0 );
}

void printdbm( int v, int x, int y )
{
  char buf[20];
  
  float dBm = (float)v * slope - yIntercept;
  int intPortion = (int) dBm;
  int floatPortion = abs( ( dBm - (float)intPortion ) * 10 );
  sprintf( buf, "(%3d.%01d)", intPortion, floatPortion );
  lcd.setCursor( x, y );
  lcd.print( buf );
}

void changeDisplay( int upDownFlag )
{
  encoderPos += upDownFlag;
}

void PinA(){
  cli(); //stop interrupts happening before we read pin values
  reading = PIND & 0xC; // read all eight pin values then strip away all but pinA and pinB's values
  if(reading == B00001100 && aFlag) { //check that we have both pins at detent (HIGH) and that we are expecting detent on this pin's rising edge
    changeDisplay( -1 );
    bFlag = 0; //reset flags for the next turn
    aFlag = 0; //reset flags for the next turn
  }
  else if (reading == B00000100) bFlag = 1; //signal that we're expecting pinB to signal the transition to detent from free rotation
  sei(); //restart interrupts
}

void PinB(){
  cli(); //stop interrupts happening before we read pin values
  reading = PIND & 0xC; //read all eight pin values then strip away all but pinA and pinB's values
  if (reading == B00001100 && bFlag) { //check that we have both pins at detent (HIGH) and that we are expecting detent on this pin's rising edge
    changeDisplay( 1 );
    bFlag = 0; //reset flags for the next turn
    aFlag = 0; //reset flags for the next turn
  }
  else if (reading == B00001000) aFlag = 1; //signal that we're expecting pinA to signal the transition to detent from free rotation
  sei(); //restart interrupts
}
