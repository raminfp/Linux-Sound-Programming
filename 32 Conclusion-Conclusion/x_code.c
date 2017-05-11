
#include <X11/Xlib.h>
#include <X11/Xutil.h>

#include <gtk/gtk.h>

#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>

#include "mytimidity.h"

#define WIDTH 720
#define HEIGHT 480

#define NUM_LINES 4

Display *display;
Window window;
GC gc, color_gc;

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
GString *lyrics_array[NUM_LINES];

lyric_lines_t lyric_lines;

typedef struct _coloured_line_t {
    gchar *line;
    gchar *front_of_line;
    gchar *marked_up_line;
    PangoAttrList *attrs;
} coloured_line_t;

int height_lyric_pixbufs[] = {100, 200, 300, 400}; // vertical offset of lyric in video
int coloured_text_offset;

// fluid_player_t* player;

// int current_panel = 1;  // panel showing current lyric line
int current_line = 0;  // which line is the current lyric
gchar *current_lyric;   // currently playing lyric line
GString *front_of_lyric;  // part of lyric to be coloured red
//GString *end_of_lyric;    // part of lyrci to not be coloured


gchar *markup[] = {"<span font=\"28\" foreground=\"RED\">",
		   "</span><span font=\"28\" foreground=\"white\">",
		   "</span>"};
gchar *markup_newline[] = {"<span foreground=\"black\">",
			   "</span>"};
GString *marked_up_label;

PangoFontDescription *font_description;

cairo_surface_t *surface;
cairo_t *cr;

extern ControlMode  *ctl;

