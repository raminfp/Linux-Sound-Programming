#include <fluidsynth.h>
#include <fluid_midi.h>
#include <string.h>
#include <stdlib.h>

#include <gtk/gtk.h>


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

fluid_synth_t* synth;

GtkWidget *lyric_labels[2];

fluid_player_t* player;

int current_panel = -1;  // panel showing current lyric line
int current_line = 0;  // which line is the current lyric
gchar *current_lyric;   // currently playing lyric line
GString *front_of_lyric;  // part of lyric to be coloured red
GString *end_of_lyric;    // part of lyric to not be coloured

gchar *markup[] = {"<span foreground=\"red\">",
		   "</span><span foreground=\"black\">",
		   "</span>"};
gchar *markup_newline[] = {"<span foreground=\"black\">",
		   "</span>"};
GString *marked_up_label;

struct _reset_label_data {
    GtkLabel *label;
    gchar *text;
    PangoAttrList *attrs;
};

typedef struct _reset_label_data reset_label_data;

/**
 * redraw a label
 */
gint reset_label_cb(gpointer data) {
    reset_label_data *rdata = ( reset_label_data *) data;

    if (rdata->label == NULL) {
	printf("Label is null, cant set its text \n");
	return FALSE;
    }
    gtk_label_set_text(rdata->label, rdata->text);
    gtk_label_set_attributes(rdata->label, rdata->attrs);

    return FALSE;
}

reset_label_data *new_label_data(gchar *str,
				 GtkWidget *label) {
    PangoAttrList *attrs;
    gchar *text;
    pango_parse_markup (str, -1,0, &attrs, &text, NULL, NULL);
    reset_label_data *label_data;
    label_data = g_new(reset_label_data, 1);
    label_data->label = ( GtkLabel *) label;
    label_data->text = text;
    label_data->attrs = attrs;

    return label_data;
}

/**
 * This MIDI event callback filters out the TEXT and LYRIC events
 * and passes the rest to the default event handler.
 * Here we colour the text in a Gtk label
 */
int event_callback(void *data, fluid_midi_event_t *event) {
    fluid_synth_t* synth = (fluid_synth_t*) data;
    int type = fluid_midi_event_get_type(event);
    if (synth == NULL) printf("Synth is null\n");
    switch(type) {
    case MIDI_TEXT:
	printf("Callback: Playing text event %s (length %d)\n", 
	       (char *) event->paramptr, event->param1);

	if (((char *) event->paramptr)[0] == '\\') {
	    // we've got a new line, change the label text on the NEXT panel
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

	    // find the next line from lyric_lines array
	    current_lyric = g_array_index(lyric_lines.lines, GString *, current_line)->str;

	    // lyric is in 2 parts: front coloured, end uncoloured
	    front_of_lyric = g_string_new(lyric+1); // lose '\'   
	    end_of_lyric = g_string_new(current_lyric);
	    printf("New line. Setting front to %s end to \"%s\"\n", lyric+1, current_lyric); 

	    // update label for next line after this one
	    next_lyric = g_array_index(lyric_lines.lines, GString *, next_line)->str;

	    gchar *str;
	    str = g_strconcat(markup_newline[0], next_lyric, markup_newline[1], NULL);
	    reset_label_data *label_data;
	    label_data = new_label_data(str,
					lyric_labels[next_panel]);

	    gdk_threads_add_idle(reset_label_cb, label_data);
	} else {
	    // change text colour as chars are played, using Pango attributes
	    char *lyric =  event->paramptr;
	    if ((front_of_lyric != NULL) && (lyric != NULL)) {
		// add the new lyric to the front of the existing coloured
		g_string_append(front_of_lyric, lyric);
		char *s = front_of_lyric->str;

		marked_up_label = g_string_new(markup[0]);
		g_string_append(marked_up_label, s);
		g_string_append(marked_up_label, markup[1]);
		g_string_append(marked_up_label, current_lyric + strlen(s));
		g_string_append(marked_up_label, markup[2]);
		printf("Marked up label \"%s\"\n", marked_up_label->str);

		reset_label_data *label_data;
		label_data = new_label_data(marked_up_label->str,
					    lyric_labels[current_panel]);
		gdk_threads_add_idle(reset_label_cb, label_data);
	    }
	}
	return  FLUID_OK;

    case MIDI_LYRIC:
	printf("Callback: Playing lyric event %d %s\n", 
	       event->param1, (char *) event->paramptr);
	return  FLUID_OK;
    
    case MIDI_EOT:
	printf("End of track\n");
	exit(0);
    }
    // default handler for all other events
    return fluid_synth_handle_midi_event( data, event);
}

/*
 * Build array of lyric lines from the MIDI file data
 */
void build_lyric_lines() {
    int n;
    lyric_t *plyric;
    GString *line = g_string_new("");
    GArray *lines =  g_array_sized_new(FALSE, FALSE, sizeof(GString *), 64);

    lyric_lines.title = NULL;

    for (n = 0; n < lyrics->len; n++) {
	plyric = g_array_index(lyrics, lyric_t *, n);
	gchar *lyric = plyric->lyric;

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
 * Here we extract the TEXT and LYRIC events and save them
 * into an array
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

    // stick the first two lines into the labels so we can see
    // what is coming
    //gdk_threads_enter();
    char *str = g_array_index(lyric_lines.lines, GString *, 1)->str;
    gtk_label_set_text((GtkLabel *) lyric_labels[0], str);
    str = g_array_index(lyric_lines.lines, GString *, 2)->str;
    gtk_label_set_text((GtkLabel *) lyric_labels[1], str);
    // gdk_flush ();
    
    /* release GTK thread lock */
    //gdk_threads_leave();
    
    return FLUID_OK;
}

/* Called when the windows are realized
 */
static void realize_cb (GtkWidget *widget, gpointer data) {
    /* now we can play the midi files, if any */
    fluid_player_play(player);
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

    /* set up the fluidsynth stuff */
    int i;
    fluid_settings_t* settings;

    fluid_audio_driver_t* adriver;
    settings = new_fluid_settings();
    fluid_settings_setstr(settings, "audio.driver", "alsa");
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

    // Gtk stuff now

   /* GtkWidget is the storage type for widgets */
    GtkWidget *window;
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

    // Gtk 3.0 deprecates gtk_vbox_new in favour of gtk_grid
    // but that isn't in Gtk 2.0, so we ignore warnings for now
    lyrics_box = gtk_box_new(TRUE, 1);
    gtk_widget_show(lyrics_box);

    char *str = "  ";
    lyric_labels[0] = gtk_label_new(str);
    lyric_labels[1] = gtk_label_new(str);

    gtk_widget_show (lyric_labels[0]);
    gtk_widget_show (lyric_labels[1]);

    
    gtk_box_pack_start (GTK_BOX (lyrics_box), lyric_labels[0], TRUE, TRUE, 0);
    gtk_box_pack_start (GTK_BOX (lyrics_box), lyric_labels[1], TRUE, TRUE, 0);

    /* This packs the button into the window (a gtk container). */
    gtk_container_add (GTK_CONTAINER (window), lyrics_box);
        
    /* and the window */
    gtk_widget_show (window);
    
    /* All GTK applications must have a gtk_main(). Control ends here
     * and waits for an event to occur (like a key press or
     * mouse event). */
    gtk_main ();
    
    /* wait for playback termination */
    fluid_player_join(player);
    /* cleanup */
    delete_fluid_audio_driver(adriver);
    delete_fluid_player(player);
    delete_fluid_synth(synth);
    delete_fluid_settings(settings);
    return 0;
}

