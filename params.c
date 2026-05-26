#include "params.h"
#include <stdint.h>
#include <stdbool.h>

// ----[ CONFIG ] ----
bool debugEnabled = true;
bool ShiftDebugEnabled = true;

bool justStarted = true;
bool resistiveStick = false;
bool garageShift = true;
bool garageShiftMove = false; // ?
bool stickCtrl = true;        // must be configurable

uint8_t wantedGear = 100; // default unreacheble gear // todo use constant
uint8_t gear;
uint8_t newGear = 2;

// Obvious internals
uint8_t prevGear = 2; //? Start on gear 2
uint8_t pendingGear = 2;

// Shift pressure defaults
int spcPercentVal = 100;
int mpcPercentVal = 100;

// for timers
unsigned long int shiftStartTime, shiftStopTime, delaySinceLast = 0;
unsigned long int shiftDuration = 0;

// Solenoid used
int cSolenoidEnabled = 0;
int cSolenoid = 0; // Change solenoid pin to be controlled.
int lastMapVal;
int shiftLoad = 0;
int shiftAtfTemp = 0;
int wrongGearPoint = 0;
bool shiftConfirmed, preShift, postShift, preShiftDone, shiftDone, postShiftDone = false;
double lastShiftPoint;

bool fullAuto = false;

bool tpsSensor = true;

// fault mode for speed sensors
bool speedFault = false;
// fault mode for battery fault
bool batteryFault = false;
// fault mode for excess slip
bool slipFault = false;

float gearSlip;
float ratio;

int avgAtfTemp;
int avgBoostValue;
int avgExhaustPresVal;
int avgExTemp;
int avgVehicleSpeedDiff;
int avgVehicleSpeedRPM;
int avgRpmValue;
int oldRpmValue;
int avgOilTemp;
int evalGearVal;
int avgAtfRef;
int avgOilRef;

unsigned long n2SpeedPulses;
unsigned long n3SpeedPulses;
unsigned long vehicleSpeedPulses;
unsigned long lastSensorTime;
unsigned long rpmPulse;
unsigned long curLog;
unsigned long lastLog;
unsigned long fuelIn;
unsigned long fuelOut;
unsigned long fuelUsed;
unsigned long fuelUsedAvg;
unsigned long vehicleTravelRevs;
unsigned long vehicleTravelDiff;

int n2Speed;
int n3Speed;
int rpmRevs;
int vehicleSpeedRevs;

struct ConfigParam config = {
    .boostMax = 700,    // boost sensor max kpa
    .boostDrop = 50,    // kpa to drop on shifts
    .boostSpring = 120, // kpa for wastegate spring pressure
    .fuelMaxRPM = 2000, // RPM limit to turn on fuel pumps
    .maxRPM = 7000,     // Max engine RPM
    .tireWidth = 195,
    .tireProfile = 65,
    .tireInches = 15,
    .rearDiffTeeth = 29,    // number of teeth in diff
    .nextShiftDelay = 2000, // ms. to wait before next shift to avoid accidental overshifting.
    .stallSpeed = 2200,     // torque converter stall speed
    .batteryLimit = 11500,  // battery voltage limit in 11.5v
    .firstTccGear = 2,      // first gear when tcc is used.
    .triggerWheelTeeth = 6, // number of teeth in trigger wheel for RPM calculation
    .tpsAgre = 2,           // 1-10 how aggressive slope tps has
    .highRPMshiftLimit = 5000,
    .lowRPMshiftLimit = 1000,
    .diffRatio = 3.27,
    .maxSlip = 0.5,     // Maximum allowed slip before error
    .transSloppy = 1.2, // multiplier for pressures coping with old transmissions
    .oneTotwo = 35,
    .twoTothree = 72,
    .threeTofour = 80,
    .fourTofive = 80,
    .fiveTofour = 65,
    .fourTothree = 65,
    .threeTotwo = 17,
    .twoToone = 35};
// -------------------

// Do not change any of these.
// Default for blocking gear switches (do not change.)
bool trans = true;
bool shiftBlocker = false;
bool shiftPending = false;
bool carRunning = false;
bool ignition = true;

bool exhaustPresSensor = false;
bool batteryMonitor = false;
bool boostSensor = false;
bool exhaustTempSensor = true;
bool rpmSpeed = true;
bool diffSpeed = true;
bool adaptive = false;
bool tccLock;
bool manual = false;
double garageTime;
int lockVal = 0;

int boostOverride = 150;

double lastShiftPoint = 0;
int lastXval;
int lastYval;

int initBVoltage = 0; // analogRead(boostPin) * 5.0;
int initEVoltage = 0; // analogRead(exhaustPresPin) * 5.0;

bool evalGear = true;

bool boostLimit;
bool boostLimitShift;

// Stick and keys

// Таблица состояний (заглушка)
const struct StickState stickStates[] = {
    // P (Паркинг)
    {HIGH, HIGH, HIGH, LOW, 8, "P"},

    // R (Реверс)
    {LOW, HIGH, HIGH, HIGH, 7, "R"},

    // N (Нейтраль)
    {HIGH, LOW, HIGH, HIGH, 6, "N"},

    // Drive 5
    {LOW, LOW, HIGH, LOW, 5, "D"},

    // Drive 4
    {LOW, LOW, LOW, HIGH, 4, "D4"},

    // Drive 3
    {LOW, HIGH, LOW, LOW, 3, "D3"},

    // Drive 2
    {HIGH, LOW, LOW, LOW, 2, "D2"},

    // Drive 1
    {HIGH, HIGH, LOW, HIGH, 1, "D1"}};

int currentStateIndex = 0;
// Количество состояний
const int stickStatesCount = sizeof(stickStates) / sizeof(stickStates[0]);
