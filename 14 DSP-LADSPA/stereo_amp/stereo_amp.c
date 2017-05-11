#include <gtk/gtk.h>

#include <stdlib.h>
#include <stdio.h>
#include <ladspa.h>
#include <dlfcn.h>
#include <sndfile.h>

#include "utils.h"

gint count = 0;
char buf[5];

pthread_t ladspa_thread;

const LADSPA_Descriptor * psDescriptor;
LADSPA_Descriptor_Function pfDescriptorFunction;
LADSPA_Handle handle;

// choose the mono plugin from the amp file
char *pcPluginFilename = "amp.so";
char *pcPluginLabel = "amp_stereo";

long lInputPortIndex = -1;
long lOutputPortIndex = -1;

int inBufferIndex = 0;
int outBufferIndex = 0;

SNDFILE* pInFile;
SNDFILE* pOutFile;

// for the amplifier, the sample rate doesn't really matter
#define SAMPLE_RATE 44100

// the buffer size isn't really important either
#define BUF_SIZE 2048
LADSPA_Data pInStereoBuffer[2*BUF_SIZE];
LADSPA_Data pOutStereoBuffer[2*BUF_SIZE];
LADSPA_Data pInBuffer[2][BUF_SIZE];
LADSPA_Data pOutBuffer[2][BUF_SIZE];

// How much we are amplifying the sound by
// We aren't allowed to change the control values
// during execution of run(). We could put a lock
// around run() or simpler, change the value of
// control only outside of run()
LADSPA_Data control;
LADSPA_Data pre_control = 0.2f;

char *pInFilePath = "/home/newmarch/Music/karaoke/nights/nightsinwhite-0.wav";
char *pOutFilePath = "tmp.wav";

void open_files() {
    // using libsndfile functions for easy read/write
    SF_INFO sfinfo;

    sfinfo.format = 0;
    pInFile = sf_open(pInFilePath, SFM_READ, &sfinfo);
    if (pInFile == NULL) {
	perror("can't open input file");
	exit(1);
    }

    pOutFile = sf_open(pOutFilePath, SFM_WRITE, &sfinfo);
    if (pOutFile == NULL) {
	perror("can't open output file");
	exit(1);
    }
}

sf_count_t fill_input_buffer() {
    int numread = sf_read_float(pInFile, pInStereoBuffer, 2*BUF_SIZE);

    // split frames into samples for each channel
    int n;
    for (n = 0; n < numread; n += 2) {
	pInBuffer[0][n/2] = pInStereoBuffer[n];
	pInBuffer[1][n/2] = pInStereoBuffer[n+1];
    }
    return numread/2;
}

void empty_output_buffer(sf_count_t numread) {
    // combine output samples back into frames
    int n;
    for (n = 0; n < 2*numread; n += 2) {
	pOutStereoBuffer[n] = pOutBuffer[0][n/2];
	pOutStereoBuffer[n+1] = pOutBuffer[1][n/2];
    }

    sf_write_float(pOutFile, pOutStereoBuffer, 2*numread);
}

gpointer run_plugin(gpointer args) {
    sf_count_t numread;

    // it's NULL for the amp plugin
    if (psDescriptor->activate != NULL)
	psDescriptor->activate(handle);

    while ((numread = fill_input_buffer()) > 0) {
	// reset control outside of run()
	control = pre_control;

	psDescriptor->run(handle, numread);
	empty_output_buffer(numread);
	usleep(1000);
    }
    printf("Plugin finished!\n");
}

