#include "mock_aruduino.h"

// Глобальная переменная, имитирующая системное время в миллисекундах
static uint32_t mock_millis = 0;

// Заглушка millis()
uint32_t millis() { return /*mock_millis*/ 25000; }

// Вспомогательная функция для продвижения времени (используйте в тестах)
void advanceMillis(uint32_t ms) { mock_millis += ms; }
