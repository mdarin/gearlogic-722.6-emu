

#include "pollstick.h"

// Polling for stick control
// This is W202 electronic gear stick, should work on any pre-canbus sticks.
void pollstick(void *)
{
#undef CANBUS
#ifdef CANBUS
    if (justStarted)
    {
        Can0.setBaudRate(500000);
        Can0.enableFIFO(1);
        Can0.enableFIFOInterrupt(1);
        Can0.onReceive(canSniff);
        Can0.intervalTimer(); // enable queue system and run callback in background.
        justStarted = false;
    }
#endif
#ifndef CANBUS
    if (!resistiveStick)
    {
        // Read the stick.
        int whiteState = stickStates[currentStateIndex].white;   // digitalRead(whitepin);
        int blueState = stickStates[currentStateIndex].blue;     // digitalRead(bluepin);
        int greenState = stickStates[currentStateIndex].green;   // digitalRead(greenpin);
        int yellowState = stickStates[currentStateIndex].yellow; // digitalRead(yellowpin);
        int autoState = HIGH;                                    // digitalRead(autoSwitch);
        garageShiftMove = true;

        // Determine position
        if (whiteState == HIGH && blueState == HIGH && greenState == HIGH && yellowState == LOW)
        {
            wantedGear = 8;
            gear = 2; // force reset gear to 2
            shiftPending = false;
            shiftBlocker = false;
            garageShiftMove = false;
            if (debugEnabled)
            {
                printf("[pollstick] pollstick: P parking \n");
            }

        } // P
        if (whiteState == LOW && blueState == HIGH && greenState == HIGH && yellowState == HIGH)
        {
            wantedGear = 7;
            gear = 2; // force reset gear to 2
            garageShiftMove = false;
            if (debugEnabled)
            {
                printf("[pollstick] pollstick: R retreat \n");
            }
        } // R
        if (whiteState == HIGH && blueState == LOW && greenState == HIGH && yellowState == HIGH)
        {
            wantedGear = 6;
            garageShiftMove = false;
            if (debugEnabled)
            {
                printf("[pollstick] pollstick: N neitral \n");
            }
        } // N
        if (whiteState == LOW && blueState == LOW && greenState == HIGH && yellowState == LOW)
        {
            wantedGear = 5;
            garageShiftMove = false; // these should not be necessary after wantedGear <5, but don't want to risk this keeping y5 alive for some reason.
            if (debugEnabled)
            {
                printf("[pollstick] pollstick: 5st(D) gear \n");
            }
        }
        if (whiteState == LOW && blueState == LOW && greenState == LOW && yellowState == HIGH)
        {
            wantedGear = 4;
            garageShiftMove = false;
            if (debugEnabled)
            {
                printf("[pollstick] pollstick: 4st gear \n");
            }
        }
        if (whiteState == LOW && blueState == HIGH && greenState == LOW && yellowState == LOW)
        {
            wantedGear = 3;
            garageShiftMove = false;
            if (debugEnabled)
            {
                printf("[pollstick] pollstick: 3st gear \n");
            }
        }
        if (whiteState == HIGH && blueState == LOW && greenState == LOW && yellowState == LOW)
        {
            wantedGear = 2;
            garageShiftMove = false;
            if (debugEnabled)
            {
                printf("[pollstick] pollstick: 2st gear \n");
            }
        }
        if (whiteState == HIGH && blueState == HIGH && greenState == LOW && yellowState == HIGH)
        {
            wantedGear = 1;
            garageShiftMove = false;
            if (debugEnabled)
            {
                printf("[pollstick] pollstick: 1st gear \n");
            }
        }

        printf("[Pollstick] table postick: %s\n", stickStates[currentStateIndex].positionName);

        if (autoState == HIGH)
        {
            if (!stickCtrl)
            {
                if (debugEnabled)
                {
                    printf("[pollstick] pollstick: stickCtrl on \n");
                }
                stickCtrl = true;
                fullAuto = true;
            }
        }
        else
        {
            if (stickCtrl)
            {
                if (debugEnabled)
                {
                    printf("[pollstick] pollstick: stickCtrl off \n");
                }
                stickCtrl = false;
                fullAuto = false;
            }
        }
    }
    else // resistive stick // * UNSED NOW
    {
        int blueState = analogRead(bluepin);

        if (blueState > 450 && blueState < 750)
        {
            wantedGear = 8;
            gear = 2; // force reset gear to 2
            shiftPending = false;
            shiftBlocker = false;
            garageShiftMove = false;
        }
        if (blueState > 300 && blueState < 400)
        {
            wantedGear = 7;
            gear = 2; // force reset gear to 2
            garageShiftMove = false;
        }
        if (blueState > 200 && blueState < 300)
        {
            wantedGear = 6;
            garageShiftMove = false;
        }
        if (blueState > 100 && blueState < 200)
        {
            wantedGear = 5;
            garageShiftMove = false;
        }
    }
#endif // CANBUS
}

// For manual microswitch control, gear up
void gearUp()
{
    if (wantedGear < 6 && !fullAuto && gear < 5)
    {
        //* Do nothing if we're on N/R/P
        if (!shiftBlocker)
        {
            stickCtrl = false;
            newGear = gear;
            newGear++;
            shiftPending = true;

            gearchangeUp(newGear);
        }

        if (debugEnabled)
        {
            printf("[pollstick] gearup: Gear up requested \n");
        }
    }
}

// For manual microswitch control, gear down
void gearDown()
{
    if (wantedGear < 6 && !fullAuto && gear > 1)
    {
        //* Do nothing if we're on N/R/P
        if (!shiftBlocker)
        {
            stickCtrl = false;
            newGear = gear;
            newGear--;
            shiftPending = true;

            gearchangeDown(newGear);
        }

        if (debugEnabled)
        {
            printf("[pollstick] geardown: Gear down requested \n");
        }
    }
}

// Polling for manual switch keys
void pollkeys()
{
    int gupState = 0;
    int gdownState = 0;

    if (!resistiveStick)
    {
        gupState = digitalRead(gupSwitch);     // Gear up
        gdownState = digitalRead(gdownSwitch); // Gear down

        if (gdownState == LOW && gupState == HIGH)
        {
            if (debugEnabled)
            {
                printf("[pollstick] pollkeys: Gear up button \n");
            }

            gearUp();
        }
        else if (gupState == LOW && gdownState == HIGH)
        {
            if (debugEnabled)
            {
                printf("[pollstick] pollkeys: Gear down button \n");
            }

            gearDown();
        }
    }
    else // resistive stick
    {

        gupState = analogRead(gupSwitchalt);  // Gear up
        gdownState = analogRead(gdownSwitch); // Gear down

        if (gupState < 20)
        {
            if (debugEnabled)
            {
                printf("[pollstick] pollkeys: Gear up button \n");
            }

            gearUp();
        }
        if (gdownState < 100)
        {
            if (debugEnabled)
            {
                printf("[pollstick] pollkeys: Gear down button \n");
            }

            gearDown();
        }
        /* Serial.print(gdownState);
      Serial.print("-");
      Serial.println(gupState);*/
    }
}
