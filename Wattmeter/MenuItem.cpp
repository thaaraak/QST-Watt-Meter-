// Please read Bounce2.h for information about the liscence and authors


#include "MenuItem.h"




MenuItem::MenuItem( const char *menuname, float *value, int intpos, int floatpos )
{
  _name = menuname;
  _value = value;
  _intpos = intpos;
  _floatpos = floatpos;
  sprintf( _formatString, "%c0%dd.%c0%dd", '%', intpos, '%', floatpos );
}

char *MenuItem::getMenuName()
{
  return _name;
}

float* MenuItem::getValue()
{
  return _value;
}

void MenuItem::setValue( float v )
{
  *_value = v;
}

int MenuItem::getIntpos()
{
  return _intpos;
}

int MenuItem::getTotalpos()
{
  return _intpos + _floatpos;
}

float MenuItem::getMenuValue( char buf[] )
{
    float corr = 0.5;
    for ( int i = 0 ; i < _floatpos ; i++ )
      corr /= 10;
    return String( buf ).toFloat() + corr;
}

void MenuItem::getMenuValue( char buf[], float v )
{
  int intPortion = (int) v;
  int floatPortion = abs( ( v - (float)intPortion ) * pow( 10, _floatpos ) );
  sprintf( buf, _formatString, intPortion, floatPortion );
}

void MenuItem::changeMenuItem( char buf[], int bufpos, int dir )
{
  if ( bufpos < _intpos )
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
}
