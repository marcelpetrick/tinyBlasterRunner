UNITY_DIR := $(IDF_PATH)/components/unity/unity/src
CC        := gcc
CFLAGS    := -Wall -Wextra -std=c11 -I. -I../main -I$(UNITY_DIR)
SRCS      := main/test_main.c ../main/game_math.c $(UNITY_DIR)/unity.c
TARGET    := test_runner

COV_FLAGS := $(CFLAGS) --coverage
COV_BIN   := test_runner_cov

.PHONY: all clean run coverage

all: $(TARGET)

$(TARGET): $(SRCS)
	$(CC) $(CFLAGS) $^ -o $@ -lm

run: $(TARGET)
	./$(TARGET)

$(COV_BIN): $(SRCS)
	$(CC) $(COV_FLAGS) $^ -o $@ -lm

coverage: $(COV_BIN)
	./$(COV_BIN)
	lcov --capture --directory . --output-file coverage.info 2>/dev/null
	lcov --remove coverage.info '*/unity/*' '*/test_main*' \
	     --output-file coverage_filtered.info 2>/dev/null
	genhtml coverage_filtered.info --output-directory coverage_html 2>/dev/null
	@LINE=$$(lcov --summary coverage_filtered.info 2>&1 | \
	         grep 'lines' | grep -oE '[0-9]+\.[0-9]+%' | head -1); \
	 echo "Coverage: $${LINE} lines → test/coverage_html/index.html"

clean:
	rm -f $(TARGET) $(COV_BIN) coverage.info coverage_filtered.info *.gcda *.gcno
	rm -rf coverage_html
