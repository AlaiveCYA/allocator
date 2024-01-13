# Definiowanie kompilatora i flag
CC = gcc
CFLAGS = -Wall -Wextra -g -std=c17

# Definiowanie plików źródłowych i obiektowych
SRCS = $(wildcard src/*.c)
OBJS = $(SRCS:.c=.o)

# Definiowanie plików testowych
TESTS = $(wildcard tests/*.c)

# Definiowanie narzędzi do analizy
VALGRIND_FLAGS = --leak-check=full --show-leak-kinds=all --track-origins=yes --trace-children=yes --error-exitcode=1
CLANG-TIDY_FLAGS = --quiet -checks=bugprone-*,-bugprone-easily-swappable-parameters,clang-analyzer-*,cert-*,concurrency-*,misc-*,modernize-*,performance-*,readability-* --warnings-as-errors=*
SCAN-BUILD_FLAGS = --status-bugs --keep-cc --show-description
XANALYZER_FLAGS = --analyze -Xanalyzer -analyzer-output=text
SANITIZER_FLAGS = -fsanitize=address

# Cel domyślny
all: program

# Budowanie programu
program: $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^

# Budowanie plików obiektowych
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Puszczanie testów
test: program
	for test in $(TESTS); do \
		./$$test || exit 1; \
	done

# Wykonywanie analizy
analyze: program
	valgrind $(VALGRIND_FLAGS) ./program || exit 1; 
	clang-tidy $(CLANG-TIDY_FLAGS) $(SRCS) || exit 1; 
	scan-build $(SCAN-BUILD_FLAGS) make clean all || exit 1; 
	clang $(XANALYZER_FLAGS) $(SRCS) || exit 1; 
	clang $(SANITIZER_FLAGS) ./program || exit 1; 

# Cel regression
regression: test analyze

# Czyszczenie plików obiektowych i binarnych
clean:
	rm -f $(OBJS) program

.PHONY: all test analyze regression clean