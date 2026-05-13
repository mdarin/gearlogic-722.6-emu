#ifndef TELEPLOT_H
#define TELEPLOT_H

#include <stdint.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <pthread.h>
#include <errno.h>

/*
📋 Форматы сообщений Teleplot
Формат                  Описание               Пример
name:value|g            Числовое значение      speed:123.4|g
name:timestamp:value|g  С меткой времени (мс)  temp:1627551892437:25.6|g
name:"text"|s           Строковое значение     status:"OK"|s
name:x:y:value|g        Точка на 2D-графике    trajectory:10:20:5.5|g
*/

// Инициализация UDP-соединения с Teleplot
// host: обычно "127.0.0.1", port: 47269
int teleplot_init(const char *host, uint16_t port);

// Отправка значения с автоматической меткой времени
// Пример: teleplot_send("motor_speed", 1234.5);
int teleplot_send(const char *var_name, double value);

// Отправка значения с явной меткой времени (в миллисекундах)
// Пример: teleplot_send_ts("temp", 1627551892437, 25.6);
int teleplot_send_ts(const char *var_name, int64_t timestamp_ms, double value);

// Отправка строкового значения (для текстовых меток)
// Пример: teleplot_send_str("status", "OK");
int teleplot_send_str(const char *var_name, const char *str_value);

// Закрытие соединения и очистка ресурсов
void teleplot_close(void);

// Макросы для удобства (автоматическое приведение типов)
#define TELEPLOT_SEND_INT(name, val) teleplot_send(name, (double)(val))
#define TELEPLOT_SEND_FLOAT(name, val) teleplot_send(name, (double)(val))
#define TELEPLOT_SEND_POINT(name, x, y, val)                                  \
    do                                                                        \
    {                                                                         \
        char _msg[256];                                                       \
        snprintf(_msg, sizeof(_msg), "%s:%.2f:%.2f:%.6g|g", name, x, y, val); \
        teleplot_send_raw(_msg);                                              \
    } while (0)

#define TELEPLOT_SEND_IF(cond, name, val) \
    do                                    \
    {                                     \
        if (cond)                         \
            teleplot_send(name, val);     \
    } while (0)

#define TELEPLOT_MAX_MSG 256
#define TELEPLOT_DEFAULT_HOST "127.0.0.1"
#define TELEPLOT_DEFAULT_PORT 47269

#endif // TELEPLOT_H