ControlMode video_ctl=
    {
	"x interface", 'x',
	"x",
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

static FILE *outfp;
int video_error_count;
static char *current_file;
struct midi_file_info *current_file_info;

static int pass_playing_list(int number_of_files, char *list_of_files[]) {
    int n;

    for (n = 0; n < number_of_files; n++) {
	printf("Playing list %s\n", list_of_files[n]);
	
	current_file = list_of_files[n];
	play_midi_file( list_of_files[n]);
    }
    XCloseDisplay(display);
    exit(0);
    return 0;
}

static void paint_background() {
    cr = cairo_create(surface);
    cairo_set_source_rgb(cr, 0.0, 0.8, 0.0);
    cairo_paint(cr);
    cairo_destroy(cr);
}

static void set_font() {
    font_description = pango_font_description_new ();
    pango_font_description_set_family (font_description, "serif");
    pango_font_description_set_weight (font_description, PANGO_WEIGHT_BOLD);
    pango_font_description_set_absolute_size (font_description, 32 * PANGO_SCALE);
}

static int draw_text(char *text, float red, float green, float blue, int height, int offset) {
  // See http://cairographics.org/FAQ/
  PangoLayout *layout;
  int width, ht;
  cairo_text_extents_t extents;

  layout = pango_cairo_create_layout (cr);
  pango_layout_set_font_description (layout, font_description);
  pango_layout_set_text (layout, text, -1);

  if (offset == 0) {
      pango_layout_get_size(layout, &width, &ht);
      offset = (WIDTH - (width/PANGO_SCALE)) / 2;
  }

  cairo_set_source_rgb (cr, red, green, blue);
  cairo_move_to (cr, offset, height);
  pango_cairo_show_layout (cr, layout);

  g_object_unref (layout);
  return offset;
}

static void init_X() {
    int screen;
    unsigned long foreground, background;
    XSizeHints hints;
    char **argv = NULL;
    XGCValues gcValues;
    Colormap colormap;
    XColor rgb_color, hw_color;
    Font font;
    //char *FNAME = "hanzigb24st";
    char *FNAME = "-misc-fixed-medium-r-normal--0-0-100-100-c-0-iso10646-1";

    display = XOpenDisplay(NULL);
    if (display == NULL) {
	fprintf(stderr, "Can't open dsplay\n");
	exit(1);
    }
    screen = DefaultScreen(display);
    foreground = BlackPixel(display, screen);
    background = WhitePixel(display, screen);

    window = XCreateSimpleWindow(display,
				 DefaultRootWindow(display),
				 0, 0, WIDTH, HEIGHT, 10,
				 foreground, background);
    hints.x = 0;
    hints.y = 0;
    hints.width = WIDTH;
    hints.height = HEIGHT;
    hints.flags = PPosition | PSize;

    XSetStandardProperties(display, window, 
			   "X", "X", 
			   None,
			   argv, 0,
			   &hints);

    XMapWindow(display, window);


    set_font();
    surface = cairo_xlib_surface_create(display, window,
					DefaultVisual(display, 0), WIDTH, HEIGHT);
    cairo_xlib_surface_set_size(surface, WIDTH, HEIGHT);

    paint_background();

    /*
    cr = cairo_create(surface);
    draw_text(g_array_index(lyric_lines.lines, GString *, 0)->str,
	      0.0, 0.0, 1.0, height_lyric_pixbufs[0]);
    draw_text(g_array_index(lyric_lines.lines, GString*, 1)->str,
	      0.0, 0.0, 1.0, height_lyric_pixbufs[0]);
    cairo_destroy(cr);
    */
    XFlush(display);
}


static int inited_video = 0;
/*ARGSUSED*/
static int ctl_open(int using_stdin, int using_stdout)
{
    init_X();

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
    exit(0);
}

/*ARGSUSED*/
static int ctl_read(int32 *valp)
{
    return RC_NONE;
}

static int cmsg(int type, int verbosity_level, char *fmt, ...)
{
    /*
      va_list ap;

      if ((type==CMSG_TEXT || type==CMSG_INFO || type==CMSG_WARNING) &&
      video_ctl.verbosity<verbosity_level)
      return 0;
      va_start(ap, fmt);
      if(type == CMSG_WARNING || type == CMSG_ERROR || type == CMSG_FATAL)
      video_error_count++;
      if (!video_ctl.opened)
      {
      vfprintf(stderr, fmt, ap);
      fputs(NLS, stderr);
      }
      else
      {
      vfprintf(outfp, fmt, ap);
      fputs(NLS, outfp);
      fflush(outfp);
      }
      va_end(ap);
    */
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

static void ctl_lyric(int lyricid)
{
    char *lyric;

    current_file_info = get_midi_file_info(current_file, 1);

    lyric = event2string(lyricid);
    if(lyric != NULL)
	lyric++;
    printf("Got a lyric %s\n", lyric);


    if (*lyric == '\\') {
	// int next_panel = (current_panel+2) % NUM_LINES; // really (current_panel+2)%2
	int next_line = current_line + NUM_LINES;
	gchar *next_lyric;

	if (current_line + NUM_LINES < lyric_lines.lines->len) {
	    current_line += 1;
	    
	    //lyrics_array[(next_line-1) % NUM_LINES] = 
	    //	g_array_index(lyric_lines.lines, GString *, next_line);
	    
	    // update label for next line after this one
	    next_lyric = g_array_index(lyric_lines.lines, GString *, next_line)->str;

	} else {
	    current_line += 1;
	    lyrics_array[(next_line-1) % NUM_LINES] = NULL;
	    next_lyric = "";
	}

	// set up new line as current line
	if (current_line < lyric_lines.lines->len) {
	    GString *gstr = g_array_index(lyric_lines.lines, GString *, current_line);
	    current_lyric = gstr->str;
	    front_of_lyric = g_string_new(lyric+1); // lose	slosh
	}	  
	printf("New line. Setting front to %s end to \"%s\"\n", lyric+1, current_lyric); 


	// Now draw stuff
	paint_background();

	cr = cairo_create(surface);

	int n;
	for (n = 0; n < NUM_LINES; n++) {
	    //lyrics_array[n] = g_array_index(lyric_lines.lines, GString *, n+1);
	    if (lyrics_array[n] != NULL) {
		draw_text(lyrics_array[n]->str,
			  0.0, 0.0, 0.5, height_lyric_pixbufs[n], 0);
	    }
	}
	// redraw current and next lines
	if (current_line < lyric_lines.lines->len) {
	    if (current_line >= 2) {
		// redraw last line still in red
		GString *gstr = lyrics_array[(current_line-2) % NUM_LINES];
		if (gstr != NULL) {
		    draw_text(gstr->str,
			      1.0, 0.0, 00, 
			      height_lyric_pixbufs[(current_line-2) % NUM_LINES],
			      0);
		}
	    }
	    // draw next line in brighter blue
	    coloured_text_offset = draw_text(lyrics_array[(current_line-1) % NUM_LINES]->str,
		      0.0, 0.0, 1.0, height_lyric_pixbufs[(current_line-1) % NUM_LINES], 0);
	    printf("coloured text offset %d\n", coloured_text_offset);
	}

	//try
	if (next_line < lyric_lines.lines->len) {
	    lyrics_array[(next_line-1) % NUM_LINES] = 
		g_array_index(lyric_lines.lines, GString *, next_line);
	}
	

	    /*
	draw_text(current_lyric, 0.0, 0.0, 1.0, 
		  height_lyric_pixbufs[(current_line-1) % NUM_LINES]);


	draw_text(next_lyric, 0.0, 0.0, 1.0, 
		  height_lyric_pixbufs[(next_line-1) % NUM_LINES]);
	    */
	cairo_destroy(cr);
	XFlush(display);


    } else {
	// change text colour as chars are played
	if ((front_of_lyric != NULL) && (lyric != NULL)) {
	    g_string_append(front_of_lyric, lyric);
	    char *s = front_of_lyric->str;
	    //coloured_lines[current_panel].front_of_line = s;

	    cairo_t *cr = cairo_create(surface);

	    // See http://cairographics.org/FAQ/
	    draw_text(s, 1.0, 0.0, 0.0, 
		      height_lyric_pixbufs[(current_line-1) % NUM_LINES], 
		      coloured_text_offset);

	    cairo_destroy(cr);
	    XFlush(display);

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
	    cr = cairo_create(surface);

	    // draw line to be sung slightly brighter
	    // than the rest
	    for (n = 0; n < NUM_LINES; n++) {
		lyrics_array[n] = g_array_index(lyric_lines.lines, GString *, n+1);
		draw_text(lyrics_array[n]->str,
			  0.0, 0.0, 0.5, height_lyric_pixbufs[n], 0);
	    }
	    draw_text(lyrics_array[0]->str,
		      0.0, 0.0, 1.0, height_lyric_pixbufs[0], 0);

	    /*
	    draw_text(g_array_index(lyric_lines.lines, GString *, 1)->str,
		      0.0, 0.0, 1.0, height_lyric_pixbufs[0]);
	    draw_text(g_array_index(lyric_lines.lines, GString*, 2)->str,
		      0.0, 0.0, 1.0, height_lyric_pixbufs[1]);
	    draw_text(g_array_index(lyric_lines.lines, GString*, 3)->str,
		      0.0, 0.0, 1.0, height_lyric_pixbufs[2]);
	    */
	    cairo_destroy(cr);
	    XFlush(display);
	    
	    break;
	case CTLE_PLAY_START:

	    ctl_total_time(e->v1);
	    break;
        case CTLE_PLAY_END:
	    printf("Exiting, play ended\n");
	    exit(0);
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

/*
 * interface_<id>_loader();
 */
ControlMode *interface_x_loader(void)
{
    return &video_ctl;
}
