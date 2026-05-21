#include "sens.h"

struct SensorVals readSensors()
{
    struct SensorVals sensor;

    //* we need to calculate these in precise moment to get accurate reading,
    //* so this acts just an interface for global vars.

    sensor.curSlip = gearSlip;
    sensor.curRatio = ratio;
    sensor.curEvalGear = evalGearVal;
    sensor.fuelUsed = fuelUsed;
    sensor.fuelUsedAvg = fuelUsedAvg;
    sensor.curOilTemp = 110; // oilRead();
    sensor.curAtfTemp = 110; // atfRead();
    sensor.curExTemp = exhaustTempRead();
    sensor.curBoost = 1; // boostRead();
    sensor.curExPres = exhaustPressureRead();
    sensor.curPresDiff = 0; // float(sensor.curExPres) / sensor.curBoost;
    sensor.curBoostLim = boostLimitRead(sensor.curOilTemp);
    sensor.curTps = (int)(my_engine.gas_pedal * (double)(100)); // tpsRead();
    sensor.curRPM = (int)(my_engine.current_rpm);               // rpmRead();
    sensor.curSpeed = (int)(my_engine.speed_kmh);               // speedRead();
    sensor.curLoad = loadRead(sensor.curTps, sensor.curBoost, sensor.curBoostLim, sensor.curRPM);
    sensor.curBattery = 14000; // batteryRead();

    // my_engine.gas_pedal * 100;
    // my_engine.speed_kmh;
    // my_engine.current_rpm;
    // my_engine.load * 100;

    return sensor;
}

int speedRead()
{
    int curRPM = rpmRead();
    int vehicleSpeedRPM = 0, vehicleSpeedDiff = 0, speedValue = 0;

    // int vehicleSpeed = 0.03654 * vehicleSpeedRevs; // 225/45/17 with 3.27 rear diff
    float tireDiameter = (config.tireWidth * config.tireProfile / 2540 * 2 + config.tireInches) * 25.4;
    float tireCircumference = 3.14 * tireDiameter;
    if (rpmSpeed)
    {
        // speed based on engine rpm
        vehicleSpeedRPM = tireCircumference * curRPM / (ratioFromGear(gear) * config.diffRatio) / 1000000 * 60;
        speedValue = vehicleSpeedRPM;
    }
    if (diffSpeed)
    {
        // speed based on diff abs sensor
        vehicleSpeedDiff = tireCircumference * vehicleSpeedRevs / config.diffRatio / 1000000 * 60;
        speedValue = vehicleSpeedDiff;
        vehicleTravelDiff = tireCircumference * vehicleTravelRevs / 1000000;
    }
    if (rpmSpeed && diffSpeed)
    {
        if (vehicleSpeedRPM / (float)vehicleSpeedDiff > 1.3 || vehicleSpeedRPM / (float)vehicleSpeedDiff < 0.7)
        {
            if (!speedFault)
            {
                speedFault = true; // if both sensors are enabled and difference is too great, then create a fault.
                if (debugEnabled)
                {
                    printf("SPEED FAULT: detected - autoshift disabled %d - %d\n", vehicleSpeedDiff, vehicleSpeedRPM);
                }
            }
        }
        else
        {
            if (speedFault)
            {
                speedFault = false; // we're in sync, good to go
                if (debugEnabled)
                {
                    printf("SPEED FAULT: recovery");
                }
            }
        }
    }
    return speedValue;
    // return vehicleSpeedRevs;
}

int tpsRead()
{
    int tpsPercentValue = 0;
    if (tpsSensor)
    {
        float refRead = analogRead(refPin);
        float refTps = refRead / 1023 * 3.3;
        float tpsVoltage = analogRead(tpsPin);
        tpsPercentValue = readTPSVoltage(tpsVoltage);

        tpsPercentValue = config.tpsAgre * tpsPercentValue;

        if (tpsPercentValue > 100)
        {
            tpsPercentValue = 100;
        }
        if (tpsPercentValue < 0)
        {
            tpsPercentValue = 0;
        }
    }
    else
    {
        tpsPercentValue = 100;
    }

    return tpsPercentValue;
}

