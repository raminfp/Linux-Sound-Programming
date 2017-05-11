
/**
 * pacat2.c 
 * Jan Newmarch
 */


/***
  This file is based on pacat.c,  part of PulseAudio.

  pacat.c:
  Copyright 2004-2006 Lennart Poettering
  Copyright 2006 Pierre Ossman <ossman@cendio.se> for Cendio AB

  PulseAudio is free software; you can redistribute it and/or modify
  it under the terms of the GNU Lesser General Public License as published
  by the Free Software Foundation; either version 2.1 of the License,
  or (at your option) any later version.

  PulseAudio is distributed in the hope that it will be useful, but
  WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
  General Public License for more details.

  You should have received a copy of the GNU Lesser General Public License
  along with PulseAudio; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
  USA.
***/


#include <stdio.h>
#include <string.h>
#include <pulse/pulseaudio.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>

// ???
#define CLEAR_LINE "\n"

// From pulsecore/macro.h JN
#define pa_memzero(x,l) (memset((x), 0, (l)))
#define pa_zero(x) (pa_memzero(&(x), sizeof(x)))

int verbose = 1;
int ret;

static pa_volume_t volume = PA_VOLUME_NORM;
static int volume_is_set = 0;

static int fdin;

static pa_sample_spec sample_spec = {
  .format = PA_SAMPLE_S16LE,
  .rate = 44100,
  .channels = 2
};

static pa_stream *stream = NULL;
static pa_channel_map channel_map;
static pa_proplist *proplist = NULL;

// Define our pulse audio loop and connection variables
static pa_mainloop *mainloop;
static pa_mainloop_api *mainloop_api;
static pa_operation *pa_op;
static pa_context *context = NULL;

static void *buffer = NULL;
static size_t buffer_length = 0, buffer_index = 0;

static pa_io_event* stdio_event = NULL;

// Get device name from e.g. paman
//static char *device = "alsa_output.pci-0000_00_1b.0.analog-stereo";
// Use default device
static char *device = NULL;

static pa_stream_flags_t flags = 0;

static size_t latency = 0, process_time = 0;
static int32_t latency_msec = 1, process_time_msec = 0;

static int raw = 1;


/* Connection draining complete */
static void context_drain_complete(pa_context*c, void *userdata) {
  pa_context_disconnect(c);
}

static void stream_drain_complete(pa_stream*s, int success, void *userdata) {
  pa_operation *o = NULL;

  if (!success) {
    printf("Failed to drain stream: %s", pa_strerror(pa_context_errno(context)));
    exit(1);
  }

  if (verbose)
    printf("Playback stream drained.");

  pa_stream_disconnect(stream);
  pa_stream_unref(stream);
  stream = NULL;

  if (!(o = pa_context_drain(context, context_drain_complete, NULL)))
    pa_context_disconnect(context);
  else {
    pa_operation_unref(o);
    if (verbose)
      printf("Draining connection to server.");
  }
}


/* Start draining */
static void start_drain(void) {
  printf("Draining\n");
  if (stream) {
    pa_operation *o;

    pa_stream_set_write_callback(stream, NULL, NULL);

    if (!(o = pa_stream_drain(stream, stream_drain_complete, NULL))) {
      //printf("pa_stream_drain(): %s", pa_strerror(pa_context_errno(context)));
      exit(1);
      return;
    }

    pa_operation_unref(o);
  } else
    exit(0);
}

/* Write some data to the stream */
static void do_stream_write(size_t length) {
  size_t l;
  assert(length);

  printf("do stream write: Writing %lu to stream\n", length);
  
  if (!buffer || !buffer_length) {
    buffer = pa_xmalloc(length);
    buffer_length = length;
    buffer_index = 0;
    //printf("  return without writing\n");
    //return;

  }

  while (buffer_length > 0) {
    l = read(fdin, buffer + buffer_index, buffer_length);
    if (l <= 0) {
      start_drain();
      return;
    }
    if (pa_stream_write(stream, (uint8_t*) buffer + buffer_index, l, NULL, 0, PA_SEEK_RELATIVE) < 0) {
      printf("pa_stream_write() failed: %s", pa_strerror(pa_context_errno(context)));
      exit(1);
      return;
    }
    buffer_length -= l;
    buffer_index += l;

    if (!buffer_length) {
      pa_xfree(buffer);
      buffer = NULL;
      buffer_index = buffer_length = 0;
    }
  }
}


