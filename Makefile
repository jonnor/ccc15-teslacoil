
check:
	g++ -o test/test -I./src test/test.cpp -Wall -Werror && ./test/test

all: check
