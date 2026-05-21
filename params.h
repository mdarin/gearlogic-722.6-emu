#ifndef PARAMS_H
#define PARAMS_H

#include <stdint.h>
#include <stdbool.h>

#ifndef HIGH
#define HIGH 1
#endif // HIGH
#ifndef LOW
#define LOW 0
#endif // LOW

#define digitalRead(A) HIGH
#define analogRead(A) 100

#define y3 36           // FMT3, orange<->brown/red // DOUT3, I need to use 16 as testing instead of 36.
#define y4 35           // orange <-> brown/grey // DOUT2
#define y5 8            // ex 14, orange <-> brown/black // DOUT1
#define mpc 38          // red <-> brown/pink // DOUT5 // FMT3
#define spc 37          // red <-> brown/yellow // DOUT4 // FMT3
#define tcc 29          // pink <-> brown/yellow/white // DOUT6
#define speedoCtrl 7    // ex 7, blue <-> blue/green // DOUT7,
#define rpmMeter 30     // FMT2 missing // DOUT10
#define boostCtrl 6     // FMT1, green <-> green/white/yellow // DOUT8, 3?
#define fuelPumpCtrl 12 // missing // DOUT9, 13?
#define hornPin 2       // Horn
// END OUTPUT PINS
#define injectionPin 36 // should be 16.

// INPUT PINS
// Stick input
#define whitepin 27  // 0.5kohm <-> yellow <-> grey-yellow-grey // DIN2 <-> blue
#define bluepin 34   // 0.5kohm <-> yellow <-> grey-green-grey // DIN4 <-> green
#define greenpin 26  // 0.5kohm <-> yellow <-> grey-white-grey // DIN1 <-> whiteblue
#define yellowpin 28 // 0.5kohm <-> yellow <-> grey-black-grey // DIN3 <-> whiteorange

// Switches
#define autoSwitch 33 // ex. 22, 0.5kohm <-> yellow <-> grey-pink-grey // DIN5

#define gdownSwitch 23 // ex. 23 <-> NC // DIN6
#define gupSwitch 24   // <-> NC // DIN7
#define gupSwitchalt 31
#define fuelInPin 32

#define exhaustPresPin A12 // A12, A9 used in coupe.
#define exhaustTempPin 12

// Car sensor input pins, black
#define tpsPin A10     // voltage div 5/3 <-> black <-> blue-black-blue = 1kohm/1.8kohm div // ANAIN3, boost? A3->A11
#define atfPin A1      // voltage div 5/3 <-> black <-> pink = 1kohm/1.8kohm div // ANAIN2
#define boostPin A11   // voltage div 5/3 <-> black <-> blue-brown-blue = 1kohm/1.8kohm div // ANAIN4, tps? A2->A10
#define oilPin A0      // voltage div 12/3 <-> black <-> white-pink-white = 1kohm/380ohm div // ANAIN1
#define n2pin 18       // voltage div 5/3 <-> black <-> whiteredwhite = 1kohm/1.8kohm div // DIN14
#define n3pin 19       // voltage div 5/3 <-> black <-> brownredwhite = 1kohm/1.8kohm div // DIN15
#define speedPin 20    // voltage div 12/3 <-> black <-> blueyellowblue = 1kohm/380ohm div // DIN10
#define rpmPin 25      // voltage div 12/3 <-> black <-> whitebluewhite = 1kohm/380ohm div // DIN12
#define batteryPin A21 // car battery monitor
#define refPin A22     // sensor voltage ref ?
// #define refPin A13 // using this in exhaustTempPin for now.

#define exhaustTemperatureCS 9
#define displayCS 10

// Macro for sizeof for better support with 2d arrays.
#define LEN(arr) ((int)(sizeof(arr) / sizeof(arr)[0]))

struct SensorVals
{
    int curSpeed, curOilTemp, curExTemp, curBoost, curExPres, curAtfTemp, curRPM, curBoostLim, curEvalGear, curBattery, fuelUsed, fuelUsedAvg, curLambda;
    uint8_t curTps, curLoad;
    float curSlip, curRatio, curPresDiff;
};

