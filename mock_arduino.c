#include "mock_aruduino.h"

// Глобальная переменная, имитирующая системное время в миллисекундах
static uint32_t mock_millis = 0;

// Заглушка millis() использует монотонные часы системы — не сбрасывается при изменениях системного времени.
uint32_t millis()
{
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (uint32_t)ts.tv_sec * 1000 + ts.tv_nsec / 1000000;
}
