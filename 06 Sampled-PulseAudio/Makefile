
# Requires libpulse-dev to be installed

LDLIBS = $(shell pkg-config --libs libpulse-simple)
# LIBS = -lpulse-simple -lpulse -lpulsecommon -lsndfile
CFLAGS = -g $(shell pkg-config --cflags libpulse-simple)

SRC =	\
	pacat-simple.c \
	parec-simple.c \
	pa-mic-2-speaker-simple.c \
	palist_devices.c \
	palist_devices_ongoing.c \
	parec.c \
	pacat2.c \
	pacat.c \
	parec-latency.c \
	pa-mic-2-speaker.c \
	pavolume.c \
	palist_clients.c \
	pamonitor_clients.c \
	pavolume_sink.c

EXE =	\
	pacat-simple \
	parec-simple \
	pa-mic-2-speaker-simple \
	palist_devices \
	palist_devices_ongoing \
	parec \
	pacat2 \
	pacat \
	parec-latency \
	pa-mic-2-speaker \
	pavolume \
	palist_clients \
	pamonitor_clients \
	pavolume_sink 


all: $(EXE)


clean:
	rm $(EXE)

zip:
	zip all.zip Makefile $(SRC)
