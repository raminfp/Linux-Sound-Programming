# Requires libavcodec-dev libavformat-dev

# LIBS = -I/usr/include/ffmpeg/ -lavformat -lavcodec -lavutil

CFLAGS = -g $(shell pkg-config --cflags libavcodec libavformat libavutil ao)
LDLIBS = $(shell pkg-config --libs libavcodec libavformat libavutil ao) -lm

SRC = api-example.c

EXE = api-example


all: $(EXE)

clean:
	rm $(EXE)

zip:
	zip all.zip Makefile $(SRC) Beethoven_Fr_Elise.mp3
