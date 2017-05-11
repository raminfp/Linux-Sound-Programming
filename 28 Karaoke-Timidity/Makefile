
V = 3

T = ../../MIDI/Timidity/TiMidity++-2.14.0.played_with

LIBS =  -lasound -l glib-2.0 $(shell pkg-config --libs gtk+-$(V).0  libavformat libavcodec libavutil libswscale) -lpthread -lX11 
# INCLUDES = -I fluidsynth-1.1.6-new/include/ -I fluidsynth-1.1.6-new/src/midi/ -I fluidsynth-1.1.6-new/src/utils/ -I/usr/include/glib-2.0 -I/usr/lib64/glib-2.0/include -I/usr/include/gtk-2.0/ -I/usr/include/cairo -I/usr/include/pango-1.0/ -I/usr/lib64/gtk-2.0/include -I/usr/include/gdk-pixbuf-2.0/ -I/usr/include/atk-1.0

LIBS_STATIC =  $(T)/libtimidity.a -lm $(LIBS)

INCLUDES =  $(shell pkg-config --cflags gtk+-$(V).0 libavformat libavcodec libswscale libavutil )  -I/home/httpd/html/LinuxSound/MIDI/Timidity/TiMidity++-2.14.0/timidity/ -I/home/httpd/html/LinuxSound/MIDI/Timidity/TiMidity++-2.14.0/utils

CFLAGS = -g $(INCLUDES)

EXE:    \
	if_x.so \
	if_v.so

	#gtkkaraoke \
	#gtkkaraoke_player_video_pango \
	#gtkkaraoke_player_pango \
	#gtkkaraoke_player_video_pango_audio \
	#ok \
	#if_video.so \
	#if_overlay.so \


all: $(EXE) zip

if_v.so: video_code.c
	gcc  -fPIC $(CFLAGS) -c -o video_code.o video_code.c $(LIBS)
	gcc -shared -o if_v.so video_code.o $(LIBS)

if_overlay.so: overlay_code.c
	gcc  -fPIC $(CFLAGS) -c -o overlay_code.o overlay_code.c $(LIBS)
	gcc -shared -o if_overlay.so overlay_code.o $(LIBS)

if_x.so: x_code.c
	gcc  -fPIC $(CFLAGS) -c -o x_code.o x_code.c $(LIBS)
	gcc -shared -o if_x.so x_code.o $(LIBS)

gtkkaraoke: gtkkaraoke.o
	gcc gtkkaraoke.o $(LIBS) -o gtkkaraoke

ok: ok.o
	gcc ok.o $(LIBS) -o ok

gtkkaraoke_player_video_pango: gtkkaraoke_player_video_pango.o video_code.o
	gcc gtkkaraoke_player_video_pango.o video_code.o $(LIBS_STATIC) -o gtkkaraoke_player_video_pango

gtkkaraoke_player_pango: gtkkaraoke_player_video_pango.o overlay_code.o
	gcc gtkkaraoke_player_video_pango.o overlay_code.o $(LIBS_STATIC) -o gtkkaraoke_player_pango

gtkkaraoke_player_video_pango_audio: play_audio.o gtkkaraoke_player_video_pango_audio.o
	gcc gtkkaraoke_player_video_pango_audio.o play_audio.o $(LIBS_STATIC) -o gtkkaraoke_player_video_pango_audio

zip:
	zip all.zip *.h *.c Makefile

clean:
	rm gtkkaraoke gtkkaraoke_player_video_pango *.o *.so