void setup_ladspa() {
    int lPluginIndex;

    void *pvPluginHandle = loadLADSPAPluginLibrary(pcPluginFilename);
    dlerror();

    pfDescriptorFunction 
	= (LADSPA_Descriptor_Function)dlsym(pvPluginHandle, "ladspa_descriptor");
    if (!pfDescriptorFunction) {
	const char * pcError = dlerror();
	if (pcError) 
	    fprintf(stderr,
		    "Unable to find ladspa_descriptor() function in plugin file "
		    "\"%s\": %s.\n"
		    "Are you sure this is a LADSPA plugin file?\n", 
		    pcPluginFilename,
		    pcError);
	exit(1);
    }

    for (lPluginIndex = 0;; lPluginIndex++) {
	psDescriptor = pfDescriptorFunction(lPluginIndex);
	if (!psDescriptor)
	    break;
	if (pcPluginLabel != NULL) {
	    if (strcmp(pcPluginLabel, psDescriptor->Label) != 0)
		continue;
	}
	// got stero_amp

	handle = psDescriptor->instantiate(psDescriptor, SAMPLE_RATE);
	if (handle == NULL) {
	    fprintf(stderr, "Can't instantiate plugin %s\n", pcPluginLabel);
	    exit(1);
	}

	// get ports
	int lPortIndex;
	printf("Num ports %lu\n", psDescriptor->PortCount);
	for (lPortIndex = 0; 
	     lPortIndex < psDescriptor->PortCount; 
	     lPortIndex++) {
	    if (LADSPA_IS_PORT_AUDIO
		(psDescriptor->PortDescriptors[lPortIndex])) {
		if (LADSPA_IS_PORT_INPUT
		    (psDescriptor->PortDescriptors[lPortIndex])) {
		    printf("input %d\n", lPortIndex);
		    lInputPortIndex = lPortIndex;
		    
		    psDescriptor->connect_port(handle,
					       lInputPortIndex, pInBuffer[inBufferIndex++]);
		} else if (LADSPA_IS_PORT_OUTPUT
			   (psDescriptor->PortDescriptors[lPortIndex])) {
		    printf("output %d\n", lPortIndex);
		    lOutputPortIndex = lPortIndex;
		    
		    psDescriptor->connect_port(handle,
					       lOutputPortIndex, pOutBuffer[outBufferIndex++]);
		}
	    }

	    if (LADSPA_IS_PORT_CONTROL
		(psDescriptor->PortDescriptors[lPortIndex])) {
		printf("control %d\n", lPortIndex);
		psDescriptor->connect_port(handle,			    
					   lPortIndex, &control);
	    }
	}
	// we've got what we wanted, get out of this loop
	break;
    }

    if ((psDescriptor == NULL) ||
	(lInputPortIndex == -1) ||
	(lOutputPortIndex == -1)) {
	fprintf(stderr, "Can't find plugin information\n");
	exit(1);
    }

    open_files();

    pthread_create(&ladspa_thread, NULL, run_plugin, NULL);
}

void slider_change(GtkAdjustment *adj,  gpointer data)
{
    count++;

    pre_control = gtk_adjustment_get_value(adj);
    //gtk_label_set_text(GTK_LABEL(label), buf);
}

int main(int argc, char** argv) {

    //GtkWidget *label;
    GtkWidget *window;
    GtkWidget *frame;
    GtkWidget *slider;
    GtkAdjustment *adjustment;

    setup_ladspa();

    gtk_init(&argc, &argv);

    window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);
    gtk_window_set_default_size(GTK_WINDOW(window), 250, 80);
    gtk_window_set_title(GTK_WINDOW(window), "Volume");

    frame = gtk_fixed_new();
    gtk_container_add(GTK_CONTAINER(window), frame);

    adjustment = gtk_adjustment_new(1.0,
                               0.0,
                               2.0,
                               0.1,
                               1.0,
                               0.0);
    slider = gtk_scale_new(GTK_ORIENTATION_HORIZONTAL, 
			 adjustment);
    gtk_widget_set_size_request(slider, 240, 5);
    gtk_fixed_put(GTK_FIXED(frame), slider, 5, 20);


    
    //label = gtk_label_new("0");
    //gtk_fixed_put(GTK_FIXED(frame), label, 190, 58); 

    gtk_widget_show_all(window);

    g_signal_connect(window, "destroy",
		     G_CALLBACK (gtk_main_quit), NULL);

    g_signal_connect(adjustment, "value-changed", 
		     G_CALLBACK(slider_change), NULL);

    gtk_main();

    return 0;
}
      
