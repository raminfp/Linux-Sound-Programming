
LIBS = -L fluidsynth-1.1.6-new/src/.libs -l fluidsynth
INCLUDES = -I fluidsynth-1.1.6-new/include/ -I fluidsynth-1.1.6-new/src/midi/ -I fluidsynth-1.1.6-new/src/utils/ -I/usr/include/glib-2.0 -I/usr/lib64/glib-2.0/include

CFLAGS = -g $(INCLUDES)

EXE:    \
	file_player

all: $(EXE)


file_player: file_player.o
	gcc $(LIBS) -o file_player file_player.o

clean:
	rm $(EXE) *.o
