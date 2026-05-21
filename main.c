#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <termios.h>
#include <signal.h>
#include <string.h>
#include <sys/select.h>
#include <sys/time.h>
#include <signal.h>
#include <unistd.h>
#include <stdint.h>
#include <stdbool.h>
#include "teleplot.h"
#include "pollstick.h"
#include "decidegear.h"
#include "params.h"
#include "engine.h"
#include "mock_aruduino.h"
#include "pollstick.h"

static volatile int keep_running = 1;

// Обработчик сигнала (Ctrl+C)
void sigint_handler(int sig)
{
    (void)sig;
    keep_running = 0;
    write(STDOUT_FILENO, "\nCaught SIGINT, exiting...\n", 26);
}

// Настройка терминала в неканонический режим (без ожидания Enter)
void set_noncanonical_mode()
{
    struct termios t;
    tcgetattr(STDIN_FILENO, &t);
    t.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &t);
}

// Восстановление исходных настроек терминала
void restore_terminal_mode()
{
    struct termios t;
    tcgetattr(STDIN_FILENO, &t);
    t.c_lflag |= (ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &t);
}

// ------[ Имитатор систем автомобиля ] ---------------------------------------------------------------

// Поток 1: обработка клавиш a, w, s, d
void *keys_thread(void *arg)
{
    (void)arg;
    char c;
    printf("[KeysThread] Started. Use 'a','w','s','d' (press 'q' in main to quit).\n");
    while (keep_running)
    {
        // Читаем один символ, если доступен; иначе спим 2000 мс
        fd_set set;
        struct timeval tv;
        FD_ZERO(&set);
        FD_SET(STDIN_FILENO, &set);
        tv.tv_sec = 0;
        tv.tv_usec = 300000;

        int ret = select(STDIN_FILENO + 1, &set, NULL, NULL, &tv);
        if (ret > 0 && FD_ISSET(STDIN_FILENO, &set))
        {
            if (read(STDIN_FILENO, &c, 1) == 1)
            {
                switch (c)
                {
                case 'a':
                    // printf("[KeysThread] 'a' pressed\n");
                    break;

                case 'w':
                    // printf("[KeysThread] 'w' pressed\n");
                    currentStateIndex++;
                    if (currentStateIndex >= stickStatesCount)
                    {
                        currentStateIndex = stickStatesCount - 1;
                    }
                    break;

                case 's':
                    // printf("[KeysThread] 's' pressed\n");
                    currentStateIndex--;
                    if (currentStateIndex < 0)
                    {
                        currentStateIndex = 0;
                    }
                    break;

                case 'd':
                    // printf("[KeysThread] 'd' pressed\n");
                    break;

                default:
                    break;
                }
            }
        }
        else
        {
            // Таймаут 2000 мс – здесь выполняется "цикл с задержкой 2000 мс"
            // printf("[KeysThread] 2000 ms delay...\n");
        }
    }
    printf("[KeysThread] Exiting.\n");
    return NULL;
}

// Поток 2: просто цикл с задержкой 2000 мс
void *dummy_thread1(void *arg)
{
    (void)arg;
    int counter = 0;
    printf("[DummyThread1] Started.\n");

    while (keep_running)
    {
        // Отправка числового значения
        double sensor_value = 20.0 + (counter % 10) * 1.5;
        // teleplot_send("sensor_1", sensor_value);

        // Отправка строкового статуса
        if (counter % 5 == 0)
        {
            // teleplot_send("thread1_status", (counter % 10 == 0) ? 1 : 0);
        }

        // printf("[DummyThread1] Sent data, counter=%d\n", counter++);
        sleep(2);
    }
    printf("[DummyThread1] Exiting.\n");
    return NULL;
}

// Поток 3: просто цикл с задержкой 2000 мс
void *dummy_thread2(void *arg)
{
    (void)arg;

    engine_init(&my_engine);
    double time = 0.0;
    // double dt = 0.05; // шаг 50 мс
    double dt = 0.5; // шаг 500 мс
    int step = 0;

    printf("[DummyThread2] Started.\n");
    while (keep_running)
    {
        if (time < 15.0)
        {
            engine_set_gas_pedal(&my_engine, 0.0);
            engine_set_load(&my_engine, 0.2);
        }
        else if (time < 80.0)
        {
            engine_set_gas_pedal(&my_engine, 0.6);
        }
        else if (time < 120.0)
        {
            engine_set_gas_pedal(&my_engine, 0.8);
            engine_set_load(&my_engine, 0.8); // высокая нагрузка (подъём)
        }
        else if (time < 150.0)
        {
            engine_set_gas_pedal(&my_engine, 0.3);
            engine_set_load(&my_engine, 0.2);
        }
        else
        {
            engine_set_gas_pedal(&my_engine, 0.4);
        }

        engine_update_state(&my_engine, dt);

        n2Speed = my_engine.current_rpm;
        // todo P and N n3speed = 0 нет связи с двигателем
        n3Speed = (int)(0.8 * (double)(n2Speed));

        teleplot_send("tps", my_engine.gas_pedal);
        teleplot_send("engine_rpm", my_engine.current_rpm);
        teleplot_send("speed_kmh", my_engine.speed_kmh);
        teleplot_send("n2speed_rpm", n2Speed);
        teleplot_send("n3speed_rpm", n3Speed);

        // if (step % 10 == 0)
        // {
        //     printf("%4.1f\t%4.2f\t%d\t\t%5.0f\t%5.1f\n",
        //            time, my_engine.gas_pedal, my_engine.gear,
        //            my_engine.current_rpm, my_engine.speed_kmh);
        // }

        time += dt;
        step++;

        // usleep(50000); // 50 000 мкс = 50 мс (шаг 0.05 сек)
        usleep(500000); // 500 мс
    }
    printf("[DummyThread2] Exiting.\n");
    return NULL;
}

