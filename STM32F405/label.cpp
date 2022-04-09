#include"label.h"
int function(int unused, ...)
{
va_list args;
va_start(args, unused);
return va_arg(args, int);
}