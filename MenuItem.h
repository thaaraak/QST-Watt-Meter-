#ifndef MenuItem_h
#define MenuItem_h

#if defined(ARDUINO) && ARDUINO >= 100
#include "Arduino.h"
#else
#include "WProgram.h"
#endif

class MenuItem
{
 public:

     MenuItem( const char *menuname, float *value, int intpos, int floatpos );

     char* getMenuName();
     float* getValue();
     void setValue( float v );

     float getMenuValue( char buf[] );
     void getMenuValue( char buf[], float v );
     void changeMenuItem( char buf[], int bufpos, int dir );

     int getIntpos();
     int getTotalpos();
     
 protected:
 private:

     char* _name;
     float* _value;
     int    _intpos;
     int    _floatpos;
     char   _formatString[10];
};

#endif
