# Requires libao-dev

LDLIBS = $(shell pkg-config --libs ao) -lm
CFLAGS = -g $(shell pkg-config --cflags ao)

SRC = ao_example.c

EXE = ao_example

all: $(EXE)

clean:
	rm $(EXE)

zip:
	zip all.zip Makefile $(SRC)
