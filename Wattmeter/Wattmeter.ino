#include <Wire.h> 
#include <LiquidCrystal_I2C.h>
#include <Bounce2.h>
#include "MenuItem.h"

LiquidCrystal_I2C lcd(0x27, 16, 2);

#define START_MENU_POS 5
#define MAX_MENU 5
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
static char menu4[]  = "Slope (R)";
static char menu5[]  = "Y Intercept (R)";

static char mprefix[] = "pnum k";

float yIntercept = 72.413;
float slope = 0.165;
float yInterceptR = 70.938;
float slopeR = 0.164;
float inputAttenuation = 0.0;
float currentMenuVal = 0;

MenuItem menu[5] =
{
  MenuItem( menu1, &inputAttenuation, 2, 1 ),
  MenuItem( menu2, &slope, 3, 3 ),
  MenuItem( menu3, &yIntercept, 3, 3 ),
  MenuItem( menu4, &slopeR, 3, 3 ),
  MenuItem( menu5, &yInterceptR, 3, 3 )
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
volatile int encoderDirection;


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

  //lcd.begin( 16, 2 );
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

    encoderDirection = encoderPos > oldEncoderPos ? 1 : -1;
      
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
    lcd.clear();

    if ( encoderDirection > 0 )
      menuNumber++;
    else
      menuNumber--;

    if ( menuNumber < 0 )
      menuNumber = MAX_MENU-1;
    else if ( menuNumber >= MAX_MENU )
      menuNumber = 0;
}

void changeMenuItem()
{
  char buf[20];
  MenuItem m = menu[menuNumber];
  
  m.getMenuValue( buf, currentMenuVal );
  m.changeMenuItem( buf, currentCursorPos, encoderDirection );
  currentMenuVal = m.getMenuValue( buf );
 
}


void editPressed()
{
  if ( menuDepth == 0 )
    return;

  MenuItem m = menu[menuNumber];

  if (!editing )
  {
    editing = true;
    currentCursorPos = 0;
  }
  else if ( currentCursorPos < m.getTotalpos()-1 )
    currentCursorPos++; 
  else
  {
    editing = false;
    MenuItem m = menu[menuNumber];
    m.setValue( currentMenuVal );
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
  MenuItem m = menu[menuNumber];
  
  lcd.setCursor( 0, 0 );
  lcd.print( m.getMenuName() );

  if ( !editing )
    currentMenuVal = *m.getValue();
  displayMenuValue(m);
}

void displayMenuValue( MenuItem m )
{
  char buf[20];

  m.getMenuValue( buf, currentMenuVal );
  lcd.setCursor( START_MENU_POS, 1 );
  lcd.print( buf );

  if ( editing )
  {
    int pos = currentCursorPos;
    if ( pos > m.getIntpos()-1 )
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

  int reflectedVoltage = analogRead(reflectedVoltagePin) - referenceVoltage + 512;
  float reflecteddbm = (float) reflectedVoltage * slopeR - yInterceptR + inputAttenuation;
  
  int forwardVoltage = analogRead(forwardVoltagePin) - referenceVoltage + 512;
  float forwarddbm = (float) forwardVoltage * slope - yIntercept + inputAttenuation;

  printWatts( reflecteddbm, 0, 0 );
  printWatts( forwarddbm, 0, 1 );

  printdbm( forwarddbm, 9, 1 );
  printdbm( reflecteddbm, 9, 0 );
}

void printWatts( float dbm, int x, int y )
{
  char buf[20];

  int prefix = ( dbm + 90 ) / 30;
  float remain = dbm - ( prefix * 30 - 90.0 );
  remain = pow( 10, remain / 10.0 );

  int intPortion = (int) remain;
  int floatPortion = abs( ( remain - (float)intPortion ) * 10 );
  sprintf( buf, "%3d.%01d %cW", intPortion, floatPortion, mprefix[prefix] );
  lcd.setCursor( x, y );
  lcd.print( buf );
}

void printdbm( float dbm, int x, int y )
{
  char buf[20];
  
  int intPortion = (int) dbm;
  int floatPortion = abs( ( dbm - (float)intPortion ) * 10 );
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
