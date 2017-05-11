#include <fluidsynth.h>
#include <fluid_midi.h>
#include <string.h>

#include <gtk/gtk.h>

#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>

/* run by
   gtkkaraoke_player_video_pango /usr/share/sounds/sf2/FluidR3_GM.sf2 /home/newmarch/Music/karaoke/sonken/songs/54154.kar
*/

struct _lyric_t {
    gchar *lyric;
    long tick;

};
typedef struct _lyric_t lyric_t;

struct _lyric_lines_t {
    char *language;
    char *title;
    char *performer;
    GArray *lines; // array of GString *
};
typedef struct _lyric_lines_t lyric_lines_t;

GArray *lyrics;

lyric_lines_t lyric_lines;

typedef struct _coloured_line_t {
    gchar *line;
    gchar *front_of_line;
    gchar *marked_up_line;
    PangoAttrList *attrs;
    GdkPixbuf *pixbuf;
} coloured_line_t;

coloured_line_t coloured_lines[2];

fluid_synth_t* synth;

GtkWidget *image;


int height_lyric_pixbufs[] = {300, 400}; // vertical offset of lyric in video

fluid_player_t* player;

int current_panel = 1;  // panel showing current lyric line
int current_line = 0;  // which line is the current lyric
gchar *current_lyric;   // currently playing lyric line
GString *front_of_lyric;  // part of lyric to be coloured red
//GString *end_of_lyric;    // part of lyrci to not be coloured



// Colours seem to get mixed up when putting a pixbuf onto a pixbuf
#define RED blue

gchar *markup[] = {"<span font=\"28\" foreground=\"RED\">",
		   "</span><span font=\"28\" foreground=\"white\">",
		   "</span>"};
gchar *markup_newline[] = {"<span foreground=\"black\">",
			   "</span>"};
GString *marked_up_label;

/* FFMpeg vbls */
AVFormatContext *pFormatCtx = NULL;
AVCodecContext *pCodecCtx = NULL;
int videoStream;
struct SwsContext *sws_ctx = NULL;
AVCodec *pCodec = NULL;

void markup_line(coloured_line_t *line) {
    GString *str =  g_string_new(markup[0]);
    g_string_append(str, line->front_of_line);
    g_string_append(str, markup[1]);
    g_string_append(str, line->line + strlen(line->front_of_line));
    g_string_append(str, markup[2]);

    line->marked_up_line = str->str;

    pango_parse_markup(str->str, -1,0, &(line->attrs), NULL, NULL, NULL);
    g_string_free(str, FALSE);
}

void update_line_pixbuf(coloured_line_t *line) {
    //return;
    cairo_surface_t *surface;
    cairo_t *cr;
            
    int lyric_width = 480;
    int lyric_height = 60;
    surface = cairo_image_surface_create (CAIRO_FORMAT_ARGB32, 
                                          lyric_width, lyric_height);
    cr = cairo_create (surface);

    PangoLayout *layout;
    PangoFontDescription *desc;
    
    // draw the attributed text
    layout = pango_cairo_create_layout (cr);
    pango_layout_set_text (layout, line->line, -1);
    pango_layout_set_attributes(layout, line->attrs);

    // centre the image in the surface
    int width, height;   
    pango_layout_get_pixel_size(layout,
				&width,
				&height);
    cairo_move_to(cr, (lyric_width-width)/2, 0);    

    pango_cairo_update_layout (cr, layout);
    pango_cairo_show_layout (cr, layout);

    // pull the pixbuf out of the surface
    unsigned char *data = cairo_image_surface_get_data(surface);
    width = cairo_image_surface_get_width(surface);
    height = cairo_image_surface_get_height(surface);
    int stride = cairo_image_surface_get_stride(surface);
    printf("Text surface width %d height %d stride %d\n", width, height, stride);

    GdkPixbuf *old_pixbuf = line->pixbuf;
    line->pixbuf = gdk_pixbuf_new_from_data(data, GDK_COLORSPACE_RGB, 1, 8, width, height, stride, NULL, NULL);
    g_object_unref(layout);
    cairo_surface_destroy(surface);
    g_object_unref(old_pixbuf);
}

/**
 * This MIDI event callback filters out the TEXT and LYRIC events
 * and passes the rest to the default event handler.
  */
