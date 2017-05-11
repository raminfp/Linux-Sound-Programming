/*
  video_player_interface.c
*/
#include <pthread.h>
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif /* HAVE_CONFIG_H */
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#ifndef NO_STRING_H
#include <string.h>
#else
#include <strings.h>
#endif

#include "support.h"
#include "timidity.h"
#include "output.h"
#include "controls.h"
#include "instrum.h"
#include "playmidi.h"
#include "readmidi.h"

static int ctl_open(int using_stdin, int using_stdout);
static void ctl_close(void);
static int ctl_read(int32 *valp);
static int cmsg(int type, int verbosity_level, char *fmt, ...);
static void ctl_total_time(long tt);
static void ctl_file_name(char *name);
static void ctl_current_time(int ct);
static void ctl_lyric(int lyricid);
static void ctl_event(CtlEvent *e);
static int pass_playing_list(int number_of_files, char *list_of_files[]);


/**********************************/
/* export the interface functions */

#define ctl karaoke_control_mode

ControlMode ctl=
    {
	"video player interface", 'v',
	"video player",
	1,          /* verbosity */
	0,          /* trace playing */
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
int video_player_error_count;
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

extern void *init_gtk(void *args);
extern void init_ffmpeg(void);

/*ARGSUSED*/
static int ctl_open(int using_stdin, int using_stdout)
{
    if(using_stdout)
	outfp=stderr;
    else
	outfp=stdout;
    ctl.opened=1;

    init_ffmpeg();

    /* start Gtk in its own thread */
    pthread_t tid_gtk;
    //init_gtk(0, NULL);
    pthread_create(&tid_gtk, NULL, init_gtk, NULL);

    return 0;
}

static void ctl_close(void)
{
    fflush(outfp);
    ctl.opened=0;
}

/*ARGSUSED*/
static int ctl_read(int32 *valp)
{
    return RC_NONE;
}

static int cmsg(int type, int verbosity_level, char *fmt, ...)
{
    va_list ap;

    if ((type==CMSG_TEXT || type==CMSG_INFO || type==CMSG_WARNING) &&
	ctl.verbosity<verbosity_level)
	return 0;
    va_start(ap, fmt);
    if(type == CMSG_WARNING || type == CMSG_ERROR || type == CMSG_FATAL)
	video_player_error_count++;
    if (!ctl.opened)
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
    return 0;
}

static void ctl_total_time(long tt)
{
    int mins, secs;
    if (ctl.trace_playing)
	{
	    secs=(int)(tt/play_mode->rate);
	    mins=secs/60;
	    secs-=mins*60;
	    cmsg(CMSG_INFO, VERB_NORMAL,
		 "Total playing time: %3d min %02d s", mins, secs);
	}
}

static void ctl_file_name(char *name)
{
    current_file = name;

    if (ctl.verbosity>=0 || ctl.trace_playing)
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
    if (ctl.trace_playing && secs != prev_secs)
	{
	    prev_secs = secs;
	    mins=secs/60;
	    secs-=mins*60;
	    //fprintf(outfp, "\r%3d:%02d", mins, secs);
	    //fflush(outfp);
	}
}

static void ctl_lyric(int lyricid)
{
    char *lyric;

    current_file_info = get_midi_file_info(current_file, 1);

    lyric = event2string(lyricid);
    if(lyric != NULL)
	{
	    if(lyric[0] == ME_KARAOKE_LYRIC)
		{
		    if(lyric[1] == '/' || lyric[1] == '\\')
			{
			    fprintf(outfp, "\n%s", lyric + 2);
			    fflush(outfp);
			}
		    else if(lyric[1] == '@')
			{
			    if(lyric[2] == 'L')
				fprintf(outfp, "\nLanguage: %s\n", lyric + 3);
			    else if(lyric[2] == 'T')
				fprintf(outfp, "Title: %s\n", lyric + 3);
			    else
				fprintf(outfp, "%s\n", lyric + 1);
			}
		    else
			{
			    fputs(lyric + 1, outfp);
			    fflush(outfp);
			}
		}
	    else
		{
		    if(lyric[0] == ME_CHORUS_TEXT || lyric[0] == ME_INSERT_TEXT)
			fprintf(outfp, "\r");
		    fputs(lyric + 1, outfp);
		    fflush(outfp);
		}
	}
}


static void ctl_event(CtlEvent *e)
{
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
	    break;
	case CTLE_PLAY_START:

	    ctl_total_time(e->v1);
	    break;
	case CTLE_CURRENT_TIME:
	    ctl_current_time((int)e->v1);
	    break;
#ifndef CFG_FOR_SF
	case CTLE_LYRIC:
	    ctl_lyric((int)e->v1);
	    break;
#endif
	}
}

/*
 * interface_<id>_loader();
 */
ControlMode *interface_v_loader(void)
{
    return &ctl;
}

