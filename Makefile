
check:
	g++ -o test/test -I./src test/test.cpp -Wall -Werror && ./test/test

upload:
	ino build -m leonardo && ino upload -m leonardo

all: check