int event_callback(void *data, fluid_midi_event_t *event) {
    fluid_synth_t* synth = (fluid_synth_t*) data;
    int type = fluid_midi_event_get_type(event);
    int chan = fluid_midi_event_get_channel(event);
    if (synth == NULL) printf("Synth is null\n");

    switch(type) {
    case MIDI_TEXT:
	printf("Callback: Playing text event %s (length %d)\n", 
	       (char *) event->paramptr, (int) event->param1);

	if (((char *) event->paramptr)[0] == '\\') {
	    int next_panel = current_panel; // really (current_panel+2)%2
	    int next_line = current_line + 2;
	    gchar *next_lyric;

	    if (current_line + 2 >= lyric_lines.lines->len) {
		return FLUID_OK;
	    }
	    current_line += 1;
	    current_panel = (current_panel + 1) % 2;

	    // set up new line as current line
	    char *lyric =  event->paramptr;
	    current_lyric = g_array_index(lyric_lines.lines, GString *, current_line)->str;
	    front_of_lyric = g_string_new(lyric+1); // lose \
	    printf("New line. Setting front to %s end to \"%s\"\n", lyric+1, current_lyric); 

	    coloured_lines[current_panel].line = current_lyric;
	    coloured_lines[current_panel].front_of_line = lyric+1;
	    markup_line(coloured_lines+current_panel);
	    update_line_pixbuf(coloured_lines+current_panel);
	    // update label for next line after this one
	    next_lyric = g_array_index(lyric_lines.lines, GString *, next_line)->str;
	    
	    marked_up_label = g_string_new(markup_newline[0]);

	    g_string_append(marked_up_label, next_lyric);
	    g_string_append(marked_up_label, markup_newline[1]);
	    PangoAttrList *attrs;
	    gchar *text;
	    pango_parse_markup (marked_up_label->str, -1,0, &attrs, &text, NULL, NULL);
	    
	    coloured_lines[next_panel].line = next_lyric;
	    coloured_lines[next_panel].front_of_line = "";
	    markup_line(coloured_lines+next_panel);
	    update_line_pixbuf(coloured_lines+next_panel);
	} else {
	    // change text colour as chars are played
	    char *lyric =  event->paramptr;
	    if ((front_of_lyric != NULL) && (lyric != NULL)) {
		g_string_append(front_of_lyric, lyric);
		char *s = front_of_lyric->str;
		coloured_lines[current_panel].front_of_line = s;
		markup_line(coloured_lines+current_panel);
		update_line_pixbuf(coloured_lines+current_panel);
	    }
	}
	return  FLUID_OK;

    case MIDI_LYRIC:
	printf("Callback: Playing lyric event %d %s\n", (int) event->param1, (char *) event->paramptr);
	return  FLUID_OK;
    
    case MIDI_EOT:
	printf("End of track\n");
	exit(0);
    }
    return fluid_synth_handle_midi_event( data, event);
}

void build_lyric_lines() {
    int n;
    lyric_t *plyric;
    GString *line = g_string_new("");
    GArray *lines =  g_array_sized_new(FALSE, FALSE, sizeof(GString *), 64);

    lyric_lines.title = NULL;

    for (n = 0; n < lyrics->len; n++) {
	plyric = g_array_index(lyrics, lyric_t *, n);
	gchar *lyric = plyric->lyric;
	int tick = plyric->tick;

	if ((strlen(lyric) >= 2) && (lyric[0] == '@') && (lyric[1] == 'L')) {
	    lyric_lines.language =  lyric + 2;
	    continue;
	}

	if ((strlen(lyric) >= 2) && (lyric[0] == '@') && (lyric[1] == 'T')) {
	    if (lyric_lines.title == NULL) {
		lyric_lines.title = lyric + 2;
	    } else {
		lyric_lines.performer = lyric + 2;
	    }
	    continue;
	}

	if (lyric[0] == '@') {
	    // some other stuff like @KMIDI KARAOKE FILE
	    continue;
	}

	if ((lyric[0] == '/') || (lyric[0] == '\\')) {
	    // start of a new line
	    // add to lines
	    g_array_append_val(lines, line);
	    line = g_string_new(lyric + 1);
	}  else {
	    line = g_string_append(line, lyric);
	}
    }
    lyric_lines.lines = lines;
    
    printf("Title is %s, performer is %s, language is %s\n", 
	   lyric_lines.title, lyric_lines.performer, lyric_lines.language);
    for (n = 0; n < lines->len; n++) {
	printf("Line is %s\n", g_array_index(lines, GString *, n)->str);
    }
    
}


