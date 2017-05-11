
INCLUDES=-I ../libomxil-bellagio-0.9.3/include/
LIBS=-L ../libomxil-bellagio-0.9.3/src/.libs -l omxil-bellagio
CFLAGS = -g

all: volume info render record ogg_decode listcomponents

volume: volume.c
	cc $(INCLUDES) -o volume volume.c $(LIBS)

listcomponents: listcomponents.c
	cc $(FLAGS) $(INCLUDES) -o listcomponents listcomponents.c $(LIBS)

info: info.c
	cc $(FLAGS) $(INCLUDES) -o info info.c $(LIBS)

render: render.c
	cc $(FLAGS) $(INCLUDES) -o render render.c $(LIBS)

record: record.c
	cc $(FLAGS) $(INCLUDES) -o record record.c $(LIBS)

ogg_decode: ogg_decode.c
	cc $(FLAGS) $(INCLUDES) -o ogg_decode ogg_decode.c $(LIBS)

omxvolcontroltest: omxvolcontroltest.c
	cc $(INCLUDES) -o omxvolcontroltest omxvolcontroltest.c $(LIBS)

clean:
	rm volume info render record listcomponents ogg_decode
