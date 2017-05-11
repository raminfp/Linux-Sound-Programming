/**
 * palist_devices.c 
 * Jan Newmarch
 */

#include <stdio.h>
#include <string.h>
#include <pulse/pulseaudio.h>

#define _(x) x

// quit when this reaches 2
int no_more_sources_or_sinks = 0;

int ret;

pa_context *context;

void show_error(char *s) {
    fprintf(stderr, "%s\n", s);
}

void print_properties(pa_proplist *props) {
    void *state = NULL;

    printf("  Properties are: \n");
    while (1) {
	const char *key;
	if ((key = pa_proplist_iterate(props, &state)) == NULL) {
	    return;
	}
	const char *value = pa_proplist_gets(props, key);
	printf("   key: %s, value: %s\n", key, value);
    }
}


/**
 * print information about a sink
 */
void sinklist_cb(pa_context *c, const pa_sink_info *i, int eol, void *userdata) {

    // If eol is set to a positive number, you're at the end of the list
    if (eol > 0) {
	printf("**No more sinks\n");
	no_more_sources_or_sinks++;
	if (no_more_sources_or_sinks == 2)
	    exit(0);
	return;
    }

    printf("Sink: name %s, description %s\n", i->name, i->description);
    print_properties(i->proplist);
}

/**
 * print information about a source
 */
void sourcelist_cb(pa_context *c, const pa_source_info *i, int eol, void *userdata) {
    if (eol > 0) {
	printf("**No more sources\n");
	no_more_sources_or_sinks++;
	if (no_more_sources_or_sinks == 2)
	    exit(0);
	return;
    }

    printf("Source: name %s, description %s\n", i->name, i->description);
    print_properties(i->proplist);
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

	// set up a callback to tell us about source devices
	if (!(o = pa_context_get_source_info_list(c,
					    sourcelist_cb,
					    NULL
						  ))) {
	    show_error(_("pa_context_subscribe() failed"));
	    return;
	}
	pa_operation_unref(o);

	// set up a callback to tell us about sink devices
	if (!(o = pa_context_get_sink_info_list(c,
					    sinklist_cb,
					    NULL
						  ))) {
	    show_error(_("pa_context_subscribe() failed"));
	    return;
	}
	pa_operation_unref(o);

	break;
    }

    case PA_CONTEXT_FAILED:
    case PA_CONTEXT_TERMINATED:
    default:
	return;
    }
}

int main(int argc, char *argv[]) {

    // Define our pulse audio loop and connection variables
    pa_mainloop *pa_ml;
    pa_mainloop_api *pa_mlapi;

    // Create a mainloop API and connection to the default server
    pa_ml = pa_mainloop_new();
    pa_mlapi = pa_mainloop_get_api(pa_ml);
    context = pa_context_new(pa_mlapi, "Device list");

    // This function connects to the pulse server
    pa_context_connect(context, NULL, 0, NULL);

    // This function defines a callback so the server will tell us its state.
    pa_context_set_state_callback(context, context_state_cb, NULL);

    if (pa_mainloop_run(pa_ml, &ret) < 0) {
	printf("pa_mainloop_run() failed.");
	exit(1);
    }
}
