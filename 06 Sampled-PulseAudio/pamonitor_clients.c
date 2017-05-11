/**
 * pamonitor_clients.c 
 * Jan Newmarch
 */



#include <stdio.h>
#include <string.h>
#include <pulse/pulseaudio.h>

#define CLEAR_LINE "\n"
#define _(x) x

// From pulsecore/macro.h
#define pa_memzero(x,l) (memset((x), 0, (l)))
#define pa_zero(x) (pa_memzero(&(x), sizeof(x)))

int ret;

pa_context *context;

void show_error(char *s) {
    /* stub */
}

void add_client_cb(pa_context *context, const pa_client_info *i, int eol, void *userdata) {

    if (eol < 0) {
        if (pa_context_errno(context) == PA_ERR_NOENTITY)
            return;

        show_error(_("Client callback failure"));
        return;
    }

    if (eol > 0) {
        return;
    }

    printf("Found a new client index %d name %s eol %d\n", i->index, i->name, eol);
}

void remove_client_cb(pa_context *context, const pa_client_info *i, int eol, void *userdata) {

    if (eol < 0) {
        if (pa_context_errno(context) == PA_ERR_NOENTITY)
            return;

        show_error(_("Client callback failure"));
        return;
    }

    if (eol > 0) {
        return;
    }

    printf("Removing a client index %d name %s\n", i->index, i->name);
}

void sink_input_cb(pa_context *c, const pa_sink_input_info *i, int eol, void *userdata) {
    if (eol < 0) {
        if (pa_context_errno(context) == PA_ERR_NOENTITY)
            return;

        show_error(_("Sink input callback failure"));
        return;
    }

    if (eol > 0) {
        return;
    }
    printf("Sink input found index %d name %s for client %d\n", i->index, i->name, i->client);
}

void source_output_cb(pa_context *c, const pa_source_output_info *i, int eol, void *userdata) {
    if (eol < 0) {
        if (pa_context_errno(context) == PA_ERR_NOENTITY)
            return;

        show_error(_("Source output callback failure"));
        return;
    }

    if (eol > 0) {
        return;
    }
    printf("Source output found index %d name %s for client %d\n", i->index, i->name, i->client);
}


void subscribe_cb(pa_context *c, pa_subscription_event_type_t t, uint32_t index, void *userdata) {

    switch (t & PA_SUBSCRIPTION_EVENT_FACILITY_MASK) {

    case PA_SUBSCRIPTION_EVENT_CLIENT:
	if ((t & PA_SUBSCRIPTION_EVENT_TYPE_MASK) == PA_SUBSCRIPTION_EVENT_REMOVE) {
	    printf("Remove event at index %d\n", index);
	    pa_operation *o;
	    if (!(o = pa_context_get_client_info(c, index, remove_client_cb, NULL))) {
		show_error(_("pa_context_get_client_info() failed"));
		return;
	    }
	    pa_operation_unref(o);

	} else {
	    pa_operation *o;
	    if (!(o = pa_context_get_client_info(c, index, add_client_cb, NULL))) {
		show_error(_("pa_context_get_client_info() failed"));
		return;
	    }
	    pa_operation_unref(o);
	}
	break;

    case PA_SUBSCRIPTION_EVENT_SINK_INPUT:
	if ((t & PA_SUBSCRIPTION_EVENT_TYPE_MASK) == PA_SUBSCRIPTION_EVENT_REMOVE)
	    printf("Removing sink input %d\n", index);
	else {
	    pa_operation *o;
	    if (!(o = pa_context_get_sink_input_info(context, index, sink_input_cb, NULL))) {
		show_error(_("pa_context_get_sink_input_info() failed"));
		return;
	    }
	    pa_operation_unref(o);
	}
	break;

    case PA_SUBSCRIPTION_EVENT_SOURCE_OUTPUT:
	if ((t & PA_SUBSCRIPTION_EVENT_TYPE_MASK) == PA_SUBSCRIPTION_EVENT_REMOVE)
	    printf("Removing source output %d\n", index);
	else {
	    pa_operation *o;
	    if (!(o = pa_context_get_source_output_info(context, index, source_output_cb, NULL))) {
		show_error(_("pa_context_get_sink_input_info() failed"));
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

	pa_context_set_subscribe_callback(c, subscribe_cb, NULL);

	if (!(o = pa_context_subscribe(c, (pa_subscription_mask_t)
				       (PA_SUBSCRIPTION_MASK_CLIENT | 
					PA_SUBSCRIPTION_MASK_SINK_INPUT | 
					PA_SUBSCRIPTION_MASK_SOURCE_OUTPUT), NULL, NULL))) {
	    show_error(_("pa_context_subscribe() failed"));
	    return;
	}
	pa_operation_unref(o);

	if (!(o = pa_context_get_client_info_list(context,
						  add_client_cb,
						  NULL
	) )) {
	    show_error(_("pa_context_subscribe() failed"));
	    return;
	}
	pa_operation_unref(o);
	
	break;
    }

    case PA_CONTEXT_FAILED:
	return;

    case PA_CONTEXT_TERMINATED:
    default:
	// Gtk::Main::quit();
	return;
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
	break;

    case PA_STREAM_FAILED:
    default:
	printf("Stream error: %s", pa_strerror(pa_context_errno(pa_stream_get_context(s))));
	exit(1);
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
    //pa_context_set_state_callback(context, state_cb, NULL);
    pa_context_set_state_callback(context, context_state_cb, NULL);

    
    if (pa_mainloop_run(pa_ml, &ret) < 0) {
	printf("pa_mainloop_run() failed.");
	exit(1);
    }
}
