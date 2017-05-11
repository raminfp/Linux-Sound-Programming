
#include <stdlib.h>
#include <stdio.h>
#include <ladspa.h>
#include <dlfcn.h>
#include <sndfile.h>

#include "utils.h"

const LADSPA_Descriptor * psDescriptor;
LADSPA_Descriptor_Function pfDescriptorFunction;
LADSPA_Handle handle;

// choose the mono plugin from the amp file
char *pcPluginFilename = "amp.so";
char *pcPluginLabel = "amp_mono";

long lInputPortIndex = -1;
long lOutputPortIndex = -1;

SNDFILE* pInFile;
SNDFILE* pOutFile;

// for the amplifier, the sample rate doesn't really matter
#define SAMPLE_RATE 44100

// the buffer size isn't really important either
#define BUF_SIZE 2048
LADSPA_Data pInBuffer[BUF_SIZE];
LADSPA_Data pOutBuffer[BUF_SIZE];

// How much we are amplifying the sound by
LADSPA_Data control = 0.5f;

char *pInFilePath = "/home/local/antialize-wkhtmltopdf-7cb5810/scripts/static-build/linux-local/qts/demos/mobile/quickhit/plugins/LevelTemplate/sound/enableship.wav";
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
    return sf_read_float(pInFile, pInBuffer, BUF_SIZE);
}

void empty_output_buffer(sf_count_t numread) {
    sf_write_float(pOutFile, pOutBuffer, numread);
}

void run_plugin() {
    sf_count_t numread;

    open_files();

    // it's NULL for the amp plugin
    if (psDescriptor->activate != NULL)
	psDescriptor->activate(handle);

    while ((numread = fill_input_buffer()) > 0) {
	printf("Num read %d\n", numread);
	psDescriptor->run(handle, numread);
	empty_output_buffer(numread);
    }
}

int main(int argc, char *argv[]) {
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
	return 1;
    }

    for (lPluginIndex = 0;; lPluginIndex++) {
	psDescriptor = pfDescriptorFunction(lPluginIndex);
	if (!psDescriptor)
	    break;
	if (pcPluginLabel != NULL) {
	    if (strcmp(pcPluginLabel, psDescriptor->Label) != 0)
		continue;
	}
	// got mono_amp

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
	    if (LADSPA_IS_PORT_INPUT
		(psDescriptor->PortDescriptors[lPortIndex])
		&& LADSPA_IS_PORT_AUDIO
		(psDescriptor->PortDescriptors[lPortIndex])) {
		printf("input %d\n", lPortIndex);
		lInputPortIndex = lPortIndex;

		psDescriptor->connect_port(handle,
					   lInputPortIndex, pInBuffer);
	    } else if (LADSPA_IS_PORT_OUTPUT
		       (psDescriptor->PortDescriptors[lPortIndex])
		       && LADSPA_IS_PORT_AUDIO
		       (psDescriptor->PortDescriptors[lPortIndex])) {
		printf("output %d\n", lPortIndex);
		lOutputPortIndex = lPortIndex;

		psDescriptor->connect_port(handle,
					   lOutputPortIndex, pOutBuffer);
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

    run_plugin();

    exit(0);
}
