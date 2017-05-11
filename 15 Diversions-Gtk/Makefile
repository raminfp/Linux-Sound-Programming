
V = 3

LDLIBS = -l glib-2.0 $(shell pkg-config --libs gtk+-$(V).0  libavformat libavcodec libavutil libswscale) -lpthread -lX11 

INCLUDES =  $(shell pkg-config --cflags gtk+-$(V).0 libavformat libavcodec libswscale libavutil ) 

CFLAGS = -g $(INCLUDES)

SRC = \
	updown.c \
	play_video.c \
	display_image.c \
	gtk_play_video.c \
	gtk_play_video_overlay.c \
	gtk_play_video_overlay_alpha.c \
	gtk_play_video_cairo.c \
	gtk_play_video_pango.c


EXE =    \
	updown \
	play_video \
	display_image \
	gtk_play_video \
	gtk_play_video_overlay \
	gtk_play_video_overlay_alpha \
	gtk_play_video_cairo \
	gtk_play_video_pango \

all: $(EXE)

zip:
	zip Makefile all.zip $(SRC)

clean:
	rm $(EXE) *.o
