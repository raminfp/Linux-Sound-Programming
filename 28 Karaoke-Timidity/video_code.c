
#include <gtk/gtk.h>

#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>

#include "mytimidity.h"

#define USE_PIXBUF

// saving as pixbufs leaks memory
#define USE_PIXBUF

/* run by
   gtkkaraoke_player_video_pango /usr/share/sounds/sf2/FluidR3_GM.sf2 /home/newmarch/Music/karaoke/sonken/songs/54154.kar
*/

/*
 * APIs:
 * GString: https://developer.gnome.org/glib/2.28/glib-Strings.html
 * Pango text attributes: https://developer.gnome.org/pango/stable/pango-Text-Attributes.html#pango-parse-markup
 * Pango layout: http://www.gtk.org/api/2.6/pango/pango-Layout-Objects.html
 * Cairo rendering: https://developer.gnome.org/pango/stable/pango-Cairo-Rendering.html#pango-cairo-create-layout
 * Cairo surface_t: http://cairographics.org/manual/cairo-cairo-surface-t.html
 * GTK+ 3 Reference Manual: https://developer.gnome.org/gtk3/3.0/
 * Gdk Pixbufs: https://developer.gnome.org/gdk/stable/gdk-Pixbufs.html
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
#ifdef USE_PIXBUF
    GdkPixbuf *pixbuf;
#endif
} coloured_line_t;

coloured_line_t coloured_lines[2];

GtkWidget *image;

int height_lyric_pixbufs[] = {300, 400}; // vertical offset of lyric in video

// fluid_player_t* player;

int current_panel = 1;  // panel showing current lyric line
int current_line = 0;  // which line is the current lyric
gchar *current_lyric;   // currently playing lyric line
GString *front_of_lyric;  // part of lyric to be coloured red
//GString *end_of_lyric;    // part of lyrci to not be coloured



// Colours seem to get mixed up when putting a pixbuf onto a pixbuf
#ifdef USE_PIXBUF
#define RED blue
#else
#define RED red
#endif

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
    printf("Marked up label \"%s\"\n", str->str);

    line->marked_up_line = str->str;
    // we have to free line->marked_up_line

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
    cairo_surface_destroy(surface);
    g_object_unref(old_pixbuf);
}


extern ControlMode  *ctl;

ControlMode video_ctl=
    {
	"video interface", 'v',
	"video",
	1,          /* verbosity */
	1,          /* trace playing */
	0,          /* opened */
	0,          /* flags */
	ctl_open,
	ctl_close,
	pass_playing_list,
	ctl_read,
	NULL,       /* write */
	cmsg,
	ctl_event
    };

static void init_timidity() {
    int err;

    timidity_start_initialize();

    if ((err = timidity_pre_load_configuration()) != 0) {
	printf("couldn't pre-load configuration file\n");
	exit(1);
    }

    err += timidity_post_load_configuration();

    if (err) {
	printf("couldn't post-load configuration file\n");
	exit(1);
    }

    timidity_init_player();

    
    extern int opt_trace_text_meta_event;
    opt_trace_text_meta_event = 1;

    //ctl = &video_ctl;
    //ctl->trace_playing = 1;
    //opt_trace_text_meta_event = 1;
    
}

static FILE *outfp;
int video_error_count;
static char *current_file;
struct midi_file_info *current_file_info;

static int pass_playing_list(int number_of_files, char *list_of_files[]) {
    int n;

    for (n = 0; n < number_of_files; n++) {
	printf("Playing list %s\n", list_of_files[n]);
	
	current_file = list_of_files[n];
	/*
	  current_file_info = get_midi_file_info(current_file, 1);
	  if (current_file_info != NULL) {
	  printf("file info not NULL\n");
	  } else {
	  printf("File info is NULL\n");
	  }
	*/
	play_midi_file( list_of_files[n]);
    }
    return 0;
}

static int inited_video = 0;
/*ARGSUSED*/
static int ctl_open(int using_stdin, int using_stdout)
{
    if (! inited_video) {
	init_video();
	inited_video = 1;
    }

    // dont know what this function does
    /*
      if (current_file != NULL) {
      current_file_info = get_midi_file_info(current_file, 1);
      printf("Opening info for %s\n", current_file);
      } else {
      printf("Current is NULL\n");
      }
    */
    ctl->opened = 1;
    return 0;
}

static void ctl_close(void)
{
    fflush(outfp);
    video_ctl.opened=0;
}

/*ARGSUSED*/
static int ctl_read(int32 *valp)
{
    return RC_NONE;
}

static int cmsg(int type, int verbosity_level, char *fmt, ...)
{
    return 0;
}

