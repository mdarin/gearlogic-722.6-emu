// #include <Arduino.h>
// #include "include/pins.h"
// #include "include/calc.h"
// #include "include/maps.h"
// #include "include/sensors.h"
// #include "include/input.h"
// #include "include/core.h"
// #include "include/config.h"
// #include <SoftTimer.h>
#include <stdio.h>
#include <stdbool.h>
#include "core.h"
#include "sens.h"
#include "mock_aruduino.h"
#include "params.h"
#include "calc.h"

// CORE
// input:pollstick -> core:decideGear -> core:gearChange[Up|Down] -> core:switchGearStart -> core:boostControl
// input:polltrans -> core:switchGearStop
// poll -> evaluateGear

// Gear shift logic
// Beginning of gear change phase
// Send PWM signal to defined solenoid in transmission conductor plate.
void switchGearStart(int cSolenoid, int spcVal, int mpcVal)
{
  delaySinceLast = millis() - shiftStopTime;

  if (debugEnabled)
  {
    printf("[switchGearStart->switchGearStart] Begin of gear change current-solenoid: [%d -> %d]\n", gear, newGear);
  }

  if (trans)
  {
    shiftBlocker = true;   // Blocking any other shift operations during the shift
    preShift = true;       // Poke start of preShift mode (input.cpp polltrans checks these).
    postShiftDone = false; // Reset previous states
    preShiftDone = false;
    shiftDone = false;
    shiftConfirmed = false;

    spcPercentVal = spcVal;
    mpcPercentVal = mpcVal;
    cSolenoidEnabled = cSolenoid;
  }
  else
  {
    if (debugEnabled)
    {
      printf("[switchGearStart->switchGearStart] blocking change or transmission disabled delaySinceLast: %lu  nextShiftDelay: %d/n",
             delaySinceLast, config.nextShiftDelay);
    }
  }
}

void doPreShift()
{
  struct SensorVals sensor = readSensors();

  // Things to do before actual shift, example is for waiting boost to settle to the target. (boostControl should let it drop in this scenario).
  if (((sensor.curBoostLim > 0 && (sensor.curBoost <= sensor.curBoostLim - config.boostDrop)) || sensor.curBoostLim == 0) || !boostLimitShift || !boostLimit)
  {
    preShift = false;
    preShiftDone = true;

    if (debugEnabled)
    {
      printf("[switchGearStart->preShift] curBoost: %d  curBoostLim: %d\n",
             sensor.curBoost, sensor.curBoostLim);
    }
  }
  else
  {
    printf("[switchGearStart->preShift] blocking curBoost: %d  curBoostLim: %d\n",
           sensor.curBoost, sensor.curBoostLim);
  }
}

void doShift()
{
  // Actual shift
  int spcSetVal = (100 - spcPercentVal) * 2.55; // convert to uint8_t 255
  int mpcSetVal = (100 - mpcPercentVal) * 2.55; // convert to uint8_t 255

  if (adaptive)
  {
    int spcModVal = adaptSPC(lastMapVal, lastXval, lastYval);
    if (spcModVal < 10)
    {
      spcModVal = 10;
    };
    if (spcModVal > 190)
    {
      spcModVal = 200;
    };
    spcPercentVal = spcModVal / 100 * spcSetVal;
  }

  if (spcPercentVal > 100)
  {
    // to make sure we're on the bounds.
    spcPercentVal = 100;
    if (debugEnabled)
    {
      printf("[switchGearStart->switchGearStart] SPC high limit hit.\n");
    }
  }
  if (spcPercentVal < 10)
  {
    // to be on safe side.
    spcPercentVal = 10;
    if (debugEnabled)
    {
      printf("[switchGearStart->switchGearStart] SPC low limit hit.\n");
    }
  }

  //! these are calculated twice to make sure if there is changes they are noted.
  spcSetVal = (100 - spcPercentVal) * 2.55;
  mpcSetVal = (100 - mpcPercentVal) * 2.55;

  int spcPressureNormalized = pressureNormalization(spcSetVal);
  int mpcPressureNormalized = pressureNormalization(mpcSetVal);
  int onPressureNormalized = pressureNormalization(100);

  shiftStartTime = millis(); // Beginning to count shiftStartTime

  // pinmode change is due the fact how n2/n3 speed sensors change during the shift.
  // pinMode(n2pin, INPUT); // N2 sensor
  // pinMode(n3pin, INPUT); // N3 sensor
  // analogWrite(tcc, 0);
  // analogWrite(spc, spcPressureNormalized);
  // analogWrite(mpc, mpcPressureNormalized);
  // ===
  // analogWrite(cSolenoidEnabled, onPressureNormalized); // Beginning of gear change
  // OR
  // digitalWrite(cSolenoidEnabled, HIGH);
  // ===

  if (debugEnabled)
  {
    printf("[switchGearStart->doShift] spcPercentVal/mpcPercentVal %d/%d\n", spcPercentVal, mpcPercentVal);
  }
  preShiftDone = false;
  shiftDone = true;
}

