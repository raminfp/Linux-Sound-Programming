
#LDFLAGS :=   -ljack -lasound -lm -lpthread -lsndfile
#CFLAGS_INC := -Ijack-1.9.8.ok/jack-1.9.8/common/ -Ijack-1.9.8.ok/jack-1.9.8/common/jack/ -Ijack-1.9.8.ok/jack-1.9.8/windows/
#CFLAGS := -g -Wall $(CFLAGS_INC)

# Requires libjack-jack2-dev libsndfile1-dev

LDLIBS = -g $(shell pkg-config --libs jack sndfile) -lpthread -lm
INCLUDES = $(shell pkg-config --cflags jack sndfile)

SRC = \
	listports.c \
	thru_client.c \
	delay.c \
	simple_client.c \
	capture_client.c \
	simple_client.c

EXE = \
	listports \
	thru_client \
	delay \
	simple_client \
	capture_client \
	simple_client

all: $(EXE)

clean:
	rm $(EXE)

zip:
	zip all.zip Makefile $(SRC)

