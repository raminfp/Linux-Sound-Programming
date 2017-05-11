
#include <stdio.h>
#include <stdlib.h>

#include "sysdep.h"
#include "controls.h"


extern ControlMode  *video_ctl;
extern ControlMode  *ctl;

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

    ctl = &video_ctl;
    //ctl->trace_playing = 1;
    //opt_trace_text_meta_event = 1;
    
}


#define MIDI_FILE "54154.kar"

static void *play_midi(void *args) {
    char *argv[1];
    argv[0] = MIDI_FILE;
    int argc = 1;

    timidity_play_main(argc, argv);

    printf("Audio finished\n");
    exit(0);
}


int main(int argc, char** argv)
{
    XInitThreads();

    int i;

    /* TiMidity */
    init_timidity();
    play_midi(NULL);


    return 0;
}