void doPostShift()
{
  //* You can do post shift stuff here.
  if (debugEnabled)
  {
    printf("[switchGearStart->postShift] completed. \n");
  }

  postShift = false;
  postShiftDone = true;
  shiftPending = false;
  shiftBlocker = false;
  lastShiftPoint = millis();
  wrongGearPoint = 0;
}

// End of gear change phase
void switchGearStop()
{
  // analogWrite(cSolenoidEnabled, 0); // turn shift solenoid off
  // digitalWrite(cSolenoidEnabled, LOW);
  // analogWrite(spc, 0);          // spc off
  // analogWrite(mpc, 0);          // mpc off
  // pinMode(n2pin, INPUT_PULLUP); // N2 sensor
  // pinMode(n3pin, INPUT_PULLUP); // N3 sensor

  prevGear = gear;
  gear = pendingGear; // we can happily say we're on new gear

  shiftStopTime = millis();

  if (debugEnabled)
  {
    printf("[switchGearStop->switchGearStop] End of gear change current-solenoid: %d - %d - %d", pendingGear, newGear, cSolenoid);
  }
  shiftStartTime = 0;
  postShift = true;
}

// upshift parameter logic gathering
void gearchangeUp(int newGear)
{
  struct SensorVals sensor = readSensors();

  if (shiftBlocker == false && shiftPending == true && sensor.curRPM >= config.lowRPMshiftLimit)
  {
    pendingGear = newGear;
    shiftLoad = sensor.curLoad;
    shiftAtfTemp = sensor.curAtfTemp;

    if (debugEnabled)
    {
      printf("[gearChangeUp->gearChangeUp] performing change prev-new: %d -> %d\n", gear, newGear);
    }

    switch (newGear)
    {
    case 1: // -------------------------------------------------------------------------
      gear = 1;
      break;

    case 2: // -------------------------------------------------------------------------
      if (debugEnabled)
      {
        printf("[gearchangeUp->switchGearStart] Solenoid y3 requested with spcMap12/mpcMap12, load/atfTemp %d - %d\n", sensor.curLoad, sensor.curAtfTemp);
      }
      if (!tpsSensor)
      {
        switchGearStart(y3, config.oneTotwo, config.oneTotwo);
      }
      if (tpsSensor)
      {
        lastMapVal = 100;

        switchGearStart(y3, readPercentualMap(spcMap12, sensor.curLoad, sensor.curAtfTemp), readPercentualMap(mpcMap12, sensor.curLoad, sensor.curAtfTemp));
      }
      break;

    case 3: // -------------------------------------------------------------------------
      if (debugEnabled)
      {
        printf("[gearchangeUp->switchGearStart] Solenoid y4 requested with spcMap23/mpcMap23, load/atfTemp %d, %d\n", sensor.curLoad, sensor.curAtfTemp);
      }
      if (!tpsSensor)
      {
        switchGearStart(y4, config.twoTothree, config.twoTothree);
      }
      if (tpsSensor)
      {
        lastMapVal = 130;

        switchGearStart(y4, readPercentualMap(spcMap23, sensor.curLoad, sensor.curAtfTemp), readPercentualMap(mpcMap23, sensor.curLoad, sensor.curAtfTemp));
      }
      break;

    case 4: // -------------------------------------------------------------------------
      if (debugEnabled)
      {
        printf("[gearchangeUp->switchGearStart] Solenoid y5 requested with spcMap34/mpcMap34 %d, load/atfTemp %d\n", sensor.curLoad, sensor.curAtfTemp);
      }
      if (!tpsSensor)
      {
        switchGearStart(y5, config.threeTofour, config.threeTofour);
      }
      if (tpsSensor)
      {
        lastMapVal = 160;

        switchGearStart(y5, readPercentualMap(spcMap34, sensor.curLoad, sensor.curAtfTemp), readPercentualMap(mpcMap34, sensor.curLoad, sensor.curAtfTemp));
      }
      break;

    case 5: // -------------------------------------------------------------------------
      if (debugEnabled)
      {
        printf("[gearchangeUp->switchGearStart] Solenoid y3 requested with spcMap45/mpcMap45 %d, load/atfTemp %d \n", sensor.curLoad, sensor.curAtfTemp);
      }
      if (!tpsSensor)
      {
        switchGearStart(y3, config.fourTofive, config.fourTofive);
      }
      if (tpsSensor)
      {
        lastMapVal = 190;

        switchGearStart(y3, readPercentualMap(spcMap45, sensor.curLoad, sensor.curAtfTemp), readPercentualMap(mpcMap45, sensor.curLoad, sensor.curAtfTemp));
      }
      break;

    default: // -------------------------------------------------------------------------
      break;
    }
  }
  else
  {
    if (debugEnabled)
    {
      printf("[gearChangeUp->gearChangeUp] Blocking change %d / %d\n", shiftBlocker, shiftPending);
    }
    if (debugEnabled && sensor.curRPM < config.lowRPMshiftLimit)
    {
      printf("[gearChangeUp->gearChangeUp] low rpm limit hit with RPM: %d limit: %d\n", sensor.curRPM, config.lowRPMshiftLimit);

      if (!shiftBlocker)
      {
        shiftPending = false;
      }
    }
  }
}

