Отличный вопрос! `load` — это связующее звено между двигателем, коробкой и дорогой. Давайте выведем **полную функциональную зависимость**.

## 📐 Основное уравнение движения

```
Двигатель → Коробка → Привод → Колёса → Сопротивления
     ↓           ↓         ↓         ↓
  крутящий   передача   редуктор   нагрузка
   момент
```

## 🔗 Полная система уравнений

### 1. Крутящий момент на колёсах

```c
// Момент на колёсах = момент двигателя * общее передаточное число
double wheel_torque = engine_torque * gear_ratio[gear] * final_drive_ratio;
```

### 2. Сила тяги на колёсах

```c
double traction_force = wheel_torque / wheel_radius_m;  // Ньютоны
```

### 3. Силы сопротивления движению (НАГРУЗКА)

```c
// Полная нагрузка на двигатель = сумма всех сопротивлений
double total_resistance = 
    rolling_resistance +    // сопротивление качению
    aero_resistance +       // аэродинамическое сопротивление
    grade_resistance +      // сопротивление подъёму
    inertia_force;          // инерция (ускорение)
```

## 📊 Функция расчёта `load` для двигателя

```c
typedef struct {
    // Параметры автомобиля
    double mass_kg;              // масса авто 1500 кг
    double wheel_radius_m;       // 0.3 м
    double final_drive_ratio;    // главная пара 3.5
    double gear_ratio[7];        // передаточные числа КПП
    double frontal_area_m2;      // 2.2 м²
    double drag_coefficient;      // 0.3
    double rolling_resistance_coef; // 0.015
    
    // Состояние
    double vehicle_speed_ms;     // скорость м/с
    double road_grade;           // уклон дороги (-0.1..0.1)
    double engine_rpm;
    int current_gear;
} Vehicle;

double calculate_load(Vehicle* veh, double engine_torque_nm) {
    // 1. Передаточное число от двигателя к колёсам
    double total_ratio = veh->gear_ratio[veh->current_gear] * veh->final_drive_ratio;
    
    // 2. Момент на колёсах
    double wheel_torque = engine_torque_nm * total_ratio;
    
    // 3. Сила тяги
    double traction_force = wheel_torque / veh->wheel_radius_m;
    
    // 4. Сила сопротивления качению
    double rolling_resistance = veh->mass_kg * 9.81 * veh->rolling_resistance_coef;
    
    // 5. Аэродинамическое сопротивление
    double air_density = 1.225;  // кг/м³
    double aero_resistance = 0.5 * air_density * veh->drag_coefficient * 
                             veh->frontal_area_m2 * veh->vehicle_speed_ms * veh->vehicle_speed_ms;
    
    // 6. Сопротивление подъёму
    double grade_resistance = veh->mass_kg * 9.81 * sin(veh->road_grade);
    
    // 7. Требуемое усилие
    double required_force = rolling_resistance + aero_resistance + grade_resistance;
    
    // 8. НАГРУЗКА ДВИГАТЕЛЯ (0..1)
    double load;
    if (traction_force > 0) {
        load = required_force / traction_force;
        if (load > 1.0) load = 1.0;   // 100% нагрузки (не хватает мощности)
        if (load < 0.1) load = 0.1;   // минимальная нагрузка (холостой ход)
    } else {
        load = 1.0;  // двигатель тормозит
    }
    
    return load;
}
```

## 🔄 Полная модель двигателя + коробка + нагрузка

