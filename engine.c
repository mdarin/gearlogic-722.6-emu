#include "engine.h"

// Максимальная скорость на каждой передаче при 4000 об/мин
const double GEAR_MAX_SPEED[] = {0, 30, 55, 80, 110, 150}; // 1-5 передачи
//                    нейтраль  1   2   3   4    5

// volatile
Engine my_engine;

// Инициализация двигателя
void engine_init(Engine *eng)
{
    eng->current_rpm = MIN_RPM;
    eng->target_rpm = MIN_RPM;
    eng->gas_pedal = 0.0;
    eng->load = 0.2; // базовая нагрузка (трение)
    eng->speed_kmh = 0.0;
    eng->gear = 1;           // начинаем с 1-й передачи
    eng->clutch_pedal = 0.0; // сцепление отпущено 1.0
    eng->target_rpm = MIN_RPM;
}

// Установка педали газа (0..1)
void engine_set_gas_pedal(Engine *eng, double pedal)
{
    if (pedal < 0.0)
        pedal = 0.0;
    if (pedal > 1.0)
        pedal = 1.0;
    eng->gas_pedal = pedal;

    // Целевые обороты: линейно от MIN_RPM до MAX_RPM в зависимости от педали
    // Но с учётом нагрузки — максимальные обороты снижаются при большой нагрузке
    // double max_achievable = MAX_RPM_IDEAL - (eng->load * (MAX_RPM_IDEAL - MIN_RPM) * 0.5);
    // if (max_achievable < MIN_RPM)
    //     max_achievable = MIN_RPM;

    // eng->target_rpm = MIN_RPM + pedal * (max_achievable - MIN_RPM);
}

// Установка внешней нагрузки (0..1)
void engine_set_load(Engine *eng, double load)
{
    if (load < 0.0)
        load = 0.0;
    if (load > 1.0)
        load = 1.0;
    eng->load = load;
}

// Главный закон: обновление оборотов за время dt (секунды)
void engine_update_state(Engine *eng, double dt)
{
    // Автоматическое переключение передач // todo (можно отключить для ручного)
    update_gear_auto(eng);

    // Коэффициент инерции (чем выше, тем быстрее отклик)
    double inertia = 5.0; // [1/сек]

    // Разница между целевыми и текущими оборотами
    double diff = eng->target_rpm - eng->current_rpm;

    // Ограничение на максимальное ускорение и замедление
    double max_accel = 800.0;   // об/сек^2 (резкое нажатие) // todo значение подбирается
    double max_decel = -1200.0; // об/сек^2 (отпускание газа + трение) // todo значение подбирается

    // Добавляем эффект нагрузки: при большой нагрузке ускорение падает
    double load_penalty = 1.0 - eng->load * 0.6; // до 40% потери
    if (diff > 0)
    {
        diff = diff * load_penalty;
    }
    else
    {
        // При замедлении нагрузка помогает торможению двигателем
        diff = diff * (1.0 + eng->load * 0.5);
    }

    // Сглаженное изменение через ограничение скорости изменения
    double delta_rpm = diff * inertia * dt;

    // Ограничение максимального изменения за шаг
    if (delta_rpm > max_accel * dt)
        delta_rpm = max_accel * dt;
    if (delta_rpm < max_decel * dt)
        delta_rpm = max_decel * dt;

    // Применяем изменение
    eng->current_rpm += delta_rpm;

    // Ограничение диапазона от MIN_RPM до MAX_RPM
    if (eng->current_rpm < MIN_RPM)
        eng->current_rpm = MIN_RPM;
    if (eng->current_rpm > MAX_RPM)
        eng->current_rpm = MAX_RPM;

    // Эффект "провала" при резком сбросе газа ниже 1200 об/мин
    if (eng->current_rpm < MIN_RPM + 200 && eng->gas_pedal < 0.05)
    {
        eng->current_rpm = MIN_RPM; // холостой ход чётко
    }

    // Расчёт целевых оборотов с учётом передачи
    double target_speed_from_gas = eng->gas_pedal * GEAR_MAX_SPEED[eng->gear];
    double target_rpm_from_gas = speed_to_target_rpm(eng, target_speed_from_gas);
    eng->target_rpm = target_rpm_from_gas;

    // Инерция автомобиля
    double speed_diff = target_speed_from_gas - eng->speed_kmh;
    eng->speed_kmh += speed_diff * 2.0 * dt;
    if (eng->speed_kmh < 0)
        eng->speed_kmh = 0;
}

// Закон пересчёта оборотов в скорость
double rpm_to_speed(Engine *eng)
{
    if (eng->gear == 0)
        return 0.0; // нейтраль

    // Линейная зависимость: скорость = (обороты / MAX_RPM) * макс_скорость_на_передаче
    double rpm_percent = (eng->current_rpm - MIN_RPM) / (MAX_RPM - MIN_RPM);
    if (rpm_percent < 0)
        rpm_percent = 0;
    if (rpm_percent > 1)
        rpm_percent = 1;

    return rpm_percent * GEAR_MAX_SPEED[eng->gear];
}

// Обратное преобразование(скорость → целевые обороты)
double speed_to_target_rpm(Engine *eng, double target_speed)
{
    if (eng->gear == 0 || target_speed == 0)
        return MIN_RPM;

    double max_speed_on_gear = GEAR_MAX_SPEED[eng->gear];
    if (target_speed > max_speed_on_gear)
        target_speed = max_speed_on_gear;

    double rpm_percent = target_speed / max_speed_on_gear;
    return MIN_RPM + rpm_percent * (MAX_RPM - MIN_RPM);
}

// Сцепление для режима работы с механической трансмиссией
void engine_set_clutch(Engine *eng, double pedal)
{
    eng->clutch_pedal = pedal;
}

void engine_apply_clutch(Engine *eng)
{
    // При выжатом сцеплении обороты падают к холостым
    if (eng->clutch_pedal < 0.1)
    {
        eng->current_rpm = MIN_RPM + (eng->current_rpm - MIN_RPM) * 0.95;
    }
}

// Логика переключения передач(автоматическая) // todo Демо
void update_gear_auto(Engine *eng)
{
    // Зона комфортных оборотов для каждой передачи
    double rpm = eng->current_rpm;

    // Переключение вверх при высоких оборотах
    if (eng->gear < 5 && rpm > 3500 && eng->gas_pedal > 0.3)
    {
        eng->gear++;
        printf("*** Переключение на %d передачу ***\n", eng->gear);
        // При переключении обороты падают (имитация)
        eng->current_rpm *= 0.7;
    }
    // Переключение вниз при низких оборотах (кроме 1-й)
    else if (eng->gear > 1 && rpm < 1500 && eng->gas_pedal < 0.2)
    {
        eng->gear--;
        printf("*** Переключение на %d передачу ***\n", eng->gear);
        // При переключении обороты падают (имитация)
        eng->current_rpm *= 1.4;
    }
    // Принудительное переключение вниз при полной остановке
    else if (eng->speed_kmh < 5 && eng->gear != 1)
    {
        eng->gear = 1;
        printf("*** Переключение на 1 передачу ***\n");
    }
}
