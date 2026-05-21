#include "polltrans.h"

// Polling time for transmission control
// R/N/P modulation pressure regulation
// idle SPC regulation
// Boost control
void polltrans(void *)
{
    struct SensorVals sensor = readSensors();
    unsigned int shiftDelay = 2000;

    if (shiftBlocker)
    {
        if (tpsSensor)
        {
            shiftDelay = readPercentualMap(shiftTimeMap, spcPercentVal, sensor.curAtfTemp);
        }
        else
        {
            shiftDelay = 1200;
        }
        shiftDuration = millis() - shiftStartTime;
        if (shiftDuration > shiftDelay && shiftDone)
        {
            if (debugEnabled)
            {
                printf("[polltrans->switchGearStop] shiftDelay-spcPercentVal-atfTemp %u - %d - %d\n",
                       shiftDelay, spcPercentVal, atfRead());
            }
            switchGearStop();
        }
        if (preShift && !preShiftDone)
        {
            doPreShift();
        }
        else if (!preShift && preShiftDone)
        {
            doShift();
        }
        else if (postShift && !postShiftDone)
        {
            doPostShift();
        }
    }

    // Raw value for pwm control (0-255) for SPC solenoid, see page 9: http://www.all-trans.by/assets/site/files/mercedes/722.6.1.pdf
    //  "Pulsed constantly while idling in Park or Neutral at approximately 40% Duty cycle" <- 102/255 = 0.4
    //  MPC = varying with load, SPC constant 33%
    // int mpcVal = readMap(mpcNormalMap, sensor.curLoad, sensor.curAtfTemp);

    if (!shiftBlocker)
    {
        // Pulsed constantly while idling in Park or Neutral at approximately 33% Duty cycle.
        if (wantedGear == 6 || wantedGear == 8)
        {
            // todo analogWrite(spc, 20);
            garageShift = true;
            garageTime = millis();
        }
        // Pulsed constantly while idling in Park or Neutral at approximately 40% Duty cycle, also for normal mpc operation
        if (wantedGear == 8 || wantedGear == 6 || (wantedGear <= 6 && !shiftPending && !shiftBlocker && (millis() - lastShiftPoint) > 5000))
        {
            // int mpcSetVal = (100 - mpcVal) * 2.55;
            int mpcSetVal = 102;
            //  analogWrite(mpc, mpcSetVal);
        }

        if ((wantedGear == 7 || (wantedGear < 6 && !shiftPending)) && garageShift && (millis() - garageTime > 1000))
        {
            // todo analogWrite(spc, 0);
            garageShift = false;
        }

        // 3-4 Shift solenoid is pulsed continuously while in Park and during selector lever movement (Garage Shifts).
        // Testing whether we actually need this.
        if (wantedGear > 5 && garageShiftMove && stickCtrl)
        {
            // todo analogWrite(y5, 255);
            // delay(500);
        }
        if (!garageShiftMove)
        {
            // todo analogWrite(y5, 0);
        }

        if (tccLock)
        {
            // Enable torque converter lock when tps is less than 40%, current speed is more than 80km/h and gear is within allowed range.
            if (sensor.curTps < 40 && sensor.curSpeed > 30 && gear >= config.firstTccGear && gear > 1 && sensor.curRPM < 2500)
            {
                if (lockVal <= 255)
                {
                    lockVal = lockVal + 85;
                    // todo analogWrite(tcc, lockVal);
                    printf("lockVal: %d\n");
                }
                else
                {
                    // todo analogWrite(tcc, 255);
                    printf("lockVal: 255\n");
                }
            }
            else
            {
                if (lockVal >= 85)
                {
                    lockVal = lockVal - 85;
                    // todo analogWrite(tcc, lockVal);
                    printf("lockVal: %d\n");
                }
                else
                {
                    // todo analogWrite(tcc, 0);
                    printf("lockVal: 255\n");
                }
            }
        }
        // "1-2/4-5 Solenoid is pulsed during ignition crank." stop doing this after we get ourselves together.
        if (ignition)
        {
            // todo analogWrite(y3, 0);
            ignition = false;
        }
        if (evalGear && !shiftBlocker && millis() - lastShiftPoint > 5000 & wrongGearPoint < 5 & !shiftConfirmed)
        {
            int evaluatedGear = evaluateGear();
            if (millis() - lastShiftPoint > 5100)
            {
                if (evaluatedGear != gear)
                {
                    wrongGearPoint++;
                }
            }
            if (millis() - lastShiftPoint > 5500)
            {
                if (evaluatedGear != gear)
                {
                    wrongGearPoint++;
                }
            }
            if (millis() - lastShiftPoint > 6000)
            {
                if (evaluatedGear != gear)
                {
                    wrongGearPoint++;
                }
            }
            if (millis() - lastShiftPoint > 6500)
            {
                if (evaluatedGear != gear)
                {
                    wrongGearPoint++;
                }
            }
            if (millis() - lastShiftPoint > 7000)
            {
                if (evaluatedGear != gear)
                {
                    wrongGearPoint++;
                }
                if (wrongGearPoint < 3)
                {
                    shiftConfirmed = true;
                }
            }
        }
        if (wrongGearPoint >= 3)
        {
            int evaluatedGear = evaluateGear();
            if (evaluatedGear < 6 && wantedGear < 6)
            {
                gear = evaluateGear();
                wrongGearPoint = 0;
            }
        }
        if (evalGear & sensor.curSpeed < 10)
        {
            // gear = evaluateGear(); // ?
        }
    }

    // if (radioEnabled)
    // {
    //     radioControl();
    // }
    if (manual)
    {
        pollkeys();
    }
    // if (horn && (millis() - hornPressTime > 300))
    // {
    //     hornOff();
    // }
    if (sensor.curRPM > 0)
    {
        carRunning = true;
    }
    else
    {
        carRunning = false;
    }
}