// downshift parameter logic gathering
void gearchangeDown(int newGear)
{
  struct SensorVals sensor = readSensors();

  if (shiftBlocker == false && shiftPending == true && sensor.curRPM < config.highRPMshiftLimit)
  {
    pendingGear = newGear;
    shiftLoad = sensor.curLoad;
    shiftAtfTemp = sensor.curAtfTemp;

    if (debugEnabled)
    {
      printf("[gearChangeDown->gearChangeDown] performing change prev->new [%d -> %d] gear\n", gear, newGear);
    }

    switch (newGear)
    {
    case 1: // -------------------------------------------------------------------------
      if (debugEnabled)
      {
        printf("[gearchangeDown->switchGearStart] Solenoid y3 requested with spcMap21/mpcMap21  load: %d  atfTemp %d\n",
               sensor.curLoad, sensor.curAtfTemp);
      }
      if (!tpsSensor)
      {
        switchGearStart(y3, config.twoToone, config.twoToone);
      }
      if (tpsSensor)
      {
        lastMapVal = 210;

        switchGearStart(y3, readPercentualMap(spcMap21, sensor.curLoad, sensor.curAtfTemp), readPercentualMap(mpcMap21, sensor.curLoad, sensor.curAtfTemp));
      }
      break;

    case 2: // -------------------------------------------------------------------------
      if (debugEnabled)
      {
        printf("[gearchangeDown->switchGearStart] Solenoid y4 requested with spcMap32/mpcMap32  load: %d  atfTemp: %d\n",
               sensor.curLoad, sensor.curAtfTemp);
      }
      if (!tpsSensor)
      {
        switchGearStart(y4, config.threeTotwo, config.threeTotwo);
      }
      if (tpsSensor)
      {
        lastMapVal = 240;

        switchGearStart(y4, readPercentualMap(spcMap32, sensor.curLoad, sensor.curAtfTemp), readPercentualMap(mpcMap32, sensor.curLoad, sensor.curAtfTemp));
      }
      break;

    case 3: // -------------------------------------------------------------------------
      if (debugEnabled)
      {
        printf("[gearchangeDown->switchGearStart] Solenoid y5 requested with spcMap43/mpcMap43  load: %d  atfTemp: %d\n",
               sensor.curLoad, sensor.curAtfTemp);
      }
      if (!tpsSensor)
      {
        switchGearStart(y5, config.fourTothree, config.fourTothree);
      }
      if (tpsSensor)
      {
        lastMapVal = 270;

        switchGearStart(y5, readPercentualMap(spcMap43, sensor.curLoad, sensor.curAtfTemp), readPercentualMap(mpcMap43, sensor.curLoad, sensor.curAtfTemp));
      }
      break;

    case 4: // -------------------------------------------------------------------------
      if (debugEnabled)
      {
        printf("[gearchangeDown->switchGearStart] Solenoid y3 requested with spcMap54/mpcMap54  load: %d  atfTemp: %d\n",
               sensor.curLoad, sensor.curAtfTemp);
      }
      if (!tpsSensor)
      {
        switchGearStart(y3, config.fiveTofour, config.fiveTofour);
      }
      if (tpsSensor)
      {
        lastMapVal = 300;

        switchGearStart(y3, readPercentualMap(spcMap54, sensor.curLoad, sensor.curAtfTemp), readPercentualMap(mpcMap54, sensor.curLoad, sensor.curAtfTemp));
      }
      break;

    case 5: // -------------------------------------------------------------------------
      gear = 5;
      printf("[gearchangeDown] 5th gear\n");
      break;

    default: // -------------------------------------------------------------------------
      printf("[gearchangeDown] do nothing, default case\n");
      break;
    }
  }
  else
  {
    if (debugEnabled)
    {
      printf("[gearChangeDown->gearChangeDown] Blocking change shiftBlocker: %d  shiftPending: %d\n",
             shiftBlocker, shiftPending);
    }

    if (debugEnabled && sensor.curRPM >= config.highRPMshiftLimit)
    {
      printf("[gearChangeDown->gearChangeDown] high rpm limit hit with RPM: %d limit: %d\n",
             sensor.curRPM, config.highRPMshiftLimit);

      if (!shiftBlocker)
      {
        shiftPending = false;
      }
    }
  }
}

