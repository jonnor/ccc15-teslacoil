
BOARD=uno328

check:
	g++ -o test/test -I./src test/test.cpp -Wall -Werror && ./test/test

upload:
	ino build -m ${BOARD} && ino upload -m ${BOARD}

all: check