```c
// ==================== integrated_model.h ====================
typedef struct {
    // Двигатель
    double rpm;
    double torque_nm;
    double gas_pedal;
    double load;           // 0..1, рассчитывается динамически
    
    // Коробка
    int gear;
    double clutch_position;
    
    // Автомобиль
    double speed_kmh;
    double road_grade;     // уклон в процентах (-10% .. +10%)
    
    // Параметры авто
    double mass_kg;
    double wheel_radius_m;
    double final_drive;
    double gear_ratios[7];
} VehicleModel;

// Двигатель: расчёт крутящего момента по оборотам и газу
double engine_get_torque(double rpm, double gas) {
    // Типичная характеристика ДВС (пик момента ~3000-3500)
    double rpm_norm = (rpm - 1000.0) / 3000.0;
    if (rpm_norm < 0) rpm_norm = 0;
    if (rpm_norm > 1) rpm_norm = 1;
    
    // Максимальный момент 200 Нм при 3000 об/мин
    double max_torque = 200.0 * sin(rpm_norm * M_PI);
    return max_torque * gas;
}

// Расчёт нагрузки на двигатель (ключевая функция!)
double calculate_engine_load(VehicleModel* vm) {
    // 1. Текущий крутящий момент двигателя
    double engine_torque = engine_get_torque(vm->rpm, vm->gas_pedal);
    
    // 2. Передаточное число от двигателя к колёсам
    double total_ratio = vm->gear_ratios[vm->gear] * vm->final_drive;
    
    // 3. Скорость в м/с
    double speed_ms = vm->speed_kmh / 3.6;
    
    // 4. Сила сопротивления качению
    double rolling = vm->mass_kg * 9.81 * 0.015;
    
    // 5. Аэродинамика
    double aero = 0.5 * 1.225 * 0.3 * 2.2 * speed_ms * speed_ms;
    
    // 6. Подъём (упрощённо: sin(atan(grade/100)))
    double grade_rad = atan(vm->road_grade / 100.0);
    double grade_force = vm->mass_kg * 9.81 * sin(grade_rad);
    
    // 7. Суммарное сопротивление
    double resistance_force = rolling + aero + grade_force;
    
    // 8. Сила тяги на колёсах
    double wheel_torque = engine_torque * total_ratio;
    double traction_force = wheel_torque / vm->wheel_radius_m;
    
    // 9. Нагрузка = отношение требуемого усилия к возможному
    double load;
    if (traction_force > 0) {
        load = resistance_force / traction_force;
        load = fmax(0.1, fmin(1.0, load));  // clamp 0.1..1.0
    } else {
        load = 1.0;  // двигатель тормозит
    }
    
    return load;
}

// Основной закон изменения оборотов
void update_vehicle_state(VehicleModel* vm, double dt) {
    // 1. Расчёт текущего крутящего момента
    double engine_torque = engine_get_torque(vm->rpm, vm->gas_pedal);
    
    // 2. Расчёт нагрузки (связка двигатель-коробка-дорога)
    vm->load = calculate_engine_load(vm);
    
    // 3. Целевые обороты от педали газа (с учётом нагрузки)
    double target_rpm = 1000.0 + vm->gas_pedal * 3000.0;
    target_rpm = target_rpm * (1.0 - vm->load * 0.3);  // нагрузка снижает целевые обороты
    
    // 4. Динамика оборотов (инерция двигателя)
    double rpm_diff = target_rpm - vm->rpm;
    double rpm_delta = rpm_diff * 4.0 * dt;
    
    // Ограничение по моменту инерции
    double max_accel = 2000.0 * (1.0 - vm->load) * dt;
    double max_decel = -1500.0 * (1.0 + vm->load) * dt;
    
    if (rpm_delta > max_accel) rpm_delta = max_accel;
    if (rpm_delta < max_decel) rpm_delta = max_decel;
    
    vm->rpm += rpm_delta;
    vm->rpm = fmax(1000.0, fmin(4000.0, vm->rpm));
    
    // 5. Обновление скорости автомобиля
    double total_ratio = vm->gear_ratios[vm->gear] * vm->final_drive;
    double net_force = traction_force - resistance_force;
    double acceleration = net_force / vm->mass_kg;
    
    double speed_ms = vm->speed_kmh / 3.6;
    speed_ms += acceleration * dt;
    if (speed_ms < 0) speed_ms = 0;
    vm->speed_kmh = speed_ms * 3.6;
    
    // Синхронизация: если сцепление отпущено, скорость связана с оборотами
    if (vm->clutch_position > 0.9 && speed_ms > 0.1) {
        double expected_rpm = speed_ms / (0.377 * vm->wheel_radius_m) * total_ratio;
        if (fabs(vm->rpm - expected_rpm) > 500) {
            vm->rpm = expected_rpm;  // принудительная синхронизация
        }
    }
}
```

## 🔬 Функциональная зависимость в формуле

```
                    engine_torque × gear_ratio × final_drive
load = 1 - ──────────────────────────────────────────────────
              rolling_resistance + aero_resistance + grade_resistance

где:
• rolling ≈ m × g × Crr
• aero ≈ 0.5 × ρ × Cd × A × v²
• grade ≈ m × g × sin(α)
```

## 📈 Пример работы

```c
int main() {
    VehicleModel vm = {
        .mass_kg = 1500,
        .wheel_radius_m = 0.3,
        .final_drive = 3.5,
        .gear_ratios = {0, 4.0, 2.5, 1.6, 1.2, 1.0, 0.8},
        .gear = 2,
        .rpm = 2000,
        .speed_kmh = 30,
        .gas_pedal = 0.5,
        .road_grade = 0  // равнина
    };
    
    for (int i = 0; i < 100; i++) {
        update_vehicle_state(&vm, 0.1);
        printf("rpm=%.0f speed=%.1f load=%.2f\n", 
               vm.rpm, vm.speed_kmh, vm.load);
    }
    return 0;
}
```

## 💡 Итоговая формула связи

```
load = f(engine_torque, gear_ratio, speed, road_grade, vehicle_params)
```

**Физический смысл:**
- `load = 0.1-0.3` → движение накатом, малая нагрузка
- `load = 0.4-0.7` → нормальное движение, средняя нагрузка
- `load = 0.8-1.0` → тяжёлые условия (подъём, разгон), двигатель работает на пределе
- `load = 1.0+` → не хватает мощности (нужно понизить передачу)

Такая модель даёт **полную физическую связь** между всеми компонентами автомобиля!
