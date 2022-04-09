#ifndef __LABEL__
#define __LABEL__
#include <stdarg.h>
#define degreeToMechanical(a) ((a)*8192.f/360.f)
#define mechanicalToDegree(a) ((a)*360.f/8192.f)
constexpr auto can1MotorNumber = 5;
constexpr auto can2MotorNumber = 4;
constexpr auto LOCK_MODE_ERROR = 70;
constexpr auto ONE_BULLET_HEAT = 10;
int function(int unused, ...);
#endif