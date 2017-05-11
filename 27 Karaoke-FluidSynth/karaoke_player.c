#include <fluidsynth.h>
#include <fluid_midi.h>

/**
 * This MIDI event callback filters out the TEXT and LYRIC events
 * and passes the rest to the default event handler.
 * Here we just print the text of the event, more
 * complex handling can be done
 */
int event_callback(void *data, fluid_midi_event_t *event) {
    fluid_synth_t* synth = (fluid_synth_t*) data;
    int type = fluid_midi_event_get_type(event);
    int chan = fluid_midi_event_get_channel(event);
    if (synth == NULL) printf("Synth is null\n");
    switch(type) {
    case MIDI_TEXT:
	printf("Callback: Playing text event %s (length %d)\n", 
	       (char *) event->paramptr, event->param1);
	return  FLUID_OK;

    case MIDI_LYRIC:
	printf("Callback: Playing lyric event %d %s\n", 
	       event->param1, (char *) event->paramptr);
	return  FLUID_OK;
    }
    return fluid_synth_handle_midi_event( data, event);
}

/**
 * This is called whenever new data is loaded, such as a new file.
 * Here we extract the TEXT and LYRIC events and just print them
 * to stdout. They could e.g. be saved and displayed in a GUI
 * as the events are received by the event callback.
 */ 
int onload_callback(void *data, fluid_player_t *player) {
    printf("Load callback, tracks %d \n", player->ntracks);
    int n;
    for (n = 0; n < player->ntracks; n++) {
	fluid_track_t *track = player->track[n];
	printf("Track %d\n", n);
	fluid_midi_event_t *event = fluid_track_first_event(track);
	while (event != NULL) {
	    switch (event->type) {
	    case MIDI_TEXT:
	    case MIDI_LYRIC:
		printf("Loaded event %s\n", (char *) event->paramptr);
	    }
	    event = fluid_track_next_event(track);
	}
    }
    return FLUID_OK;
}

int main(int argc, char** argv)
{
    int i;
    fluid_settings_t* settings;
    fluid_synth_t* synth;
    fluid_player_t* player;
    fluid_audio_driver_t* adriver;
    settings = new_fluid_settings();
    fluid_settings_setstr(settings, "audio.driver", "alsa");
    fluid_settings_setint(settings, "synth.polyphony", 64);
    synth = new_fluid_synth(settings);
    player = new_fluid_player(synth);

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
    /* play the midi files, if any */
    fluid_player_play(player);
    /* wait for playback termination */
    fluid_player_join(player);
    /* cleanup */
    delete_fluid_audio_driver(adriver);
    delete_fluid_player(player);
    delete_fluid_synth(synth);
    delete_fluid_settings(settings);
    return 0;
}

