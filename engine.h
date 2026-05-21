#ifndef ENGINE_H
#define ENGINE_H

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define MIN_RPM 600.0
#define MAX_RPM 8000.0       // верхняя граница
#define MAX_RPM_IDEAL 3500.0 // комфортный предел (можно до 4000)

typedef struct
{
    double current_rpm;
    double target_rpm;
    double gas_pedal;    // от 0.0 до 1.0
    double load;         // нагрузка на двигатель (0..1)
    double speed_kmh;    // скорость автомобиля (км/ч)
    int gear;            // текущая передача (1-5, 0=нейтраль)
    int target_gear;     // желаемая передача (для автоматического переключения)
    double clutch_pedal; // 0=выжато, 1=полностью отпущено (для механической КПП)
} Engine;

// Логика переключения передач(автоматическая) // todo Демо
void update_gear_auto(Engine *eng);

// Инициализация двигателя
void engine_init(Engine *eng);
// Установка педали газа (0..1)
void engine_set_gas_pedal(Engine *eng, double pedal);
// Установка внешней нагрузки (0..1)
void engine_set_load(Engine *eng, double load);
// Главный закон: обновление оборотов за время dt (секунды)
void engine_update_state(Engine *eng, double dt);
// Закон пересчёта оборотов в скорость
double rpm_to_speed(Engine *eng);
// Обратное преобразование(скорость → целевые обороты)
double speed_to_target_rpm(Engine *eng, double target_speed);
// Сцепление для режима работы с механической трансмиссией
void engine_set_clutch(Engine *eng, double pedal);
void engine_apply_clutch(Engine *eng);

extern /*volatile*/ Engine my_engine;

#endif // ENGINE_H
