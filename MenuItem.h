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
    
 protected:
 private:

     char* _name;
     float* _value;
     int    _intpos;
     int    _floatpos;
};

#endif
