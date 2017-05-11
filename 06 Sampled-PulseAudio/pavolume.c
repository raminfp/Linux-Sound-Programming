/**
 * pavolume.c 
 * Jan Newmarch
 */

#include <stdio.h>
#include <string.h>
#include <pulse/pulseaudio.h>

#define _(x) x

char *device = "alsa_output.pci-0000_00_1b.0.analog-stereo";

int ret;

pa_context *context;

void show_error(char *s) {
    fprintf(stderr, "%s\n", s);
}


void volume_cb(pa_context *c, int success, void *userdata) {
    if (success) 
	printf("Volume set\n");
    else
	printf("Volume not set\n");
}
		       
void context_state_cb(pa_context *c, void *userdata) {

    switch (pa_context_get_state(c)) {
    case PA_CONTEXT_UNCONNECTED:
    case PA_CONTEXT_CONNECTING:
    case PA_CONTEXT_AUTHORIZING:
    case PA_CONTEXT_SETTING_NAME:
	break;

    case PA_CONTEXT_READY: {
	pa_operation *o;

	break;
    }

    case PA_CONTEXT_FAILED:
    case PA_CONTEXT_TERMINATED:
    default:
	return;
    }
}

int main(int argc, char *argv[]) {
    long volume = 0;
    char buf[128];
    struct pa_cvolume v;

    // Define our pulse audio loop and connection variables
    pa_threaded_mainloop *pa_ml;
    pa_mainloop_api *pa_mlapi;

    // Create a mainloop API and connection to the default server
    //pa_ml = pa_mainloop_new();
    pa_ml = pa_threaded_mainloop_new();
    pa_mlapi = pa_threaded_mainloop_get_api(pa_ml);
    context = pa_context_new(pa_mlapi, "Voulme control");

    // This function connects to the pulse server
    pa_context_connect(context, NULL, 0, NULL);

    // This function defines a callback so the server will tell us its state.
    pa_context_set_state_callback(context, context_state_cb, NULL);

    pa_threaded_mainloop_start(pa_ml);
    printf("Enter volume for device\n");
    
    pa_cvolume_init(&v);
    while (1) {
        puts("Enter an integer 0-65536\n");
        fgets(buf, 128, stdin);
        volume = atoi(buf);
	pa_cvolume_set(&v, 2, volume);
	pa_context_set_sink_volume_by_name(context,
					   device,
					   &v,
					   volume_cb,
					   NULL
					   );
    }
}
