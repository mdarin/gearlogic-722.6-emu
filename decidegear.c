#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>
#include "params.h"
#include "sens.h"
#include "core.h"
#include "mock_aruduino.h"

// Logic for automatic new gear, this makes possible auto up/downshifts.
void decideGear(void *)
{
  int moreGear = gear + 1;
  int lessGear = gear - 1;
  struct SensorVals sensor = readSensors();

  // Determine speed related downshift and upshift here.
  int autoGear = 0; // todo readMap(gearMap, sensor.curTps, sensor.curSpeed);

  if (stickCtrl && !fullAuto && wantedGear < 6)
  {
    if (wantedGear > gear)
    {
      newGear = moreGear;
      shiftPending = true;
      gearchangeUp(newGear);
    }
    else if (wantedGear < gear)
    {
      newGear = lessGear;
      shiftPending = true;
      gearchangeDown(newGear);
    }
  }
  if (!shiftBlocker && !shiftPending && !speedFault && wantedGear < 6 && millis() - lastShiftPoint > config.nextShiftDelay)
  {
    if (autoGear > gear && fullAuto && sensor.curSpeed > 10)
    {
      int newGear = moreGear;

      if (debugEnabled)
      {
        printf("[decideGear->gearchangeUp] tpsPercent-vehicleSpeed: %d %d\n", sensor.curTps, sensor.curSpeed);
      }

      if (debugEnabled)
      {
        printf("[decideGear->gearchangeUp] wantedGear-autoGear-newGear-gear: %d %d %d %d\n", wantedGear, autoGear, newGear, gear);
      }
      if (evalGear)
      {
        int evaluatedGear = evaluateGear();

        if (evaluatedGear == gear)
        {
          shiftPending = true;
          gearchangeUp(newGear);
        }
        else
        {
          if (debugEnabled)
          {
            printf("Blocking shift, evaluatedGear != gear");
          }
        }
      }
      else
      {
        shiftPending = true;
        gearchangeUp(newGear);
      }
    }

    if (autoGear < gear && fullAuto)
    {
      int newGear = lessGear;

      if (debugEnabled)
      {
        printf("[decideGear->gearchangeDown] tpsPercent-vehicleSpeed: %d %d\n", sensor.curTps, sensor.curSpeed);
      }
      if (debugEnabled)
      {
        printf("[decideGear->gearchangeDown] wantedGear-autoGear-newGear-gear: %d %d %d %d\n", wantedGear, autoGear, newGear, gear);
      }
      if (evalGear)
      {
        int evaluatedGear = evaluateGear();
        if (evaluatedGear == gear)
        {
          shiftPending = true;
          gearchangeDown(newGear);
        }
        else
        {
          if (debugEnabled)
          {
            printf("Blocking shift, evaluatedGear != gear\n");
          }
        }
      }
      else
      {
        shiftPending = true;
        gearchangeDown(newGear);
      }
    }
  }
}