/**
 * This is called whenever new data is loaded, such as a new file.
 * Here we extract the TEXT and LYRIC events and just print them
 * to stdout. They could e.g. be saved and displayed in a GUI
 * as the events are received by the event callback.
 */ 
int onload_callback(void *data, fluid_player_t *player) {
    long ticks = 0L;
    lyric_t *plyric;

    printf("Load callback, tracks %d \n", player->ntracks);
    int n;
    for (n = 0; n < player->ntracks; n++) {
	fluid_track_t *track = player->track[n];
	printf("Track %d\n", n);
	fluid_midi_event_t *event = fluid_track_first_event(track);
	while (event != NULL) {
	    switch (fluid_midi_event_get_type (event)) {
	    case MIDI_TEXT:
	    case MIDI_LYRIC:
		/* there's no fluid_midi_event_get_sysex()
		   or fluid_midi_event_get_time() so we 
		   have to look inside the opaque struct
		*/
		ticks += event->dtime;
		printf("Loaded event %s for time %ld\n", 
		       (char *) event->paramptr,
		       ticks);
		plyric = g_new(lyric_t, 1);
		plyric->lyric = g_strdup(event->paramptr);
		plyric->tick = ticks;
		g_array_append_val(lyrics, plyric);
	    }
	    event = fluid_track_next_event(track);
	}
    }

    printf("Saved %d lyric events\n", lyrics->len);
    for (n = 0; n < lyrics->len; n++) {
	plyric = g_array_index(lyrics, lyric_t *, n);
	printf("Saved lyric %s at %ld\n", plyric->lyric, plyric->tick);
    }

    build_lyric_lines();
    
    return FLUID_OK;
}

static void pixmap_destroy_notify(guchar *pixels,
				  gpointer data) {
    printf("Ddestroy pixmap\n");
}

static gboolean draw_image(gpointer user_data) {
    GdkPixbuf *pixbuf = (GdkPixbuf *) user_data;

    gtk_image_set_from_pixbuf((GtkImage *) image, pixbuf);
    gtk_widget_queue_draw(image);
    g_object_unref(pixbuf);
    
    return G_SOURCE_REMOVE;
}

static void *play_background(void *args) {

    int i;
    AVPacket packet;
    int frameFinished;
    AVFrame *pFrame = NULL;

    int oldSize;
    char *oldData;
    int bytesDecoded;
    GdkPixbuf *pixbuf;
    AVFrame *picture_RGB;
    char *buffer;

    pFrame=avcodec_alloc_frame();

    i=0;
    picture_RGB = avcodec_alloc_frame();
    buffer = malloc (avpicture_get_size(PIX_FMT_RGB24, 720, 576));
    avpicture_fill((AVPicture *)picture_RGB, buffer, PIX_FMT_RGB24, 720, 576);

    while(av_read_frame(pFormatCtx, &packet)>=0) {
	if(packet.stream_index==videoStream) {
	    //printf("Frame %d\n", i++);
	    usleep(33670);  // 29.7 frames per second
	    // Decode video frame
	    avcodec_decode_video2(pCodecCtx, pFrame, &frameFinished,
				  &packet);
	    int width = pCodecCtx->width;
	    int height = pCodecCtx->height;
	    
	    sws_ctx = sws_getContext(pCodecCtx->width, pCodecCtx->height, pCodecCtx->pix_fmt, pCodecCtx->width, pCodecCtx->height, PIX_FMT_RGB24, SWS_BICUBIC, NULL, NULL, NULL);

	    if (frameFinished) {
		printf("Frame %d\n", i++);
		
		sws_scale(sws_ctx,  (uint8_t const * const *) pFrame->data, pFrame->linesize, 0, pCodecCtx->height, picture_RGB->data, picture_RGB->linesize);
		

		pixbuf = gdk_pixbuf_new_from_data(picture_RGB->data[0], GDK_COLORSPACE_RGB, 0, 8, 720, 480, picture_RGB->linesize[0], pixmap_destroy_notify, NULL);

                // Create the destination surface
                cairo_surface_t *surface = cairo_image_surface_create (CAIRO_FORMAT_ARGB32, 
                                                                       width, height);
                cairo_t *cr = cairo_create(surface);

                // draw the background image
                gdk_cairo_set_source_pixbuf(cr, pixbuf, 0, 0);
                cairo_paint (cr);

		// draw the lyric
		GdkPixbuf *lyric_pixbuf = coloured_lines[current_panel].pixbuf;
		if (lyric_pixbuf != NULL) {
		    int width = gdk_pixbuf_get_width(lyric_pixbuf);
		    gdk_cairo_set_source_pixbuf(cr, 
						lyric_pixbuf, 
						(720-width)/2, 
						 height_lyric_pixbufs[current_panel]);
		    cairo_paint (cr);
		}

		int next_panel = (current_panel+1) % 2;
		lyric_pixbuf = coloured_lines[next_panel].pixbuf;
		if (lyric_pixbuf != NULL) {
		    int width = gdk_pixbuf_get_width(lyric_pixbuf);
		    gdk_cairo_set_source_pixbuf(cr, 
						lyric_pixbuf, 
						(720-width)/2, 
						 height_lyric_pixbufs[next_panel]);
		    cairo_paint (cr);
		}

		pixbuf = gdk_pixbuf_get_from_surface(surface,
						     0,
						     0,
						     width,
						     height);

		gdk_threads_add_idle(draw_image, pixbuf);

	       	/* reclaim memory */
		sws_freeContext(sws_ctx);
		cairo_surface_destroy(surface);
		cairo_destroy(cr);
	    }
	}
	av_free_packet(&packet);
    }
    printf("Video over!\n");
    exit(0);
}

