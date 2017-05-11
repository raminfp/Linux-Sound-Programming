
INCLUDES=-I ../../lim-omx-1.1/LIM/limoi-core/include/
#LIBS=-L ../../lim-omx-1.1/LIM/limoi-base/src/.libs -l limoi-base
LIBS = -L /home/newmarch/osm-build/lib/ -l limoa -l limoi-core -l limutil
CFLAGS = -g -DLIM -Werror

all: info render record decode_ogg play_ogg listcomponents

volume: volume.c
	cc $(INCLUDES) -o volume volume.c $(LIBS)

listcomponents: listcomponents.c
	cc $(CFLAGS) $(INCLUDES) -o listcomponents listcomponents.c $(LIBS)

info: info.c
	cc $(CFLAGS) $(INCLUDES) -o info info.c $(LIBS)

render: render.c
	cc $(CFLAGS) $(INCLUDES) -o render render.c $(LIBS)

record: record.c
	cc $(CFLAGS) $(INCLUDES) -o record record.c $(LIBS)

decode_ogg: decode_ogg.c
	cc $(CFLAGS) $(INCLUDES) -o decode_ogg decode_ogg.c $(LIBS)

play_ogg: play_ogg.c
	cc $(CFLAGS) $(INCLUDES) -o play_ogg play_ogg.c $(LIBS)

omxvolcontroltest: omxvolcontroltest.c
	cc $(INCLUDES) -o omxvolcontroltest omxvolcontroltest.c $(LIBS)

clean:
	rm volume info render record listcomponents decode_ogg
