
#LDFLAGS :=   -ljack -lasound -lm -lpthread -lsndfile
#CFLAGS_INC := -Ijack-1.9.8.ok/jack-1.9.8/common/ -Ijack-1.9.8.ok/jack-1.9.8/common/jack/ -Ijack-1.9.8.ok/jack-1.9.8/windows/
#CFLAGS := -g -Wall $(CFLAGS_INC)

LDLIBS = -g $(shell pkg-config --libs jack)
INCLUDES = $(shell pkg-config --cflags jack)

SRC = \
	delay.c

EXE = \
	delay

all: $(EXE)

clean:
	rm $(EXE)

zip:
	zip all.zip Makefile $(SRC)

