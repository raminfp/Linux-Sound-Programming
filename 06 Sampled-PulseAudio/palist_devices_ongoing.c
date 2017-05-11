/**
 * palist_clients.c 
 * Jan Newmarch
 */

/***
    This file is based on pacat.c and pavuctl.c,  part of PulseAudio.

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

#define _(x) x

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
void sink_cb(pa_context *c, const pa_sink_info *i, int eol, void *userdata) {

    // If eol is set to a positive number, you're at the end of the list
    if (eol > 0) {
	return;
    }

    printf("Sink: name %s, description %s\n", i->name, i->description);
    // print_properties(i->proplist);
}

/**
 * print information about a source
 */
void source_cb(pa_context *c, const pa_source_info *i, int eol, void *userdata) {
    if (eol > 0) {
	return;
    }

    printf("Source: name %s, description %s\n", i->name, i->description);
    // print_properties(i->proplist);
}

void subscribe_cb(pa_context *c, pa_subscription_event_type_t t, uint32_t index, void *userdata) {

    switch (t & PA_SUBSCRIPTION_EVENT_FACILITY_MASK) {
	
    case PA_SUBSCRIPTION_EVENT_SINK:
	if ((t & PA_SUBSCRIPTION_EVENT_TYPE_MASK) == PA_SUBSCRIPTION_EVENT_REMOVE)
	    printf("Removing sink index %d\n", index);
	else {
	    pa_operation *o;
	    if (!(o = pa_context_get_sink_info_by_index(c, index, sink_cb, NULL))) {
		show_error(_("pa_context_get_sink_info_by_index() failed"));
		return;
	    }
	    pa_operation_unref(o);
	}
	break;

    case PA_SUBSCRIPTION_EVENT_SOURCE:
	if ((t & PA_SUBSCRIPTION_EVENT_TYPE_MASK) == PA_SUBSCRIPTION_EVENT_REMOVE)
	    printf("Removing source index %d\n", index);
	else {
	    pa_operation *o;
	    if (!(o = pa_context_get_source_info_by_index(c, index, source_cb, NULL))) {
		show_error(_("pa_context_get_source_info_by_index() failed"));
		return;
	    }
	    pa_operation_unref(o);
	}
	break;
    }
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

	if (!(o = pa_context_get_source_info_list(c,
						  source_cb,
						  NULL
						  ))) {
	    show_error(_("pa_context_subscribe() failed"));
	    return;
	}
	pa_operation_unref(o);

	if (!(o = pa_context_get_sink_info_list(c,
						sink_cb,
						NULL
						))) {
	    show_error(_("pa_context_subscribe() failed"));
	    return;
	}
	pa_operation_unref(o);

	pa_context_set_subscribe_callback(c, subscribe_cb, NULL);

	if (!(o = pa_context_subscribe(c, (pa_subscription_mask_t)
				       (PA_SUBSCRIPTION_MASK_SINK|
					PA_SUBSCRIPTION_MASK_SOURCE), NULL, NULL))) {
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
    pa_operation *pa_op;
    pa_time_event *time_event;

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