static void ctl_total_time(long tt)
{
    /*
      int mins, secs;
      if (video_ctl.trace_playing)
      {
      secs=(int)(tt/play_mode->rate);
      mins=secs/60;
      secs-=mins*60;
      cmsg(CMSG_INFO, VERB_NORMAL,
      "Total playing time: %3d min %02d s", mins, secs);
      }
    */
}

static void ctl_file_name(char *name)
{
    current_file = name;

    if (video_ctl.verbosity>=0 || video_ctl.trace_playing)
	cmsg(CMSG_INFO, VERB_NORMAL, "Playing %s", name);
}

static void ctl_current_time(int secs)
{
    int mins;
    static int prev_secs = -1;

#ifdef __W32__
    if(wrdt->id == 'w')
	return;
#endif /* __W32__ */
    if (ctl->trace_playing && secs != prev_secs)
	{
	    prev_secs = secs;
	    mins=secs/60;
	    secs-=mins*60;
	    fprintf(stdout, "\r%3d:%02d", mins, secs);
	}
}

void build_lyric_lines() {
    int n;
    lyric_t *plyric;
    GString *line = g_string_new("");
    GArray *lines =  g_array_sized_new(FALSE, FALSE, sizeof(GString *), 64);

    lyric_lines.title = NULL;

    n = 1;
    char *evt_str;
    while ((evt_str = event2string(n++)) != NULL) {

        gchar *lyric = evt_str+1;
	printf("Building line %s\n", lyric);

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

#if GTK_MAJOR_VERSION == 2
    GdkPixmap *pixmap;
    GdkBitmap *mask;
#endif

    // pFrame=avcodec_alloc_frame();
    pFrame=av_frame_alloc();

    i=0;
    // picture_RGB = avcodec_alloc_frame();
    picture_RGB = av_frame_alloc();
    buffer = malloc (avpicture_get_size(PIX_FMT_RGB24, 720, 576));
    avpicture_fill((AVPicture *)picture_RGB, buffer, PIX_FMT_RGB24, 720, 576);

    int width = pCodecCtx->width;
    int height = pCodecCtx->height;
    
    sws_ctx = sws_getContext(pCodecCtx->width, pCodecCtx->height, pCodecCtx->pix_fmt, pCodecCtx->width, pCodecCtx->height, PIX_FMT_RGB24, SWS_BICUBIC, NULL, NULL, NULL);
    
    while(av_read_frame(pFormatCtx, &packet)>=0) {
	if(packet.stream_index==videoStream) {
	    // printf("Frame %d\n", i++);
	    usleep(33670);  // 29.7 frames per second
	    // Decode video frame
	    avcodec_decode_video2(pCodecCtx, pFrame, &frameFinished,
				  &packet);


	    if (frameFinished) {
		printf("Frame %d width %d height %d\n", i++, width, height);
		
		sws_scale(sws_ctx,  (uint8_t const * const *) pFrame->data, pFrame->linesize, 0, pCodecCtx->height, picture_RGB->data, picture_RGB->linesize);
		

		pixbuf = gdk_pixbuf_new_from_data(picture_RGB->data[0], GDK_COLORSPACE_RGB, 0, 8, 720, 480, picture_RGB->linesize[0], pixmap_destroy_notify, NULL);

#define SHOW_LYRIC
#ifdef SHOW_LYRIC
		printf("Creating cairo surface\n");
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

		//gtk_image_set_from_pixbuf((GtkImage*) image, pixbuf);

		//g_object_unref(pixbuf);		/* reclaim memory */
		//sws_freeContext(sws_ctx);
		//g_object_unref(layout);
		cairo_surface_destroy(surface);
		cairo_destroy(cr);
#else
		gtk_image_set_from_pixbuf((GtkImage*) image, pixbuf);
#endif /* SHOW_LYRIC */

	    }
	}
	av_free_packet(&packet);
    }
    sws_freeContext(sws_ctx);

    printf("Video over!\n");
    exit(0);
}

static void ctl_lyric(int lyricid)
{
    char *lyric;

    current_file_info = get_midi_file_info(current_file, 1);

    lyric = event2string(lyricid);
    if(lyric != NULL)
	lyric++;
    printf("Got a lyric %s\n", lyric);
    if (*lyric == '\\') {
	int next_panel = current_panel; // really (current_panel+2)%2
	int next_line = current_line + 2;
	gchar *next_lyric;

	if (current_line + 2 >= lyric_lines.lines->len) {
	    return;
	}
	current_line += 1;
	current_panel = (current_panel + 1) % 2;

	// set up new line as current line
	current_lyric = g_array_index(lyric_lines.lines, GString *, current_line)->str;
	front_of_lyric = g_string_new(lyric+1); // lose \
	printf("New line. Setting front to %s end to \"%s\"\n", lyric+1, current_lyric); 

	coloured_lines[current_panel].line = current_lyric;
	coloured_lines[current_panel].front_of_line = lyric+1;
	markup_line(coloured_lines+current_panel);
#ifdef USE_PIXBUF
	update_line_pixbuf(coloured_lines+current_panel);
#endif
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
#ifdef USE_PIXBUF
	update_line_pixbuf(coloured_lines+next_panel);
#endif
    } else {
	// change text colour as chars are played
	if ((front_of_lyric != NULL) && (lyric != NULL)) {
	    g_string_append(front_of_lyric, lyric);
	    char *s = front_of_lyric->str;
	    coloured_lines[current_panel].front_of_line = s;
	    markup_line(coloured_lines+current_panel);
#ifdef USE_PIXBUF
	    update_line_pixbuf(coloured_lines+current_panel);
#endif
	}
    }
}


static void ctl_event(CtlEvent *e)
{
    //printf("Got ctl event %d\n", e->type);
    switch(e->type)
	{
	case CTLE_NOW_LOADING:
	    ctl_file_name((char *)e->v1);
	    break;
	case CTLE_LOADING_DONE:
	    // MIDI file is loaded, about to play
	    current_file_info = get_midi_file_info(current_file, 1);
	    if (current_file_info != NULL) {
		printf("file info not NULL\n");
	    } else {
		printf("File info is NULL\n");
	    }

	    int n = 1;
	    char *evt_str;
	    while ((evt_str = event2string(n++)) != NULL) {
		printf("Event in tabel: %s\n", evt_str);
	    }

	    build_lyric_lines();

	    break;
	case CTLE_PLAY_START:

	    ctl_total_time(e->v1);
	    break;
	case CTLE_CURRENT_TIME:
	    ctl_current_time((int)e->v1);
	    break;
	case CTLE_LYRIC:
	    ctl_lyric((int)e->v1);
	    break;
	    /*
	      case CTLE_REFRESH:
	      printf("Refresh\n");
	      break;
	    */
	default:
	    0;
	    //printf("Other event\n");
	}
}



static void *play_timidity(void *args) {
    char *argv[] = {"54154.kar"};
    
    timidity_play_main(1, argv);
    printf("Play timidity finished\n");
}

/* Called when the windows are realized
 */
static void realize_cb (GtkWidget *widget, gpointer data) {
    /* start the video playing in its own thread */
    pthread_t back_id;
    pthread_create(&back_id, NULL, play_background, NULL);

#if 0
    pthread_t timidity_id;
    pthread_create(&timidity_id, NULL, play_timidity, NULL);
#endif
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

void *play_gtk(void *args) {
    printf("About to start gtk_main\n");
    gtk_main();
}

int init_video() {
    #define FNAME "short.mpg"

    lyrics = g_array_sized_new(FALSE, FALSE, sizeof(lyric_t *), 1024);

    /* FFMpeg stuff */

    AVFrame *pFrame = NULL;
    AVPacket packet;

    AVDictionary *optionsDict = NULL;

    av_register_all();

    if(avformat_open_input(&pFormatCtx, FNAME, NULL, NULL)!=0) {
	printf("Couldn't open video file called short.mpg\n");
	return -1; // Couldn't open file
    } else {
	printf("Opened short.mpg\n");
    }
  
    // Retrieve stream information
    if(avformat_find_stream_info(pFormatCtx, NULL)<0) {
	printf("Couldn't find stream information\n");
	return -1; // Couldn't find stream information
    }

    // Dump information about file onto standard error
    av_dump_format(pFormatCtx, 0, FNAME, 0);
  
    // Find the first video stream
    videoStream=-1;

    int i;
    for(i=0; i<pFormatCtx->nb_streams; i++)
	if(pFormatCtx->streams[i]->codec->codec_type==AVMEDIA_TYPE_VIDEO) {
	    videoStream=i;
	    break;
	}
    if(videoStream==-1) {
	fprintf(stderr, "Couldn't find a vide stream\n");
	return -1; // Didn't find a video stream
    }

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

    /* GTK stuff now */

    /* GtkWidget is the storage type for widgets */
    GtkWidget *window;
    GtkWidget *button;
    GtkWidget *lyrics_box;

    
    /* This is called in all GTK applications. Arguments are parsed
     * from the command line and are returned to the application. */
    gtk_init (0, NULL);
    
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

    //lyrics_box = gtk_vbox_new(TRUE, 1);
    lyrics_box = gtk_box_new(TRUE, 1);
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
    pthread_t tid_gtk;
    pthread_create(&tid_gtk, NULL, play_gtk, NULL);
    //gtk_main ();
}

/*
 * interface_<id>_loader();
 */
ControlMode *interface_v_loader(void)
{
    return &video_ctl;
}