// void tpsInit(int action)
// {
//     switch (action)
//     {
//     case 0:
//     {
//         int curValue = EEPROM.read(100);
//         float refRead = analogRead(refPin);
//         int tpsVoltage = analogRead(tpsPin);
//         // if (curValue != tpsVoltage)
//         //{
//         byte lowByte = ((tpsVoltage >> 0) & 0xFF);
//         byte highByte = ((tpsVoltage >> 8) & 0xFF);
//         EEPROM.write(1000, lowByte);
//         EEPROM.write(1100, highByte);
//         Serial.print("Written voltage val 1000:");
//         Serial.println(tpsVoltage);
//         //}
//         break;
//     }
//     case 1:
//     {
//         int curValue = EEPROM.read(200);
//         float refRead = analogRead(refPin);
//         int tpsVoltage = analogRead(tpsPin);
//         // if (curValue != tpsVoltage)
//         //{
//         byte lowByte = ((tpsVoltage >> 0) & 0xFF);
//         byte highByte = ((tpsVoltage >> 8) & 0xFF);
//         EEPROM.write(2000, lowByte);
//         EEPROM.write(2100, highByte);
//         Serial.print("Written voltage val 2000:");
//         Serial.println(tpsVoltage);
//         //  }
//         break;
//     }
//     default:
//         break;
//     }
// }

int rpmRead()
{

    if (rpmRevs > config.maxRPM)
    {
        rpmRevs = config.maxRPM;
    }
    // Sensor smoothing if needed.
    /*avgRpmValue = alpha*oldRpmValue + (1-alpha)*rpmRevs;
      oldRpmValue = avgRpmValue;*/
    return rpmRevs;
}

int oilRead()
{
    // wip
    // w124 temp sensor B = 3500 roughly, 2.0kohm at 25c, voltage divider resistor is 2250ohm in 12V (dropped Vmax to 3V respectively)
    /*
      Steinhart-Hart coefficients
      a[0] = 1.689126553357672e-03
      a[1] = 8.951863613981253e-05
      a[2] = 2.411208545519697e-05
      a[3] = -9.456539654701360e-07 <- this can be c4
    */
    /* OLD
      float c1 = 1.689126553357672e-03, c2 = 8.951863613981253e-05, c3 = 2.411208545519697e-05;
      float tempRead = analogRead(oilPin);
      avgTemp = (avgTemp * 5 + tempRead) / 10;
      int R2 = 2250 * (1023.0 / (float)avgTemp - 1.0);
      float logR2 = log(R2);
      float T = (1.0 / (c1 + c2 * logR2 + c3 * logR2 * logR2 * logR2));
      // float T = (1.0 / (c1 + c2 * logR2 + c3 * logR2 * logR2 * logR2 + c4 * logR2 * logR2 * logR2));
      float oilTemp = T - 273.15;
      // oilTemp = 100;
      return oilTemp;
    */
    // float c1 = 1.689126553357672e-03, c2 = 8.951863613981253e-05, c3 = 2.411208545519697e-05;
    static float oilTemp;
    /* if (!shiftBlocker)
     {
       float c1 = 1.268318203e-03, c2 = 2.662206632e-04, c3 = 1.217978476e-07;
       float refRead = analogRead(refPin);
       float tempRead = analogRead(oilPin);
       tempRead = analogRead(oilPin);
       float refTemp = refRead / 1023 * 3.3;
       filterOneLowpass2.input(tempRead);
       //avgOilTemp = (avgOilTemp * 9 + tempRead) / 10;
       //avgOilRef = (avgOilRef * 9 + refRead) / 10;
       int R2 = 4700 / (refRead / (float)filterOneLowpass2.output() - 1.0);
       float logR2 = log(R2);
       float T = (1.0 / (c1 + c2 * logR2 + c3 * logR2 * logR2 * logR2));
       // float T = (1.0 / (c1 + c2 * logR2 + c3 * logR2 * logR2 * logR2 + c4 * logR2 * logR2 * logR2));
       oilTemp = T - 273.15;
     }
     return oilTemp;
     /* if (wantedGear == 6 || wantedGear == 8)
       {
       avgOilTemp = (avgOilTemp * 5 + oilTemp) / 10 +30;
       }
       else {
       avgOilTemp = (avgOilTemp * 5 + oilTemp) / 10 +30;
       }*/
    //   int R2 = 0;
    if (!shiftBlocker)
    {
        float refRead = analogRead(refPin);
        float tempRead = analogRead(oilPin);
        // todo filterOneLowpass2.input(tempRead);
        float refVoltage = (refRead + 30) * 5.0 / 1024;
        float ref3V3 = (refRead + 30) * 3.3 / 1024;
        float buffer = tempRead * refVoltage;
        //  Serial.print("buffer: ");
        //  Serial.println(buffer);
        float outVoltage = (buffer) / (refRead + 30);
        buffer = (refVoltage / outVoltage) - 1;

        int R2 = 4700 * (1 / (((ref3V3) / (outVoltage)) - 1)) - 70;
        // todo oilTemp = readTempMapInverted(oilSensorMap, R2);
    }
    return oilTemp;

    /*
      float tempRead = analogRead(oilPin);
      float refRead = analogRead(refPin);
      float refTemp = refRead / 1023 * 3.0;
      filterOneLowpass2.input(tempRead);

      // int R2 = (4700 / (1023 / (float)filterOneLowpass2.output() - 1.0)) + 2050;
      int R2 = (4700 / (1023 / tempRead - 1.0)) + 2500;
      int oilTemp = readTempMapInverted(oilSensorMap, R2);



      return oilTemp;*/
}

