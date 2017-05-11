/** @file delay.c
 *
 * @brief This client delays one channel by 4096 framse.
 */

#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <signal.h>
#ifndef WIN32
#include <unistd.h>
#endif
#include <jack/jack.h>

jack_client_t *client;

void print_port_info(char *name) {
    printf("Port name is %s\n", name);
    jack_port_t *port =	jack_port_by_name (client, name);
    if (port == NULL) {
	printf("No port by name %s\n", name);
	return;
    }
    printf("  Type is %s\n", jack_port_type(port));

    int flags = jack_port_flags(port);
    if (flags & JackPortIsInput) 
	printf("  Is an input port\n");
    else
	printf("  Is an output port\n");
    char **connections = jack_port_get_connections(port);
    char **c = connections;
    printf("  Connected to:\n");
    while ((c != NULL) && (*c != NULL)) {
	printf("    %s\n", *c++);
    }
    if (connections != NULL)
	jack_free(connections);
}

int
main ( int argc, char *argv[] )
{
    int i;
    const char **ports;
    const char *client_name;
    const char *server_name = NULL;
    jack_options_t options = JackNullOption;
    jack_status_t status;

    if ( argc >= 2 )        /* client name specified? */
    {
        client_name = argv[1];
        if ( argc >= 3 )    /* server name specified? */
        {
            server_name = argv[2];
            options |= JackServerName;
        }
    }
    else              /* use basename of argv[0] */
    {
        client_name = strrchr ( argv[0], '/' );
        if ( client_name == 0 )
        {
            client_name = argv[0];
        }
        else
        {
            client_name++;
        }
    }

    /* open a client connection to the JACK server */

    client = jack_client_open ( client_name, options, &status, server_name );
    if ( client == NULL )
    {
        fprintf ( stderr, "jack_client_open() failed, "
                  "status = 0x%2.0x\n", status );
        if ( status & JackServerFailed )
        {
            fprintf ( stderr, "Unable to connect to JACK server\n" );
        }
        exit ( 1 );
    }
    if ( status & JackServerStarted )
    {
        fprintf ( stderr, "JACK server started\n" );
    }
    if ( status & JackNameNotUnique )
    {
        client_name = jack_get_client_name ( client );
        fprintf ( stderr, "unique name `%s' assigned\n", client_name );
    }

    if ( jack_activate ( client ) )
    {
        fprintf ( stderr, "cannot activate client" );
        exit ( 1 );
    }

     ports = jack_get_ports ( client, NULL, NULL, 0 );
    if ( ports == NULL )
    {
        fprintf ( stderr, "no ports\n" );
        exit ( 1 );
    }
    char **p = ports;
    while (*p != NULL)
	print_port_info(*p++);
    jack_free(ports);

    jack_client_close ( client );
    exit ( 0 );
}