int evaluateGear()
{
  struct SensorVals sensor = readSensors();
  int incomingShaftSpeed = 0;
  int measuredGear = 0;

  if (n3Speed == 0)
  {
    incomingShaftSpeed = n2Speed * 1.64;
  }
  else
  {
    // when gear is 2, 3 or 4, n3 speed is not zero,
    // and then incoming shaft speed (=turbine speed) equals to n2 speed)
    incomingShaftSpeed = n2Speed;
  }

  if (n3Speed == 0 && sensor.curSpeed < 10)
  {
    // If we're near standstill and there is no N3 info, we can assume that we're on 1.
    measuredGear = 1;
  }
  else
  {
    ratio = (float)incomingShaftSpeed / vehicleSpeedRevs;
    measuredGear = gearFromRatio(ratio);
  }

  return measuredGear;
}

// ========================================================================
// Utilitarty functions
// ========================================================================

float ratioFromGear(int inputGear)
{
  float gearRatio;
  switch (inputGear)
  {
  case 1:
    gearRatio = 3.59;
    return gearRatio;
    break;

  case 2:
    gearRatio = 2.19;
    return gearRatio;
    break;

  case 3:
    gearRatio = 1.41;
    return gearRatio;
    break;

  case 4:
    gearRatio = 1.00;
    return gearRatio;
    break;

  case 5:
    gearRatio = 0.83;
    return gearRatio;
    break;

  default:
    gearRatio = 0.00;
    return gearRatio;
    break;
  }
}

int gearFromRatio(float inputRatio)
{
  if (inputRatio < 3.82 && inputRatio > 3.46)
  {
    int returnGear = 1;
    return returnGear;
  }
  else if (inputRatio < 2.62 && inputRatio > 2.26)
  {
    int returnGear = 2;
    return returnGear;
  }
  else if (inputRatio < 1.60 && inputRatio > 1.35)
  {
    int returnGear = 3;
    return returnGear;
  }
  else if (inputRatio < 1.20 && inputRatio > 0.92)
  {
    int returnGear = 4;
    return returnGear;
  }
  else if (inputRatio < 0.90 && inputRatio > 0.60)
  {
    int returnGear = 5;
    return returnGear;
  }
  else
  {
    int returnGear = 6;
    return returnGear;
  }
}

float getGearSlip()
{
  struct SensorVals sensor = readSensors();
  static float maxRatio[5] = {0.00, 0.00, 0.00, 0.00, 0.00}, minRatio[5] = {0.00, 0.00, 0.00, 0.00, 0.00};
  float slip;

  if (ratio > maxRatio[gear] && sensor.curRPM > config.stallSpeed)
  {
    maxRatio[gear] = ratio;
  }
  else if (ratio < minRatio[gear] && sensor.curRPM > config.stallSpeed)
  {
    minRatio[gear] = ratio;
  }

  slip = maxRatio[gear] - minRatio[gear];

  return slip;
}

// ==========================
//* UNUSED NOW
void faultMon(void *)
{
  struct SensorVals sensor = readSensors();

  if (sensor.curSlip > config.maxSlip && sensor.curRPM > config.stallSpeed)
  {
    slipFault = true;
  }
  else
  {
    slipFault = false;
  }

  if (sensor.curBattery < config.batteryLimit)
  {
    batteryFault = true;
  }
  else
  {
    batteryFault = false;
  }
}
// END OF CORE
