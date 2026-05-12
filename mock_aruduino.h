#ifndef MOCK_ARDUINO_H
#define MOCK_ARDUINO_H

#include <stdint.h>
#include <stddef.h>

// Определим константы, если они не определены (например, вне Arduino.h)
#ifndef HIGH
#define HIGH 0x1
#endif
#ifndef LOW
#define LOW 0x0
#endif

#ifndef INPUT
#define INPUT 0x0
#endif
#ifndef OUTPUT
#define OUTPUT 0x1
#endif
#ifndef INPUT_PULLUP
#define INPUT_PULLUP 0x2
#endif

// Заглушка, имитирующая pgm_read_word_near, но читающая из RAM
#define pgm_read_word_near(addr) (*(addr))
#define pgm_read_dword_near(addr) (*(addr))

// Определяем типы фильтров (как в оригинальной библиотеке)
#define LOWPASS 0
#define HIGHPASS 1
#define BANDPASS 2
#define BANDSTOP 3

// Заглушка millis()
uint32_t millis();

// Вспомогательная функция для продвижения времени (используйте в тестах)
void advanceMillis(uint32_t ms);

/* ===== Целочисленные версии (полный аналог Arduino) ===== */
static inline long map_long(long x, long in_min, long in_max, long out_min, long out_max)
{
    if (in_max == in_min)
        return out_min;
    return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

static inline int map_int(int x, int in_min, int in_max, int out_min, int out_max)
{
    if (in_max == in_min)
        return out_min;
    return (int)map_long(x, in_min, in_max, out_min, out_max);
}

/* ===== Вещественные версии (сохраняют дробную часть) ===== */
static inline float map_float(float x, float in_min, float in_max, float out_min, float out_max)
{
    if (in_max == in_min)
        return out_min;
    return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

static inline double map_double(double x, double in_min, double in_max, double out_min, double out_max)
{
    if (in_max == in_min)
        return out_min;
    return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

/* ===== C11: автоматический выбор функции по типу первого аргумента ===== */
#if __STDC_VERSION__ >= 201112L
#define map(x, in_min, in_max, out_min, out_max) \
    _Generic((x),                                \
        int: map_int,                            \
        long: map_long,                          \
        float: map_float,                        \
        double: map_double,                      \
        default: map_double)((x), (in_min), (in_max), (out_min), (out_max))
#endif

#define constrain(amt, low, high) \
    ((amt) < (low) ? (low) : ((amt) > (high) ? (high) : (amt)))

// Безопасный map с ограничением входного диапазона
static inline long map_clamped_long(long x, long in_min, long in_max, long out_min, long out_max)
{
    return map_long(constrain(x, in_min, in_max), in_min, in_max, out_min, out_max);
}

#endif // MOCK_ARDUINO_H
