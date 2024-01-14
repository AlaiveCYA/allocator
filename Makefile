# Definiowanie kompilatora i flag
CC = gcc
CFLAGS = -Wall -Wextra -g -std=c17 -D_XOPEN_SOURCE=500
DIR=$(shell pwd)

# Definiowanie plików źródłowych i obiektowych
SRC = $(wildcard src/*.c)
UNIT_TEST_SRC = test/unittestmylloc.c $(SRC)
UNIT_OBJS = $(UNIT_TEST_SRC:.c=.o)
OBJS = $(SRC:.c=.o)
HEADERS = $(wildcard src/*.h)

# Definiowanie narzędzi do analizy
VALGRIND_FLAGS = --leak-check=full --show-leak-kinds=all --track-origins=yes --trace-children=yes --error-exitcode=1
CLANG-TIDY_FLAGS = --quiet -checks=bugprone-*,-bugprone-easily-swappable-parameters,clang-analyzer-*,cert-*,concurrency-*,misc-*,modernize-*,performance-*,readability-* --warnings-as-errors=*
SCAN-BUILD_FLAGS = --status-bugs --keep-cc --show-description
XANALYZER_FLAGS = --analyze -Xanalyzer -analyzer-output=text

# Cel domyślny
all: regression clean

# Budowanie programu
unit_test: $(UNIT_OBJS) 
	$(CC) $(CFLAGS) -fprofile-arcs -ftest-coverage $^ -o program

e2e_test: $(OBJS) test/e2etest_correct.o test/e2etest_faulty.o
	$(CC) $(CFLAGS) test/e2etest_correct.o $(OBJS) -o e2etest_correct
	$(CC) $(CFLAGS) test/e2etest_faulty.o $(OBJS) -o e2etest_faulty

# Budowanie plików obiektowych
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Puszczanie testów
test: unit_test e2e_test
	./program || exit 1;
	python3 test/e2etest.py || exit 1;
	


# Wykonywanie analizy
analyze: program
	gcov $(UNIT_TEST_SRC) || exit 1;
	valgrind $(VALGRIND_FLAGS) ./program || exit 1; 
	clang-tidy $(CLANG-TIDY_FLAGS) $(UNIT_TEST_SRC) || exit 1; 
	scan-build $(SCAN-BUILD_FLAGS) make test || exit 1; 
	clang $(XANALYZER_FLAGS) $(UNIT_TEST_SRC) || exit 1; 
	clang -fsanitize=address $(UNIT_TEST_SRC) || exit 1;
	clang -fsanitize=memory $(UNIT_TEST_SRC) || exit 1;
	clang -fsanitize=undefined $(UNIT_TEST_SRC) || exit 1; 

# Cel regression
regression: test analyze

# Czyszczenie plików obiektowych i binarnych
clean:
	rm -f $(UNIT_OBJS) $(E2E_OBJS) test/e2etest_correct.o test/e2etest_faulty.o src/*.gcda src/*.gcno test/*.gcda test/*.gcno a.out *.gcov program e2etest_correct e2etest_faulty

install:
	sudo chmod +x ./install_env.sh
	sudo chmod +x ./install_lib.sh
	./install_env.sh
	./install_lib.sh $(DIR)

.PHONY: all test analyze regression clean install