// ------[ Задачи управления АКПП ] -------------------------------------------------------------------

// Поток управления опросом органов управления (кулиса и лепестки/кнопки)
void *pollstick_thread(void *arg)
{
    (void)arg;
    printf("[PollstickThread] Started.\n");
    while (keep_running)
    {
        pollstick(NULL);
        sleep(1);
    }
    printf("[PollstickThread] Exiting.\n");
    return NULL;
}

// Поток логики принятия решения на переключение
void *decidegear_thread(void *arg)
{
    (void)arg;
    printf("[DecidegearThread] Started.\n");
    while (keep_running)
    {
        decideGear(NULL);
        sleep(1);
    }
    printf("[DecidegearThread] Exiting.\n");
    return NULL;
}

// todo Поток управления АКПП выполняет процедуру переключения на выбранную передачу
void *polltrans_thread(void *arg)
{
    (void)arg;
    printf("[PolltransThread] Started.\n");
    while (keep_running)
    {
        printf("[PolltransThread] 2000 ms delay...\n");
        sleep(2); // задержка 2000 мс
    }
    printf("[PolltransThread] Exiting.\n");
    return NULL;
}

// ------[ Менеджер задач ] ---------------------------------------------------------------------------

int main()
{
    pthread_t t1, t2, t3, t4, t5;
    struct sigaction sa = {0};

    // Установка обработчика Ctrl+C
    // memset(&sa, 0, sizeof(sa));
    sa.sa_handler = sigint_handler;
    sigaction(SIGINT, &sa, NULL);

    if (teleplot_init("127.0.0.1", 47269) != 0)
    {
        fprintf(stderr, "Failed to initialize Teleplot\n");
        // Можно продолжить работу без телеметрии
    }

    // Переводим терминал в режим посимвольного чтения (без буферизации строк)
    set_noncanonical_mode();

    // Запускаем потоки
    if (pthread_create(&t1, NULL, keys_thread, NULL) != 0)
    {
        perror("pthread_create keys_thread");
        restore_terminal_mode();
        return EXIT_FAILURE;
    }
    if (pthread_create(&t2, NULL, dummy_thread1, NULL) != 0)
    {
        perror("pthread_create dummy_thread1");
        restore_terminal_mode();
        return EXIT_FAILURE;
    }
    if (pthread_create(&t3, NULL, dummy_thread2, NULL) != 0)
    {
        perror("pthread_create dummy_thread2");
        restore_terminal_mode();
        return EXIT_FAILURE;
    }
    // todo запускам потоки с задачами управления АКПП
    if (pthread_create(&t4, NULL, pollstick_thread, NULL) != 0)
    {
        perror("pthread_create pollstick_thread");
        restore_terminal_mode();
        return EXIT_FAILURE;
    }
    if (pthread_create(&t5, NULL, decidegear_thread, NULL) != 0)
    {
        perror("pthread_create decidegear_thread");
        restore_terminal_mode();
        return EXIT_FAILURE;
    }
    // if (pthread_create(&t3, NULL, dummy_thread2, NULL) != 0)
    // {
    //     perror("pthread_create dummy_thread2");
    //     restore_terminal_mode();
    //     return EXIT_FAILURE;
    // }

    printf("[Main] Threads started. Press 'q' or Ctrl+C to exit.\n");

    // Главный поток блокируется в ожидании 'q' или сигнала завершения
    char c;
    while (keep_running)
    {
        if (read(STDIN_FILENO, &c, 1) == 1)
        {
            if (c == 'q')
            {
                printf("\n[Main] 'q' pressed, exiting...\n");
                keep_running = 0;
                break;
            }
        }
    }

    // Ожидаем завершения потоков
    pthread_join(t1, NULL);
    pthread_join(t2, NULL);
    pthread_join(t3, NULL);
    pthread_join(t4, NULL);
    pthread_join(t5, NULL);

    // Восстанавливаем терминал
    restore_terminal_mode();
    teleplot_close(); // Очистка ресурсов
    printf("[Main] Telemetry terminated.\n");
    printf("[Main] All threads finished. Goodbye.\n");
    return EXIT_SUCCESS;
}
