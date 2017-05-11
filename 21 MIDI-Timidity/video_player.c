#include <string.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

#define MIDI_FILE "54154.mid"

void timidity_start_initialize(void);
int timidity_pre_load_configuration(void);
int timidity_post_load_configuration(void);
void timidity_init_player(void);

static void *play_midi(void *args) {
    char *argv[1];
    argv[0] = MIDI_FILE;
    int argc = 1;

    timidity_play_main(argc, argv);

    printf("Audio finished\n");
    exit(0);
}

void *init_gtk(void *args);
void init_ffmpeg();

int main(int argc, char** argv)
{

    int i;

    /* make X happy */
    //XInitThreads();

    /* Timidity stuff */
    int err;

    timidity_start_initialize();
    if ((err = timidity_pre_load_configuration()) == 0) {
	err = timidity_post_load_configuration();
    }
    if (err) {
        printf("couldn't load configuration file\n");
        exit(1);
    }

    timidity_init_player();

    init_ffmpeg();
    pthread_t tid_gtk;
    pthread_create(&tid_gtk, NULL, init_gtk, NULL);

    play_midi(NULL);    
    return 0;
}
