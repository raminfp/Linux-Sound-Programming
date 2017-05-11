
V = 3

LIBS = -l glib-2.0 $(shell pkg-config --libs gtk+-$(V).0)  -lsndfile -ldl -lpthread -lX11 

INCLUDES =  $(shell pkg-config --cflags gtk+-$(V).0 libavformat libavcodec libswscale libavutil ) 

CFLAGS = -g $(INCLUDES)

EXE:    \
	stereo_amp

all: $(EXE)


stereo_amp: stereo_amp.o load.o
	gcc -o stereo_amp stereo_amp.o load.o $(LIBS)



clean:
	rm $(EXE) *.o
