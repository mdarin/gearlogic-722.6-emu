#include "teleplot.h"

static int udp_socket = -1;
static struct sockaddr_in teleplot_addr;
static pthread_mutex_t teleplot_mutex = PTHREAD_MUTEX_INITIALIZER;
static int teleplot_initialized = 0;

int teleplot_init(const char *host, uint16_t port)
{
    if (teleplot_initialized)
    {
        teleplot_close();
    }

    udp_socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (udp_socket < 0)
    {
        perror("[Teleplot] socket create failed");
        return -1;
    }

    // Опционально: увеличиваем буфер отправки
    int sndbuf = 65536;
    setsockopt(udp_socket, SOL_SOCKET, SO_SNDBUF, &sndbuf, sizeof(sndbuf));

    memset(&teleplot_addr, 0, sizeof(teleplot_addr));
    teleplot_addr.sin_family = AF_INET;
    teleplot_addr.sin_port = htons(port);

    if (inet_pton(AF_INET, host, &teleplot_addr.sin_addr) <= 0)
    {
        perror("[Teleplot] inet_pton failed");
        close(udp_socket);
        udp_socket = -1;
        return -1;
    }

    teleplot_initialized = 1;
    printf("[Teleplot] Connected to %s:%u\n", host, port);
    return 0;
}

static int teleplot_send_raw(const char *message)
{
    if (!teleplot_initialized || udp_socket < 0)
    {
        return -1;
    }

    pthread_mutex_lock(&teleplot_mutex);

    ssize_t sent = sendto(udp_socket, message, strlen(message), 0,
                          (struct sockaddr *)&teleplot_addr, sizeof(teleplot_addr));

    pthread_mutex_unlock(&teleplot_mutex);

    if (sent < 0)
    {
        perror("[Teleplot] send failed");
        return -1;
    }
    return 0;
}

int teleplot_send(const char *var_name, double value)
{
    // Получаем текущее время в миллисекундах
    struct timeval tv;
    gettimeofday(&tv, NULL);
    int64_t timestamp_ms = (int64_t)tv.tv_sec * 1000 + tv.tv_usec / 1000;

    return teleplot_send_ts(var_name, timestamp_ms, value);
}

int teleplot_send_ts(const char *var_name, int64_t timestamp_ms, double value)
{
    char msg[TELEPLOT_MAX_MSG];
    // Формат: varName:timestamp:value|g
    int len = snprintf(msg, sizeof(msg), "%s:%lld:%.6g|g",
                       var_name, (long long)timestamp_ms, value);

    if (len < 0 || len >= (int)sizeof(msg))
    {
        fprintf(stderr, "[Teleplot] message too long or encoding error\n");
        return -1;
    }

    return teleplot_send_raw(msg);
}

int teleplot_send_str(const char *var_name, const char *str_value)
{
    char msg[TELEPLOT_MAX_MSG];
    // Формат для строк: varName:"value"|s
    int len = snprintf(msg, sizeof(msg), "%s:\"%s\"|s", var_name, str_value);

    if (len < 0 || len >= (int)sizeof(msg))
    {
        fprintf(stderr, "[Teleplot] message too long or encoding error\n");
        return -1;
    }

    return teleplot_send_raw(msg);
}

void teleplot_close(void)
{
    if (udp_socket >= 0)
    {
        close(udp_socket);
        udp_socket = -1;
    }
    teleplot_initialized = 0;
    printf("[Teleplot] Closed\n");
}