int boostRead()
{
    int boostValue = 0;
    // float boostVoltage = 0;
    // if (boostSensor)
    // {
    //     // reading MAP/boost
    //     int refRead = analogRead(refPin);
    //     boostVoltage = (analogRead(boostPin) - boostSensorOffset) * (3.3 / refRead);
    //     boostValue = boostVoltage * 700 / 2.95;
    //     boostSensorFilter.input(boostValue);
    //     boostValue = boostSensorFilter.output();
    // }
    // if (boostValue < 0)
    // {
    //     boostValue = 0;
    // }

    return boostValue;
}

int exhaustPressureRead()
{
    int exhaustPresVal = 0;
    float exhaustPresVol = 0;
    if (exhaustPresSensor)
    {
        // reading exhaust pressure
        int refRead = analogRead(refPin);
        // exhaustPresVol = (analogRead(exhaustPresPin) - exhaustSensorOffset) * (3.3 / refRead);
        // exhaustPresVal = exhaustPresVol * 700 / 2.95;
        // exhaustPressureFilter.input(exhaustPresVal);
        // exhaustPresVal = exhaustPressureFilter.output();
    }
    if (exhaustPresVal < 0)
    {
        exhaustPresVal = 0;
    }

    return exhaustPresVal;
}

int batteryRead()
{
    int batteryMonVal = 0;
    if (batteryMonitor)
    {
        // reading battery voltage
        float batteryMonVol = analogRead(batteryPin) * 3.3;
        batteryMonVal = readBatVoltage(batteryMonVol);
    }
    return batteryMonVal;
}

int boostLimitRead(int oilTemp)
{
    //  int allowedBoostPressure = readGearMap(boostControlPressureMap, gear, oilTemp);
    //  return allowedBoostPressure;
    return boostOverride;
}

int loadRead(int curTps, int curBoost, int curBoostLim, int curRPM)
{
    unsigned int trueLoad = 0;
    int boostPercent = 0;

    if (curBoostLim == 0)
    {
        boostPercent = 100 * curBoost / config.boostSpring;
    }
    else
    {
        boostPercent = 100 * curBoost / curBoostLim;
    }

    int vehicleRPM = 100 * curRPM / config.maxRPM;

    if (boostSensor && tpsSensor && !rpmSpeed)
    {
        trueLoad = (curTps * 0.60) + (boostPercent * 0.40);
    }
    else if (boostSensor && tpsSensor && rpmSpeed)
    {
        trueLoad = (curTps * 0.60) + (boostPercent * 0.20) + (vehicleRPM * 0.20);
    }
    else if (tpsSensor && !boostSensor)
    {
        trueLoad = curTps;
    }
    else if (!tpsSensor || trueLoad >= 100)
    {
        trueLoad = 100;
    }
    trueLoad = trueLoad + 30;

    // trueLoad = trueLoad + 50;
    if (trueLoad > 100)
    {
        trueLoad = 100;
    }
    return trueLoad;
}

