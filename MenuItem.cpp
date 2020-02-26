// Please read Bounce2.h for information about the liscence and authors


#include "MenuItem.h"




MenuItem::MenuItem( const char *menuname, float *value, int intpos, int floatpos )
{
  _name = menuname;
  _value = value;
  _intpos = intpos;
  _floatpos = floatpos;
}
