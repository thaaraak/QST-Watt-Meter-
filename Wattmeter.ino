#include <Wire.h> 
#include <LiquidCrystal_I2C.h>

LiquidCrystal_I2C lcd(0x27, 16, 2);

int referenceVoltagePin = A0;
int forwardVoltagePin = A2;



void setup() 
{
  lcd.begin();
  lcd.backlight();
}


void loop() 
{
  char buf[20];

  int referenceVoltage = analogRead(referenceVoltagePin);
  sprintf( buf, "Ref: %4d", referenceVoltage );
  lcd.setCursor(0,0);
  lcd.print( buf );

  int forwardVoltage = analogRead(forwardVoltagePin);
  sprintf( buf, "Fwd: %4d", forwardVoltage );
  lcd.setCursor(0,1);
  lcd.print( buf );



  
  delay(10); 
}