struct ConfigParam
{
    int boostMax, boostDrop, boostSpring, fuelMaxRPM, maxRPM, tireWidth, tireProfile, tireInches, rearDiffTeeth, nextShiftDelay, stallSpeed, batteryLimit, firstTccGear, triggerWheelTeeth, tpsAgre, highRPMshiftLimit, lowRPMshiftLimit;
    float diffRatio, maxSlip, transSloppy;
    int oneTotwo, twoTothree, threeTofour, fourTofive, fiveTofour, fourTothree, threeTotwo, twoToone;
};

// Структура для хранения состояния стика
struct StickState
{
    int white;
    int blue;
    int green;
    int yellow;
    int wantedGear;
    const char *positionName; // для отладки
};

extern bool debugEnabled;

extern bool justStarted;
extern bool resistiveStick;
extern bool garageShift;
extern bool garageShiftMove; // ?
extern bool stickCtrl;       // must be configurable
extern bool tccLock;
extern bool manual;
extern bool carRunning;
extern bool ignition;
extern double garageTime;
extern int lockVal;

extern uint8_t wantedGear; // default unreacheble gear
extern uint8_t gear;
extern uint8_t newGear;

// Obvious internals
extern uint8_t prevGear; // Start on gear 2
extern uint8_t pendingGear;

// float ratio;
// Shift pressure defaults
extern int spcPercentVal;
extern int mpcPercentVal;

// for timers
extern unsigned long int shiftStartTime, shiftStopTime, delaySinceLast;
extern unsigned long int shiftDuration;

// Solenoid used
extern int cSolenoidEnabled;
extern int cSolenoid; // Change solenoid pin to be controlled.
extern int lastMapVal;
extern int shiftLoad;
extern int shiftAtfTemp;
extern int wrongGearPoint;
extern bool shiftConfirmed, preShift, postShift, preShiftDone, shiftDone, postShiftDone;
extern double lastShiftPoint;

extern bool shiftBlocker;
extern bool shiftPending;

extern bool fullAuto;

extern bool tpsSensor;

extern bool speedFault;
extern bool batteryFault;
extern bool slipFault;

extern bool boostLimit;
extern bool boostLimitShift;

extern struct ConfigParam config;

extern float gearSlip;
extern float ratio;

extern int avgAtfTemp;
extern int avgBoostValue;
extern int avgExhaustPresVal;
extern int avgExTemp;
extern int avgVehicleSpeedDiff;
extern int avgVehicleSpeedRPM;
extern int avgRpmValue;
extern int oldRpmValue;
extern int avgOilTemp;
extern int evalGearVal;
extern int avgAtfRef;
extern int avgOilRef;

extern unsigned long n2SpeedPulses;
extern unsigned long n3SpeedPulses;
extern unsigned long vehicleSpeedPulses;
extern unsigned long lastSensorTime;
extern unsigned long rpmPulse;
extern unsigned long curLog;
extern unsigned long lastLog;
extern unsigned long fuelIn;
extern unsigned long fuelOut;
extern unsigned long fuelUsed;
extern unsigned long fuelUsedAvg;
extern unsigned long vehicleTravelRevs;
extern unsigned long vehicleTravelDiff;

extern int n2Speed;
extern int n3Speed;
extern int rpmRevs;
extern int vehicleSpeedRevs;

extern bool trans;
extern bool shiftBlocker;
extern bool shiftPending;

extern bool exhaustPresSensor;
extern bool batteryMonitor;
extern bool boostSensor;
extern bool exhaustTempSensor;
extern bool rpmSpeed;
extern bool diffSpeed;
extern bool adaptive;

extern int boostOverride;

extern double lastShiftPoint;

extern double lastShiftPoint;
extern int lastXval;
extern int lastYval;
extern bool ShiftDebugEnabled;
extern int initBVoltage;
extern int initEVoltage;

extern bool evalGear;

extern bool boostLimit;
extern bool boostLimitShift;

// Stick and keys
extern const struct StickState stickStates[];
extern int currentStateIndex;
extern const int stickStatesCount;

#endif // PARAMS_H
