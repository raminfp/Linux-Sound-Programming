/**
 * parec.c 
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
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <pulse/pulseaudio.h>

#define CLEAR_LINE "\n"
#define _(x) x

// From pulsecore/macro.h
#define pa_memzero(x,l) (memset((x), 0, (l)))
#define pa_zero(x) (pa_memzero(&(x), sizeof(x)))

int fdout;
char *fname = "tmp.s16";

int verbose = 1;
int ret;

pa_context *context;

static pa_sample_spec sample_spec = {
  .format = PA_SAMPLE_S16LE,
  .rate = 44100,
  .channels = 2
};

static pa_stream *stream = NULL;

/* This is my builtin card. Use paman to find yours
   or set it to NULL to get the default device
*/
static char *device = "alsa_input.pci-0000_00_1b.0.analog-stereo";

static pa_stream_flags_t flags = 0;

void stream_state_callback(pa_stream *s, void *userdata) {
  assert(s);

  switch (pa_stream_get_state(s)) {
  case PA_STREAM_CREATING:
    // The stream has been created, so 
    // let's open a file to record to
    printf("Creating stream\n");
    fdout = creat(fname,  0711);
    break;

  case PA_STREAM_TERMINATED:
    close(fdout);
    break;

  case PA_STREAM_READY:

    // Just for info: no functionality in this branch
    if (verbose) {
      const pa_buffer_attr *a;
      char cmt[PA_CHANNEL_MAP_SNPRINT_MAX], sst[PA_SAMPLE_SPEC_SNPRINT_MAX];

      printf("Stream successfully created.");

      if (!(a = pa_stream_get_buffer_attr(s)))
	printf("pa_stream_get_buffer_attr() failed: %s", pa_strerror(pa_context_errno(pa_stream_get_context(s))));
      else {
	printf("Buffer metrics: maxlength=%u, fragsize=%u", a->maxlength, a->fragsize);
                    
      }

      printf("Connected to device %s (%u, %ssuspended).",
	     pa_stream_get_device_name(s),
	     pa_stream_get_device_index(s),
	     pa_stream_is_suspended(s) ? "" : "not ");
    }

    break;

  case PA_STREAM_FAILED:
  default:
    printf("Stream error: %s", pa_strerror(pa_context_errno(pa_stream_get_context(s))));
    exit(1);
  }
}

/*********** Stream callbacks **************/

/* This is called whenever new data is available */
static void stream_read_callback(pa_stream *s, size_t length, void *userdata) {

  assert(s);
  assert(length > 0);

  // Copy the data from the server out to a file
  fprintf(stderr, "Can read %lu\n", length);


  while (pa_stream_readable_size(s) > 0) {
    const void *data;
    size_t length;
    
    // peek actually creates and fills the data vbl
    if (pa_stream_peek(s, &data, &length) < 0) {
      fprintf(stderr, "Read failed\n");
      exit(1);
      return;
    }
    fprintf(stderr, "Writing %lu\n", length);
    write(fdout, data, length);

    // swallow the data peeked at before
    pa_stream_drop(s);
  }
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

    if (!(stream = pa_stream_new(c, "JanCapture", &sample_spec, NULL))) {
      printf("pa_stream_new() failed: %s", pa_strerror(pa_context_errno(c)));
      exit(1);
    }

    // Watch for changes in the stream state to create the output file
    pa_stream_set_state_callback(stream, stream_state_callback, NULL);
    
    // Watch for changes in the stream's read state to write to the output file
    pa_stream_set_read_callback(stream, stream_read_callback, NULL);

    // Set properties of the record buffer
    pa_zero(buffer_attr);
    buffer_attr.maxlength = (uint32_t) -1;
    buffer_attr.prebuf = (uint32_t) -1;
    buffer_attr.fragsize = buffer_attr.tlength = (uint32_t) -1;
    buffer_attr.minreq = (uint32_t) -1;
      
    // and start recording
    if (pa_stream_connect_record(stream, device, &buffer_attr, flags) < 0) {
      printf("pa_stream_connect_record() failed: %s", pa_strerror(pa_context_errno(c)));
      exit(1);
    }
  }

    break;
  }
}

int main(int argc, char *argv[]) {

  // Define our pulse audio loop and connection variables
  pa_mainloop *pa_ml;
  pa_mainloop_api *pa_mlapi;
  pa_operation *pa_op;
  pa_time_event *time_event;

  // Create a mainloop API and connection to the default server
  pa_ml = pa_mainloop_new();
  pa_mlapi = pa_mainloop_get_api(pa_ml);
  context = pa_context_new(pa_mlapi, "test");

  // This function connects to the pulse server
  pa_context_connect(context, NULL, 0, NULL);

  // This function defines a callback so the server will tell us its state.
  pa_context_set_state_callback(context, state_cb, NULL);

  if (pa_mainloop_run(pa_ml, &ret) < 0) {
    printf("pa_mainloop_run() failed.");
    exit(1);
  }
}
