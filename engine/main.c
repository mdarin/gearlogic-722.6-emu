#include <stdio.h>
#include <stdlib.h>
#include "engine.h"

// Демонстрация работы
int main()
{
    Engine engine;
    engine_init(&engine);

    double time = 0.0;
    double dt = 0.05; // шаг 50 мс
    // double dt = 0.1; // шаг 100 мс
    // double dt = 1.0;

    printf("initial state: %4.1f\t%4.2f\t%d\t\t%5.0f\t%5.1f\n",
           time, engine.gas_pedal, engine.gear,
           engine.current_rpm, engine.speed_kmh);

    printf("Время\tГаз\tПередача\tОбороты\tСкорость\n");
    printf("-----------------------------------------------\n");

    for (int step = 0; step < 500; step++)
    {
        // Сценарий: разгон, переключения, торможение
        if (time < 3.0)
        {
            engine_set_gas_pedal(&engine, 0.6);
            engine_set_load(&engine, 0.5);
        }
        else if (time < 8.0)
        {
            engine_set_gas_pedal(&engine, 0.7); // интенсивный разгон
            engine_set_load(&engine, 0.3);
        }
        else if (time < 12.0)
        {
            engine_set_gas_pedal(&engine, 0.3); // поддержание скорости
            engine_set_load(&engine, 0.3);
        }
        else if (time < 15.0)
        {
            engine_set_gas_pedal(&engine, 0.0); // сброс газа
            engine_set_load(&engine, 0.7);
        }
        else
        {
            engine_set_gas_pedal(&engine, 0.0); // остановка
            engine_set_load(&engine, 0.0);
        }

        engine_update_state(&engine, dt);

        if (step % 10 == 0)
        {
            printf("%4.1f\t%4.2f\t%d\t\t%5.0f\t%5.1f\n",
                   time, engine.gas_pedal, engine.gear,
                   engine.current_rpm, engine.speed_kmh);
        }

        time += dt;
    }

    return 0;
}
