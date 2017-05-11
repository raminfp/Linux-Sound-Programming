
#include <stdio.h>

extern void timidity_start_initialize(void);
extern int timidity_pre_load_configuration(void);
extern int timidity_post_load_configuration(void);
extern void timidity_init_player(void);
extern int timidity_play_main(int nfiles, char **files);
extern int got_a_configuration;

int main(int argc, char **argv)
{
    int err, main_ret;

    timidity_start_initialize();




    if ((err = timidity_pre_load_configuration()) != 0)
	return err;

    err += timidity_post_load_configuration();

    if (err) {
	printf("couldn't load configuration file\n");
	exit(1);
    }

    timidity_init_player();

    main_ret = timidity_play_main(argc, argv);

    return main_ret;
}

