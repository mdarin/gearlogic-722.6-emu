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
        tv.tv_sec = 1;
        tv.tv_usec = 0;

        int ret = select(STDIN_FILENO + 1, &set, NULL, NULL, &tv);
        if (ret > 0 && FD_ISSET(STDIN_FILENO, &set))
        {
            if (read(STDIN_FILENO, &c, 1) == 1)
            {
                switch (c)
                {
                case 'a':
                    printf("[KeysThread] 'a' pressed\n");
                    break;
                case 'w':
                    printf("[KeysThread] 'w' pressed\n");
                    break;
                case 's':
                    printf("[KeysThread] 's' pressed\n");
                    break;
                case 'd':
                    printf("[KeysThread] 'd' pressed\n");
                    break;
                default:
                    break;
                }
            }
        }
        else
        {
            // Таймаут 2000 мс – здесь выполняется "цикл с задержкой 2000 мс"
            printf("[KeysThread] 2000 ms delay...\n");
        }
    }
    printf("[KeysThread] Exiting.\n");
    return NULL;
}

// Поток 2: просто цикл с задержкой 2000 мс
void *dummy_thread1(void *arg)
{
    (void)arg;
    printf("[DummyThread1] Started.\n");
    while (keep_running)
    {
        printf("[DummyThread1] 2000 ms delay...\n");
        sleep(2); // задержка 2000 мс
    }
    printf("[DummyThread1] Exiting.\n");
    return NULL;
}

// Поток 3: просто цикл с задержкой 2000 мс
void *dummy_thread2(void *arg)
{
    (void)arg;
    printf("[DummyThread2] Started.\n");
    while (keep_running)
    {
        printf("[DummyThread2] 2000 ms delay...\n");
        sleep(2);
    }
    printf("[DummyThread2] Exiting.\n");
    return NULL;
}

int main()
{
    pthread_t t1, t2, t3;
    struct sigaction sa = {0};

    // Установка обработчика Ctrl+C
    // memset(&sa, 0, sizeof(sa));
    sa.sa_handler = sigint_handler;
    sigaction(SIGINT, &sa, NULL);

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

    // Восстанавливаем терминал
    restore_terminal_mode();
    printf("[Main] All threads finished. Goodbye.\n");
    return EXIT_SUCCESS;
}
