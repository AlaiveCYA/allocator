# Definiowanie kompilatora i flag
CC = gcc
CFLAGS = -Wall -Wextra -g -std=c17 -D_XOPEN_SOURCE=500 -ftest-coverage -fprofile-arcs
DIR=$(shell pwd)

# Definiowanie plików źródłowych i obiektowych
SRCS = $(wildcard test/*.c) $(wildcard src/*.c)
OBJS = $(SRCS:.c=.o)
HEADERS = $(wildcard src/*.h)

# Definiowanie narzędzi do analizy
VALGRIND_FLAGS = --leak-check=full --show-leak-kinds=all --track-origins=yes --trace-children=yes --error-exitcode=1
CLANG-TIDY_FLAGS = --quiet -checks=bugprone-*,-bugprone-easily-swappable-parameters,clang-analyzer-*,cert-*,concurrency-*,misc-*,modernize-*,performance-*,readability-* --warnings-as-errors=*
SCAN-BUILD_FLAGS = --status-bugs --keep-cc --show-description
XANALYZER_FLAGS = --analyze -Xanalyzer -analyzer-output=text

# Cel domyślny
all: regression clean

# Budowanie programu
program: $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^

# Budowanie plików obiektowych
%.o: %.c $(HEADERS)
	$(CC) $(CFLAGS) -c $< -o $@

# Puszczanie testów
test: program
	./program || exit 1;

# Wykonywanie analizy
analyze: program
	gcov $(SRCS) || exit 1;
	valgrind $(VALGRIND_FLAGS) ./program || exit 1; 
	clang-tidy $(CLANG-TIDY_FLAGS) $(SRCS) || exit 1; 
	scan-build $(SCAN-BUILD_FLAGS) make test || exit 1; 
	clang $(XANALYZER_FLAGS) $(SRCS) || exit 1; 
	clang -fsanitize=address $(SRCS) || exit 1;
	clang -fsanitize=memory $(SRCS) || exit 1;
	clang -fsanitize=undefined $(SRCS) || exit 1; 

# Cel regression
regression: test analyze

# Czyszczenie plików obiektowych i binarnych
clean:
	rm -f $(OBJS) src/*.gcda src/*.gcno test/*.gcda test/*.gcno a.out *.gcov program

install:
	sudo chmod +x ./install_env.sh
	sudo chmod +x ./install_lib.sh
	./install_env.sh
	./install_lib.sh $(DIR)

uninstall:
	sudo chmod +x ./uninstall_lib.sh
	./uninstall_lib.sh

.PHONY: all test analyze regression clean install uninstall