void stream_state_callback(pa_stream *s, void *userdata) {
  assert(s);

  switch (pa_stream_get_state(s)) {
  case PA_STREAM_CREATING:
    break;
  case PA_STREAM_TERMINATED:
    break;

  case PA_STREAM_READY:

    if (verbose) {
      const pa_buffer_attr *a;
      char cmt[PA_CHANNEL_MAP_SNPRINT_MAX], sst[PA_SAMPLE_SPEC_SNPRINT_MAX];

      printf("Stream successfully created.\n");

      if (!(a = pa_stream_get_buffer_attr(s)))
	printf("pa_stream_get_buffer_attr() failed: %s\n", pa_strerror(pa_context_errno(pa_stream_get_context(s))));
      else {
	printf("Buffer metrics: maxlength=%u, fragsize=%u\n", a->maxlength, a->fragsize);
                    
      }
      /*
	printf("Using sample spec '%s', channel map '%s'.",
	pa_sample_spec_snprint(sst, sizeof(sst), pa_stream_get_sample_spec(s)),
	pa_channel_map_snprint(cmt, sizeof(cmt), pa_stream_get_channel_map(s)));
      */

      printf("Connected to device %s (%u, %ssuspended).\n",
	     pa_stream_get_device_name(s),
	     pa_stream_get_device_index(s),
	     pa_stream_is_suspended(s) ? "" : "not ");
    }

    break;

  case PA_STREAM_FAILED:
  default:
    printf("Stream error: %s", pa_strerror(pa_context_errno(pa_stream_get_context(s))));
    exit(1); //quit(1);
  }
}


/*********** Stream callbacks **************/


static void stream_success(pa_stream *s, int succes, void *userdata) {
  printf("Succeded\n");
}


static void stream_suspended_callback(pa_stream *s, void *userdata) {
  assert(s);

  if (verbose) {
    if (pa_stream_is_suspended(s))
      fprintf(stderr, "Stream device suspended.%s \n", CLEAR_LINE);
    else
      fprintf(stderr, "Stream device resumed.%s \n", CLEAR_LINE);
  }
}

static void stream_underflow_callback(pa_stream *s, void *userdata) {
  assert(s);

  if (verbose)
    fprintf(stderr, "Stream underrun.%s \n",  CLEAR_LINE);
}

static void stream_overflow_callback(pa_stream *s, void *userdata) {
  assert(s);

  if (verbose)
    fprintf(stderr, "Stream overrun.%s \n", CLEAR_LINE);
}

static void stream_started_callback(pa_stream *s, void *userdata) {
  assert(s);

  if (verbose)
    fprintf(stderr, "Stream started.%s \n", CLEAR_LINE);
}

static void stream_moved_callback(pa_stream *s, void *userdata) {
  assert(s);

  if (verbose)
    fprintf(stderr, "Stream moved to device %s (%u, %ssuspended).%s \n", pa_stream_get_device_name(s), pa_stream_get_device_index(s), pa_stream_is_suspended(s) ? "" : "not ",  CLEAR_LINE);
}

static void stream_buffer_attr_callback(pa_stream *s, void *userdata) {
  assert(s);

  if (verbose)
    fprintf(stderr, "Stream buffer attributes changed.%s \n",  CLEAR_LINE);
}

static void stream_event_callback(pa_stream *s, const char *name, pa_proplist *pl, void *userdata) {
  char *t;

  assert(s);
  assert(name);
  assert(pl);

  t = pa_proplist_to_string_sep(pl, ", ");
  fprintf(stderr, "Got event '%s', properties '%s'\n", name, t);
  pa_xfree(t);
}


