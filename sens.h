#ifndef SENS_H
#define SENS_H

#include <stdint.h>
#include <stdbool.h>
#include "params.h"
#include "calc.h"
#include "core.h"

// Поддерживает адаптивную настройку давления
int adaptSPC(int, int, int);
int speedRead(void);
int tpsRead(void);
int rpmRead(void);
int oilRead(void);
int boostRead(void);
int exhaustPressureRead(void);
int batteryRead(void);
int boostLimitRead(int);
int loadRead(int, int, int, int);
int atfRead(void);
int exhaustTempRead(void);

struct SensorVals readSensors(void);

#endif // SENS_H
