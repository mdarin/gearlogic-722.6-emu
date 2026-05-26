#ifndef MAPS_H
#define MAPS_H

/*
Структура карты

Карта представляет собой 2D-таблицу с осями:
X — обычно RPM (обороты двигателя)
Y — нагрузка (MAP, TPS и т.п.)

        RPM:  800  1200  1600  2000 ...
Load:
  20%  [ v00   v10   v20   v30 ]
  40%  [ v01   v11   v21   v31 ]
  60%  [ v02   v12   v22   v32 ]
*/

#include <stdint.h>

extern const int atfSensorMap[23][2];
extern const int oilSensorMap[25][2];
extern const int injectionMap[14][12];
extern const int gearMap[14][12];
extern const int mpcNormalMap[14][12];
extern const int shiftTimeMap[14][12];
extern const int boostControlPressureMap[14][6];
extern const int spcMap12[14][12];
extern const int mpcMap12[14][12];
extern const int spcMap23[14][12];
extern const int mpcMap23[14][12];
extern const int mpcMap23[14][12];
extern const int spcMap34[14][12];
extern const int mpcMap34[14][12];
extern const int spcMap45[14][12];
extern const int mpcMap45[14][12];
extern const int spcMap54[14][12];
extern const int mpcMap54[14][12];
extern const int spcMap43[14][12];
extern const int mpcMap43[14][12];
extern const int spcMap32[14][12];
extern const int mpcMap32[14][12];
extern const int spcMap21[14][12];
extern const int mpcMap21[14][12];
extern const uint8_t mersu_map[];

#endif // MAPS_H
