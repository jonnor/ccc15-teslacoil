
BOARD=nano328
BAUD=115200
PORT=/dev/ttyUSB0
MIDIFILE=./midi/beethoven_fifth_op67.mid

ttymidi:
	./ttymidi/ttymidi -s ${PORT} --baudrate ${BAUD}

play:
	aplaymidi ${MIDIFILE} --port ttymidi:1

check:
	g++ -o test/test -I./src test/test.cpp -Wall -Werror && ./test/test

upload:
	ino build -m ${BOARD} && ino upload -m ${BOARD}

all: check

.PHONY: ttymidi
