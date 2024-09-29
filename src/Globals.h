#pragma once
#include <N2KTypes.h>

extern int counter;
extern int State;
extern int old_State;
extern int move_flag;
extern int button_flag;
extern int flesh_flag;
extern int shortButtonStateLatched;
extern int longButtonStateLatched;

extern double locEngRPM;
extern double locEngOilPres;
extern double locEngOilTemp;
extern double locEngCoolTemp; 
extern double locEngAltVolt; 
extern double locEngFuelRate; 
extern double locEngHours; 
extern double locEngCoolPres;
extern double locEngFuelPres;

extern double locCOG;
extern double locSOG;
extern tN2kHeadingReference locRef;
extern double locWindSpeed, locWindAngle;
extern tN2kWindReference locWindReference;