/* This is called whenever new data may be written to the stream */
static void stream_write_callback(pa_stream *s, size_t length, void *userdata) {
  //assert(s);
  //assert(length > 0);

  printf("Stream write callback: Ready to write %lu bytes\n", length);
  
  printf("  do stream write from stream write callback\n");
  do_stream_write(length);

 }


// This callback gets called when our context changes state.  We really only
// care about when it's ready or if it has failed
void state_cb(pa_context *c, void *userdata) {
  pa_context_state_t state;
  int *pa_ready = userdata;

  printf("State changed\n");
  state = pa_context_get_state(c);
  switch  (state) {
    // There are just here for reference
  case PA_CONTEXT_UNCONNECTED:
  case PA_CONTEXT_CONNECTING:
  case PA_CONTEXT_AUTHORIZING:
  case PA_CONTEXT_SETTING_NAME:
  default:
    break;
  case PA_CONTEXT_FAILED:
  case PA_CONTEXT_TERMINATED:
    *pa_ready = 2;
    break;
  case PA_CONTEXT_READY: {
    pa_buffer_attr buffer_attr;

    if (verbose)
      printf("Connection established.%s\n", CLEAR_LINE);

    if (!(stream = pa_stream_new(c, "JanPlayback", &sample_spec, NULL))) {
      printf("pa_stream_new() failed: %s", pa_strerror(pa_context_errno(c)));
      exit(1); // goto fail;
    }

    pa_stream_set_state_callback(stream, stream_state_callback, NULL);
    
    pa_stream_set_write_callback(stream, stream_write_callback, NULL);
    
    //pa_stream_set_read_callback(stream, stream_read_callback, NULL);
    
    pa_stream_set_suspended_callback(stream, stream_suspended_callback, NULL);
    pa_stream_set_moved_callback(stream, stream_moved_callback, NULL);
    pa_stream_set_underflow_callback(stream, stream_underflow_callback, NULL);
    pa_stream_set_overflow_callback(stream, stream_overflow_callback, NULL);
    
    pa_stream_set_started_callback(stream, stream_started_callback, NULL);
    
    pa_stream_set_event_callback(stream, stream_event_callback, NULL);
    pa_stream_set_buffer_attr_callback(stream, stream_buffer_attr_callback, NULL);
    
    

    pa_zero(buffer_attr);
    buffer_attr.maxlength = (uint32_t) -1;
    buffer_attr.prebuf = (uint32_t) -1;
    
    pa_cvolume cv;
      



    if (pa_stream_connect_playback(stream, NULL, &buffer_attr, flags,
				   NULL, 
				   NULL) < 0) {
      printf("pa_stream_connect_playback() failed: %s", pa_strerror(pa_context_errno(c)));
      exit(1); //goto fail;
    } else {
      printf("Set playback callback\n");
    }

    pa_stream_trigger(stream, stream_success, NULL);
  }

    break;
  }
}


int main(int argc, char *argv[]) {


  struct stat st;
  off_t size;
  ssize_t nread;

  // We'll need these state variables to keep track of our requests
  int state = 0;
  int pa_ready = 0;

  if (argc != 2) {
    fprintf(stderr, "Usage: %s file\n", argv[0]);
    exit(1);
  }
  // slurp the whole file into buffer
  if ((fdin = open(argv[1],  O_RDONLY)) == -1) {
    perror("open");
    exit(1);
  }

  // Create a mainloop API and connection to the default server
  mainloop = pa_mainloop_new();
  mainloop_api = pa_mainloop_get_api(mainloop);
  context = pa_context_new(mainloop_api, "test");

  // This function connects to the pulse server
  pa_context_connect(context, NULL, 0, NULL);
  printf("Connecting\n");

  // This function defines a callback so the server will tell us it's state.
  // Our callback will wait for the state to be ready.  The callback will
  // modify the variable to 1 so we know when we have a connection and it's
  // ready.
  // If there's an error, the callback will set pa_ready to 2
  pa_context_set_state_callback(context, state_cb, &pa_ready);


  if (pa_mainloop_run(mainloop, &ret) < 0) {
    printf("pa_mainloop_run() failed.");
    exit(1); // goto quit
  }



}
