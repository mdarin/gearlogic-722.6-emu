#ifndef CORE_H
#define CORE_H

#include <stdint.h>
#include <stdbool.h>
#include "maps.h"
#include "params.h"
#include "sens.h"

void switchGearStart(int cSolenoid, int spcVal, int mpcVal);
void switchGearStop();

void gearchangeUp(int newGear);
void gearchangeDown(int newGear);

// void decideGear(void *me);
int evaluateGear();
float ratioFromGear(int inputGear);
int gearFromRatio(float inputRatio);
float getGearSlip();

void doPreShift();
void doShift();
void doPostShift();
// void faultMon(Task *me);

#endif // CORE_H
