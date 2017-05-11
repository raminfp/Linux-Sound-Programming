
LDLIBS = $(shell pkg-config --libs alsa) -lm

CFLAGS = -g $(shell pkg-config --cflags alsa)

SRC = \
	aplay-l.c \
	aplay-L.c \
	device-info.c \
	alsa_capture.c \
	alsa_playback.c \
	playback-capture.c \
	volume.c
EXE = 	\
	aplay-l \
	aplay-L \
	device-info \
	alsa_capture \
	alsa_playback \
	playback-capture \
	volume

all: $(EXE)


clean:
	rm $(EXE)

zip:
	zip all.zip Makefile $(SRC)