static void *play_midi(void *args) {
    fluid_player_play(player);

    printf("Audio finished\n");
    //exit(0);
}


/* Called when the windows are realized
 */
static void realize_cb (GtkWidget *widget, gpointer data) {
    /* start the video playing in its own thread */
    pthread_t tid;
    pthread_create(&tid, NULL, play_background, NULL);

    /* start the MIDI file playing in its own thread */
    pthread_t tid_midi;
    pthread_create(&tid_midi, NULL, play_midi, NULL);
}

static gboolean delete_event( GtkWidget *widget,
                              GdkEvent  *event,
                              gpointer   data )
{
    /* If you return FALSE in the "delete-event" signal handler,
     * GTK will emit the "destroy" signal. Returning TRUE means
     * you don't want the window to be destroyed.
     * This is useful for popping up 'are you sure you want to quit?'
     * type dialogs. */

    g_print ("delete event occurred\n");

    /* Change TRUE to FALSE and the main window will be destroyed with
     * a "delete-event". */

    return TRUE;
}

/* Another callback */
static void destroy( GtkWidget *widget,
                     gpointer   data )
{
    gtk_main_quit ();
}

int main(int argc, char** argv)
{
    XInitThreads();

    int i;

    fluid_settings_t* settings;

    fluid_audio_driver_t* adriver;
    settings = new_fluid_settings();
    fluid_settings_setstr(settings, "audio.driver", "alsa");
    //fluid_settings_setint(settings, "lash.enable", 0);
    fluid_settings_setint(settings, "synth.polyphony", 64);
    fluid_settings_setint(settings, "synth.reverb.active", FALSE);
    fluid_settings_setint(settings, "synth.sample-rate", 22050);
    synth = new_fluid_synth(settings);
    player = new_fluid_player(synth);

    lyrics = g_array_sized_new(FALSE, FALSE, sizeof(lyric_t *), 1024);

    /* Set the MIDI event callback to our own functions rather than the system default */
    fluid_player_set_playback_callback(player, event_callback, synth);

    /* Add an onload callback so we can get information from new data before it plays */
    fluid_player_set_onload_callback(player, onload_callback, NULL);
    
    adriver = new_fluid_audio_driver(settings, synth);
  
    /* process command line arguments */
    for (i = 1; i < argc; i++) {
        if (fluid_is_soundfont(argv[i])) {
            fluid_synth_sfload(synth, argv[1], 1);
        } else {
            fluid_player_add(player, argv[i]);
        }
    }

    /* FFMpeg stuff */

    AVFrame *pFrame = NULL;
    AVPacket packet;

    AVDictionary *optionsDict = NULL;

    av_register_all();

    if(avformat_open_input(&pFormatCtx, "short.mpg", NULL, NULL)!=0) {
	printf("Couldn't open video file\n");
	return -1; // Couldn't open file
    }
  
    // Retrieve stream information
    if(avformat_find_stream_info(pFormatCtx, NULL)<0) {
	printf("Couldn't find stream information\n");
	return -1; // Couldn't find stream information
    }

    // Dump information about file onto standard error
    av_dump_format(pFormatCtx, 0, argv[1], 0);
  
    // Find the first video stream
    videoStream=-1;
    for(i=0; i<pFormatCtx->nb_streams; i++)
	if(pFormatCtx->streams[i]->codec->codec_type==AVMEDIA_TYPE_VIDEO) {
	    videoStream=i;
	    break;
	}
    if(videoStream==-1)
	return -1; // Didn't find a video stream

    for(i=0; i<pFormatCtx->nb_streams; i++)
	if(pFormatCtx->streams[i]->codec->codec_type==AVMEDIA_TYPE_AUDIO) {
	    printf("Found an audio stream too\n");
	    break;
	}
 
    // Get a pointer to the codec context for the video stream
    pCodecCtx=pFormatCtx->streams[videoStream]->codec;
  
    // Find the decoder for the video stream
    pCodec=avcodec_find_decoder(pCodecCtx->codec_id);
    if(pCodec==NULL) {
	fprintf(stderr, "Unsupported codec!\n");
	return -1; // Codec not found
    }
  
    // Open codec
    if(avcodec_open2(pCodecCtx, pCodec, &optionsDict)<0) {
	printf("Could not open codec\n");
	return -1; // Could not open codec
    }

    sws_ctx =
	sws_getContext
	(
	 pCodecCtx->width,
	 pCodecCtx->height,
	 pCodecCtx->pix_fmt,
	 pCodecCtx->width,
	 pCodecCtx->height,
	 PIX_FMT_YUV420P,
	 SWS_BILINEAR,
	 NULL,
	 NULL,
	 NULL
	 );

    /* GTK stuff now */

    /* GtkWidget is the storage type for widgets */
    GtkWidget *window;
    GtkWidget *button;
    GtkWidget *lyrics_box;

    
    /* This is called in all GTK applications. Arguments are parsed
     * from the command line and are returned to the application. */
    gtk_init (&argc, &argv);
    
    /* create a new window */
    window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
    
    /* When the window is given the "delete-event" signal (this is given
     * by the window manager, usually by the "close" option, or on the
     * titlebar), we ask it to call the delete_event () function
     * as defined above. The data passed to the callback
     * function is NULL and is ignored in the callback function. */
    g_signal_connect (window, "delete-event",
		      G_CALLBACK (delete_event), NULL);
    
    /* Here we connect the "destroy" event to a signal handler.  
     * This event occurs when we call gtk_widget_destroy() on the window,
     * or if we return FALSE in the "delete-event" callback. */
    g_signal_connect (window, "destroy",
		      G_CALLBACK (destroy), NULL);

    g_signal_connect (window, "realize", G_CALLBACK (realize_cb), NULL);
    
    /* Sets the border width of the window. */
    gtk_container_set_border_width (GTK_CONTAINER (window), 10);

    lyrics_box = gtk_vbox_new(TRUE, 1);
    gtk_widget_show(lyrics_box);

    /*
    char *str = "     ";
    lyric_labels[0] = gtk_label_new(str);
    str =  "World";
    lyric_labels[1] = gtk_label_new(str);
    */

    image = gtk_image_new();

    //image_drawable = gtk_drawing_area_new();
    //gtk_widget_set_size_request (canvas, 720, 480);
    //gtk_drawing_area_size((GtkDrawingArea *) image_drawable, 720, 480);

    //gtk_widget_show (lyric_labels[0]);
    //gtk_widget_show (lyric_labels[1]);

    gtk_widget_show (image);
    
    //gtk_box_pack_start (GTK_BOX (lyrics_box), lyric_labels[0], TRUE, TRUE, 0);
    //gtk_box_pack_start (GTK_BOX (lyrics_box), lyric_labels[1], TRUE, TRUE, 0);
    gtk_box_pack_start (GTK_BOX (lyrics_box), image, TRUE, TRUE, 0);
    //gtk_box_pack_start (GTK_BOX (lyrics_box), canvas, TRUE, TRUE, 0);
    //gtk_box_pack_start (GTK_BOX (lyrics_box), image_drawable, TRUE, TRUE, 0);
    
    /* This packs the button into the window (a gtk container). */
    gtk_container_add (GTK_CONTAINER (window), lyrics_box);
        
    /* and the window */
    gtk_widget_show (window);
    
    /* All GTK applications must have a gtk_main(). Control ends here
     * and waits for an event to occur (like a key press or
     * mouse event). */
    gtk_main ();
    
    return 0;

    /* wait for playback termination */
    fluid_player_join(player);
    /* cleanup */
    delete_fluid_audio_driver(adriver);
    delete_fluid_player(player);
    delete_fluid_synth(synth);
    delete_fluid_settings(settings);

    return 0;
}

