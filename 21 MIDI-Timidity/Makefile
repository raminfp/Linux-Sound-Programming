
INCLUDES2 = -ITiMidity++-2.14.0  $(shell pkg-config --cflags gtk+-2.0 libavformat libavcodec libswscale libavutil ) 

INCLUDES3 = -ITiMidity++-2.14.0/timidity -ITiMidity++-2.14.0 -ITiMidity++-2.14.0/utils  $(shell pkg-config --cflags gtk+-3.0 libavformat libavcodec libswscale libavutil ) 

LIBS2 =  $(shell pkg-config --libs gtk+-2.0 libavformat libavcodec libswscale libavutil )

LIBS3 =  $(shell pkg-config --libs gtk+-3.0 libavformat libavcodec libswscale libavutil )

#-I/usr/include/gtk-3.0/ -I/usr/include/glib-2.0 \
#	-I/usr/lib/x86_64-linux-gnu/glib-2.0/include -I/usr/include/pango-1.0 \
#	-I/usr/include/cairo/ -I/usr/include/gdk-pixbuf-2.0 \
#	-I/usr/include/atk-1.0

# T = TiMidity++-2.14.0.played_with
T = TiMidity++-2.14.0.played_with

CFLAGS = -g $(INCLUDES3)

SRCS  := $(wildcard *.c)

all: my_timidity video_player draw_image  if_my_interface.so if_video_player.so zip


my_timidity: my_main.o
	gcc -g -o my_timidity my_main.o $(T)/libtimidity.a -lasound -lm

if_my_interface.so: my_interface.c
	gcc  -fPIC $(CFLAGS) -c -o my_interface.o my_interface.c
	gcc -shared  -L./lib/termcap  -o if_my_interface.so my_interface.o

video_code.o: video_code.c
	gcc  -fPIC $(CFLAGS) -c -o video_code.o video_code.c $(LIBS3)

if_video_player.so: video_player_interface.c video_code.o
	gcc  -fPIC $(CFLAGS) -c -o video_player_interface.o video_player_interface.c
	gcc -shared -o if_video_player.so video_player_interface.o video_code.o $(LIBS3)


video_player: video_player.o video_code.o
	gcc $(INCLUDES3) -g -o video_player video_code.o video_player.o $(LIBS3) $(T)/libtimidity.a -lasound -lm -lX11

thread_eg: thread_eg.o
	gcc $(INCLUDES2) -g -o thread_eg thread_eg.o $(LIBS2) -lm -lX11

video_player3: video_player3.o
	gcc $(INCLUDES2) -g -o video_player3 video_player3.o $(LIBS2) $(T)/libtimidity.a -lasound -lm -lX11

video_player3a: video_player3a.o
	gcc $(INCLUDES2) -g -o video_player3a video_player3a.o $(LIBS2) $(T)/libtimidity.a -lasound -lm -lX11

draw_image: draw_image.o
	gcc $(INCLUDES3) -g -o draw_image draw_image.o $(LIBS3) $(T)/libtimidity.a -lasound -lm -lX11

zip:
	zip all.zip $(SRCS)

clean:
	rm *.o *.so
