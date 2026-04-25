# Компилятор и флаги
CC := gcc
CFLAGS := -Wall -Wextra -std=gnu11 -O2 -g
LDFLAGS := -lpthread

# Исходные файлы и цель
SRCS := *.c
TARGET := threads_demo
OBJS := $(SRCS:.c=.o)

# Цели, которые не являются файлами
.PHONY: all clean run help rebuild

# Сборка по умолчанию
all: $(TARGET)

# Линковка исполняемого файла
$(TARGET): $(OBJS)
	@echo "Linking $@..."
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)
	@echo "Build complete: ./$(TARGET)"

# Компиляция объектных файлов
%.o: %.c
	@echo "Compiling $<..."
	$(CC) $(CFLAGS) -c -o $@ $<

# Очистка сборочных артефактов
clean:
	@echo "Cleaning..."
	rm -f $(TARGET) $(OBJS)
	@echo "Clean complete."

# Сборка и запуск
run: $(TARGET)
	@echo "Running $(TARGET)..."
	./$(TARGET)

# Полная пересборка с нуля
rebuild: clean all

# Справка по доступным целям
help:
	@echo "Доступные цели:"
	@echo "  all      - Сборка программы (по умолчанию)"
	@echo "  run      - Сборка и запуск программы"
	@echo "  clean    - Удаление собранных файлов"
	@echo "  rebuild  - Полная пересборка (clean + all)"
	@echo "  help     - Эта справка"