// reading oil temp sensor / pn-switch (same input pin, see page 27: http://www.all-trans.by/assets/site/files/mercedes/722.6.1.pdf)
int atfRead()
{
    float refRead = analogRead(refPin);
    float tempRead = analogRead(atfPin);
    // todo filterOneLowpass.input(tempRead);
    float refVoltage = (refRead + 30) * 5.0 / 1024;
    float ref3V3 = (refRead + 30) * 3.3 / 1024;
    float buffer = tempRead * refVoltage;
    //  Serial.print("buffer: ");
    //  Serial.println(buffer);
    float outVoltage = (buffer) / (refRead + 30);
    buffer = (refVoltage / outVoltage) - 1;

    int R2 = 2740 * (1 / (((ref3V3) / (outVoltage)) - 1)) - 150;
    /* Serial.print("refRead: ");
     Serial.println(refRead);
     Serial.print("tempRead: ");
     Serial.println(tempRead);
     Serial.print("refVoltage: ");
     Serial.println(refVoltage);
     Serial.print("buffer: ");
     Serial.println(buffer);
     Serial.print("outVoltage: ");
     Serial.println(outVoltage);
     Serial.print("R2: ");
     Serial.println(R2);
       Serial.print("ref3V3: ");
     Serial.println(ref3V3);*/
    if (R2 < 564)
    {
        R2 = 564;
    }

    if (R2 > 2479)
    {
        R2 = 2479;
    }

    int atfTemp = 0; // todo readTempMap(atfSensorMap, R2);

    if (wantedGear == 6 || wantedGear == 8)
    {
        atfTemp = oilRead();
    }
    atfTemp = atfTemp;
    return atfTemp;
}

int exhaustTempRead()
{
    static double exhaustTemp = 0;
    // if (exhaustTempSensor)
    // {
    //     // todo exhaustTemp = kTC.readCelsius();
    // }
    return exhaustTemp;
}

int adaptSPC(int mapId, int xVal, int yVal)
{
    int current = 0;
    // #ifdef ASPC
    //     int modVal = 5;
    //     int aSpcUpState = digitalRead(aSpcUpSwitch);     // Adapt pressure up
    //     int aSpcDownState = digitalRead(aSpcDownSwitch); // Adapt pressure down
    //     static int prevaSpcUpState = 0;
    //     static int prevaSpcDownState = 0;
    //     int current = readEEPROM(mapId, xVal, yVal);

    //     if (aSpcDownState != prevaSpcDownState || aSpcUpState != prevaSpcUpState)
    //     {
    //         if (aSpcDownState == LOW && aSpcUpState == HIGH)
    //         {
    //             prevaSpcUpState = aSpcUpState;

    //             if (debugEnabled)
    //             {
    //                 Serial.println(F("adaptSPC: More pressure button"));
    //                 Serial.print(F("adaptSPC: request values: "));
    //                 Serial.print(mapId);
    //                 Serial.print(F("-"));
    //                 Serial.print(xVal);
    //                 Serial.print(F("-"));
    //                 Serial.println(yVal);
    //                 Serial.print(F("adaptSPC: old adapt pressure is: "));
    //                 Serial.println(current);
    //             }
    //             current = current + modVal;
    //             writeEEPROM(mapId, xVal, yVal, current);

    //             if (debugEnabled)
    //             {
    //                 Serial.print(F("adaptSPC: New adapt pressure is: "));
    //                 Serial.println(current);
    //             }
    //         }
    //         else if (aSpcUpState == LOW && aSpcDownState == HIGH)
    //         {
    //             prevaSpcDownState = aSpcDownState;

    //             if (debugEnabled)
    //             {
    //                 Serial.println(F("adaptSPC: Less pressure button"));
    //                 Serial.print(F("adaptSPC: request values: "));
    //                 Serial.print(mapId);
    //                 Serial.print(F("-"));
    //                 Serial.print(xVal);
    //                 Serial.print(F("-"));
    //                 Serial.println(yVal);
    //                 Serial.print(F("adaptSPC: old adapt pressure is: "));
    //                 Serial.println(current);
    //             }
    //             current = current - modVal;
    //             writeEEPROM(mapId, xVal, yVal, current);

    //             if (debugEnabled)
    //             {
    //                 Serial.print(F("adaptSPC: New adapt pressure is: "));
    //                 Serial.println(current);
    //             }
    //         }
    //     }
    // #endif
    return current;
}
