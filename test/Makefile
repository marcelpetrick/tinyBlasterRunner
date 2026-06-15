UNITY_DIR := $(IDF_PATH)/components/unity/unity/src
CC        := gcc
CFLAGS    := -Wall -Wextra -std=c11 -I. -I../main -I$(UNITY_DIR)
SRCS      := main/test_main.c ../main/game_math.c $(UNITY_DIR)/unity.c
TARGET    := test_runner

.PHONY: all clean run

all: $(TARGET)

$(TARGET): $(SRCS)
	$(CC) $(CFLAGS) $^ -o $@ -lm

run: $(TARGET)
	./$(TARGET)

clean:
	rm -f $(TARGET)
