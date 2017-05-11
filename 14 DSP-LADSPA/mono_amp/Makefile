
CFLAGS = -g
LIBS = -lsndfile -ldl

mono_amp: mono_amp.c load.c
	cc $(CFLAGS) -o mono_amp mono_amp.c load.c $(LIBS)
