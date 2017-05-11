

void timidity_start_initialize(void);
int timidity_pre_load_configuration(void);
int timidity_post_load_configuration(void);
void timidity_init_player(void);
int timidity_play_main(int nfiles, char **files);
int got_a_configuration;

#define HAVE_STRNCASECMP
#include <sysdep.h>
#include <timidity.h>
#include <controls.h>
#include <output.h>
#include <instrum.h>
#include <playmidi.h>
#include <readmidi.h>

static int ctl_open(int using_stdin, int using_stdout);
static void ctl_close(void);
static int ctl_read(int32_t *valp);
static int cmsg(int type, int verbosity_level, char *fmt, ...);
static void ctl_total_time(long tt);
static void ctl_file_name(char *name);
static void ctl_current_time(int ct);
static void ctl_lyric(int lyricid);
static void ctl_event(CtlEvent *e);
static int pass_playing_list(int number_of_files, char *list_of_files[]);
