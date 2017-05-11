/**
   Contains code
   Copyright (C) 2007-2009 STMicroelectronics
   Copyright (C) 2007-2009 Nokia Corporation and/or its subsidiary(-ies).
   under the LGPL

   and code Copyright (C) 2010 SWOAG Technology <www.swoag.com> under the LGPL
*/

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/stat.h>
#include <pthread.h>

#include <OMX_Core.h>
#include <OMX_Component.h>
#include <OMX_Types.h>
#include <OMX_Audio.h>

/* we will use 4 of each port's buffers */
#define NUM_BUFFERS_USED 4

#ifdef LIM
char *decodeComponentName = "OMX.limoi.ogg_dec";
char *renderComponentName = "OMX.limoi.alsa_sink";
#endif

OMX_HANDLETYPE decodeHandle;
OMX_HANDLETYPE renderHandle;
OMX_VERSIONTYPE specVersion, compVersion;

int inFd = 0;

static OMX_BOOL bEOS=OMX_FALSE;

OMX_U32 inDecodeBufferSize;
OMX_U32 outDecodeBufferSize;
int numInDecodeBuffers;
int numOutDecodeBuffers;

OMX_U32 inRenderBufferSize;
int numInRenderBuffers;

OMX_BUFFERHEADERTYPE *inDecodeBuffers[NUM_BUFFERS_USED];
OMX_BUFFERHEADERTYPE *outDecodeBuffers[NUM_BUFFERS_USED];
OMX_BUFFERHEADERTYPE *inRenderBuffers[NUM_BUFFERS_USED];

pthread_mutex_t mutex;
OMX_STATETYPE currentState = OMX_StateLoaded;
pthread_cond_t stateCond;

void waitFor(OMX_STATETYPE state) {
    pthread_mutex_lock(&mutex);
    while (currentState != state)
	pthread_cond_wait(&stateCond, &mutex);
    printf("Wait successfully completed\n");
    pthread_mutex_unlock(&mutex);
}

void wakeUp(OMX_STATETYPE newState) {
    pthread_mutex_lock(&mutex);
    currentState = newState;
    pthread_cond_signal(&stateCond);
    pthread_mutex_unlock(&mutex);
}
pthread_mutex_t empty_mutex;
int emptyState = 0;
OMX_BUFFERHEADERTYPE* pEmptyBuffer;
pthread_cond_t emptyStateCond;

void waitForEmpty() {
    pthread_mutex_lock(&empty_mutex);
    while (emptyState == 1)
	pthread_cond_wait(&emptyStateCond, &empty_mutex);
    emptyState = 1;
    pthread_mutex_unlock(&empty_mutex);
}

void wakeUpEmpty(OMX_BUFFERHEADERTYPE* pBuffer) {
    pthread_mutex_lock(&empty_mutex);
    emptyState = 0;
    pEmptyBuffer = pBuffer;
    pthread_cond_signal(&emptyStateCond);
    pthread_mutex_unlock(&empty_mutex);
}

void mutex_init() {
    int n = pthread_mutex_init(&mutex, NULL);
    if ( n != 0) {
	fprintf(stderr, "Can't init state mutex\n");
    }
    n = pthread_mutex_init(&empty_mutex, NULL);
    if ( n != 0) {
	fprintf(stderr, "Can't init empty mutex\n");
    }
}

static void display_help() {
    fprintf(stderr, "Usage: play_ogg input_file");
}


static void setHeader(OMX_PTR header, OMX_U32 size) {
    /* header->nVersion */
    OMX_VERSIONTYPE* ver = (OMX_VERSIONTYPE*)(header + sizeof(OMX_U32));
    /* header->nSize */
    *((OMX_U32*)header) = size;

    /* for 1.2
       ver->s.nVersionMajor = OMX_VERSION_MAJOR;
       ver->s.nVersionMinor = OMX_VERSION_MINOR;
       ver->s.nRevision = OMX_VERSION_REVISION;
       ver->s.nStep = OMX_VERSION_STEP;
    */
    ver->s.nVersionMajor = specVersion.s.nVersionMajor;
    ver->s.nVersionMinor = specVersion.s.nVersionMinor;
    ver->s.nRevision = specVersion.s.nRevision;
    ver->s.nStep = specVersion.s.nStep;
}

static void reconfig_component_port (OMX_HANDLETYPE comp, int port) {
    OMX_PARAM_PORTDEFINITIONTYPE port_def;

    port_def.nSize = sizeof(OMX_PARAM_PORTDEFINITIONTYPE);
    port_def.nPortIndex = port;
    if (OMX_ErrorNone != OMX_GetParameter
        (comp, OMX_IndexParamPortDefinition, &port_def))
    {
        fprintf(stderr, "unable to get port %d definition.\n", port);
        exit(-1);
    } else
    {
        port_def.nBufferCountActual = NUM_BUFFERS_USED;
        port_def.nBufferCountMin = NUM_BUFFERS_USED;

        OMX_SetParameter(comp, OMX_IndexParamPortDefinition, &port_def);
    }
}

OMX_ERRORTYPE cEventHandler(
			    OMX_HANDLETYPE hComponent,
			    OMX_PTR pAppData,
			    OMX_EVENTTYPE eEvent,
			    OMX_U32 Data1,
			    OMX_U32 Data2,
			    OMX_PTR pEventData) {
    OMX_ERRORTYPE err;

    printf("Hi there, I am in the %s callback\n", __func__);
    if(eEvent == OMX_EventCmdComplete) {
	if (Data1 == OMX_CommandStateSet) {
	    printf("Component State changed in ", 0);
	    switch ((int)Data2) {
	    case OMX_StateInvalid:
		printf("OMX_StateInvalid\n", 0);
		break;
	    case OMX_StateLoaded:
		printf("OMX_StateLoaded\n", 0);
		break;
	    case OMX_StateIdle:
		printf("OMX_StateIdle\n",0);
		break;
	    case OMX_StateExecuting:
		printf("OMX_StateExecuting\n",0);
		break;
	    case OMX_StatePause:
		printf("OMX_StatePause\n",0);
		break;
	    case OMX_StateWaitForResources:
		printf("OMX_StateWaitForResources\n",0);
		break;
	    }
	    wakeUp((int) Data2);
	} else  if (Data1 == OMX_CommandPortEnable){
	    printf("OMX State Port enabled\n");
	} else if (Data1 == OMX_CommandPortDisable){
	    printf("OMX State Port disabled\n");     
	}
    } else if(eEvent == OMX_EventBufferFlag) {
	if((int)Data2 == OMX_BUFFERFLAG_EOS) {
     
	}
    } else if(eEvent == OMX_EventError) {
	printf("Event is Error\n");
    } else  if(eEvent == OMX_EventMark) {
	printf("Event is Buffer Mark\n");
    } else  if(eEvent == OMX_EventPortSettingsChanged) {
	/* See 1.1 spec, section 8.9.3.1 Playback Use Case */
	OMX_PARAM_PORTDEFINITIONTYPE sPortDef;

	setHeader(&sPortDef, sizeof(OMX_PARAM_PORTDEFINITIONTYPE));
	sPortDef.nPortIndex = Data1;

	err = OMX_GetConfig(hComponent, OMX_IndexParamPortDefinition, &sPortDef);
	if(err != OMX_ErrorNone){
	    fprintf(stderr, "Error in getting OMX_PORT_DEFINITION_TYPE parameter\n", 0);
	    exit(1);
	}
	printf("Event is Port Settings Changed on port %d in component %p\n",
	       Data1, hComponent);
	printf("Port has %d buffers of size is %d\n",  sPortDef.nBufferCountMin, sPortDef.nBufferSize);
    } else {
	printf("Event is %i\n", (int)eEvent);
	printf("Param1 is %i\n", (int)Data1);
	printf("Param2 is %i\n", (int)Data2);
    }

    return OMX_ErrorNone;
}

OMX_ERRORTYPE cDecodeEmptyBufferDone(
			       OMX_HANDLETYPE hComponent,
			       OMX_PTR pAppData,
			       OMX_BUFFERHEADERTYPE* pBuffer) {
    int n;
    OMX_ERRORTYPE err;

    printf("Hi there, I am in the %s callback, buffer %p.\n", __func__, pBuffer);
    if (bEOS) {
	printf("Buffers emptied, exiting\n");
	wakeUp(OMX_StateLoaded);
	exit(0);
    }
    printf("  left in buffer %d\n", pBuffer->nFilledLen);

    /* put data into the buffer, and then empty it */
    int data_read = read(inFd, pBuffer->pBuffer, inDecodeBufferSize);
    if (data_read <= 0) {
	bEOS = 1;
    }
    printf("  filled buffer %p with %d\n", pBuffer, data_read);
    pBuffer->nFilledLen = data_read;

    err = OMX_EmptyThisBuffer(decodeHandle, pBuffer);
    if (err != OMX_ErrorNone) {
	fprintf(stderr, "Error on emptying decode buffer\n");
	exit(1);
    }

    return OMX_ErrorNone;
}

OMX_ERRORTYPE cDecodeFillBufferDone(
			      OMX_HANDLETYPE hComponent,
			      OMX_PTR pAppData,
			      OMX_BUFFERHEADERTYPE* pBuffer) {
    OMX_BUFFERHEADERTYPE* pRenderBuffer;
    int n;
    OMX_ERRORTYPE err;

    printf("Hi there, I am in the %s callback, buffer %p with %d bytes.\n", 
	   __func__, pBuffer, pBuffer->nFilledLen);
    if (bEOS) {
	printf("Buffers filled, exiting\n");
    }

    /* find matching render buffer */
    for (n = 0; n < NUM_BUFFERS_USED; n++) {
	if (pBuffer == outDecodeBuffers[n]) {
	    pRenderBuffer = inRenderBuffers[n];
	    break;
	}
    }

    pRenderBuffer->nFilledLen = pBuffer->nFilledLen;
    /* We don't attempt to refill the decode output buffer until we
       have emptied the render input buffer. So we can just use the
       decode output buffer for the render input buffer.
       Avoids us copying the data across buffers.
    */
    pRenderBuffer->pBuffer = pBuffer->pBuffer;

    err = OMX_EmptyThisBuffer(renderHandle, pRenderBuffer);
    if (err != OMX_ErrorNone) {
	fprintf(stderr, "Error on emptying buffer\n");
	exit(1);
    }

    return OMX_ErrorNone;
}


OMX_CALLBACKTYPE decodeCallbacks  = { .EventHandler = cEventHandler,
				.EmptyBufferDone = cDecodeEmptyBufferDone,
				.FillBufferDone = cDecodeFillBufferDone
};


OMX_ERRORTYPE cRenderEmptyBufferDone(
			       OMX_HANDLETYPE hComponent,
			       OMX_PTR pAppData,
			       OMX_BUFFERHEADERTYPE* pBuffer) {
    OMX_BUFFERHEADERTYPE *inDecodeBuffer;
    OMX_BUFFERHEADERTYPE *outDecodeBuffer;
    int n;
    OMX_ERRORTYPE err;
 
    printf("Hi there, I am in the %s callback, buffer %p.\n", __func__, pBuffer);

    /* find matching buffer indices */
    for (n = 0; n < NUM_BUFFERS_USED; n++) {
	if (pBuffer == inRenderBuffers[n]) {
	    outDecodeBuffer = outDecodeBuffers[n];
	    break;
	}
    }

    /* and fill the corresponding output buffer */
    outDecodeBuffer->nFilledLen = 0;
    err = OMX_FillThisBuffer(decodeHandle, outDecodeBuffer);
    if (err != OMX_ErrorNone) {
	fprintf(stderr, "Error on filling buffer\n");
	exit(1);
    }
       
    return OMX_ErrorNone;
}


OMX_CALLBACKTYPE renderCallbacks  = { .EventHandler = cEventHandler,
				.EmptyBufferDone = cRenderEmptyBufferDone,
				.FillBufferDone = NULL
};

void printState(OMX_HANDLETYPE handle) {
    OMX_STATETYPE state;
    OMX_ERRORTYPE err;

    err = OMX_GetState(handle, &state);
    if (err != OMX_ErrorNone) {
	fprintf(stderr, "Error on getting state\n");
	exit(1);
    }
    switch (state) {
    case OMX_StateLoaded: printf("StateLoaded\n"); break;
    case OMX_StateIdle: printf("StateIdle\n"); break;
    case OMX_StateExecuting: printf("StateExecuting\n"); break;
    case OMX_StatePause: printf("StatePause\n"); break;
    case OMX_StateWaitForResources: printf("StateWiat\n"); break;
    default:  printf("State unknown\n"); break;
    }
}

void setEncoding(OMX_HANDLETYPE handle, int portNumber, OMX_AUDIO_CODINGTYPE encoding) {
    OMX_AUDIO_PARAM_PORTFORMATTYPE sAudioPortFormat;
    OMX_ERRORTYPE err;


    setHeader(&sAudioPortFormat, sizeof(OMX_AUDIO_PARAM_PORTFORMATTYPE));
    sAudioPortFormat.nIndex = 0;
    sAudioPortFormat.nPortIndex = portNumber;

    err = OMX_GetParameter(handle, OMX_IndexParamAudioPortFormat, &sAudioPortFormat);
    if (err == OMX_ErrorNoMore) {
	printf("Can't get format\n");
	return;
    }
    sAudioPortFormat.eEncoding = encoding;
    err = OMX_SetParameter(handle, OMX_IndexParamAudioPortFormat, &sAudioPortFormat);
    if (err == OMX_ErrorNoMore) {
	printf("Can't set format\n");
	return;
    }
    printf("Set format on port %d\n", portNumber);
}

/*
   *pBuffer is set to non-zero if a particular buffer size is required by the client
 */
void createBuffers(OMX_HANDLETYPE handle, int portNumber, 
		      OMX_U32 *pBufferSize, OMX_BUFFERHEADERTYPE **ppBuffers) {
    OMX_PARAM_PORTDEFINITIONTYPE sPortDef;
    int n;
    int nBuffers;
    OMX_ERRORTYPE err;

    setHeader(&sPortDef, sizeof(OMX_PARAM_PORTDEFINITIONTYPE));
    sPortDef.nPortIndex = portNumber;
    err = OMX_GetParameter(handle, OMX_IndexParamPortDefinition, &sPortDef);
    if(err != OMX_ErrorNone){
	fprintf(stderr, "Error in getting OMX_PORT_DEFINITION_TYPE parameter\n", 0);
	exit(1);
    }

    /* if no size pre-allocated, use the minimum */
    if (*pBufferSize == 0) {
	*pBufferSize = sPortDef.nBufferSize;
    } else {
	sPortDef.nBufferSize = *pBufferSize;
    }

    nBuffers = NUM_BUFFERS_USED;
    printf("Port %d has %d buffers of size is %d\n", portNumber, nBuffers, *pBufferSize);

    for (n = 0; n < nBuffers; n++) {
	err = OMX_AllocateBuffer(handle, ppBuffers+n, portNumber, NULL,
				 *pBufferSize);
	if (err != OMX_ErrorNone) {
	    fprintf(stderr, "Error on AllocateBuffer is %d\n", err);
	    exit(1);
	}
    }
}

int main(int argc, char** argv) {

    OMX_PORT_PARAM_TYPE param;
    OMX_PARAM_PORTDEFINITIONTYPE sPortDef;
    OMX_AUDIO_PORTDEFINITIONTYPE sAudioPortDef;
    OMX_AUDIO_PARAM_PORTFORMATTYPE sAudioPortFormat;
    OMX_AUDIO_PARAM_PCMMODETYPE sPCMMode;
    OMX_ERRORTYPE err;

    OMX_AUDIO_PARAM_MP3TYPE sMP3Mode;


    unsigned char name[OMX_MAX_STRINGNAME_SIZE];
    OMX_UUIDTYPE uid;
    int startDecodePortNumber;
    int nDecodePorts;
    int startRenderPortNumber;
    int nRenderPorts;
    int n;

    printf("Thread id is %p\n", pthread_self());
    if(argc < 1){
	display_help();
	exit(1);
    }

    inFd = open(argv[1], O_RDONLY);
    if(inFd < 0){
	fprintf(stderr, "Error opening input file \"%s\"\n", argv[1]);
	exit(1);
    }

    err = OMX_Init();
    if(err != OMX_ErrorNone) {
	fprintf(stderr, "OMX_Init() failed\n", 0);
	exit(1);
    }
 
   /** Ask the core for a handle to the decode component
     */
    err = OMX_GetHandle(&decodeHandle, decodeComponentName, 
			NULL /*app private data */, &decodeCallbacks);
    if(err != OMX_ErrorNone) {
	fprintf(stderr, "OMX_GetHandle failed\n", 0);
	exit(1);
    }
    err = OMX_GetComponentVersion(decodeHandle, name, &compVersion, &specVersion, &uid);
    if(err != OMX_ErrorNone) {
	fprintf(stderr, "OMX_GetComponentVersion failed\n", 0);
	exit(1);
    }

    /** Ask the core for a handle to the render component
     */
    err = OMX_GetHandle(&renderHandle, renderComponentName, 
			NULL /*app private data */, &renderCallbacks);
    if(err != OMX_ErrorNone) {
	fprintf(stderr, "OMX_GetHandle failed\n", 0);
	exit(1);
    }

    /** no other ports to disable */

    /** Get audio port information */
    /* Decode component */
    setHeader(&param, sizeof(OMX_PORT_PARAM_TYPE));
    err = OMX_GetParameter(decodeHandle, OMX_IndexParamAudioInit, &param);
    if(err != OMX_ErrorNone){
	fprintf(stderr, "Error in getting OMX_PORT_PARAM_TYPE parameter\n", 0);
	exit(1);
    }
    startDecodePortNumber = ((OMX_PORT_PARAM_TYPE)param).nStartPortNumber;
    nDecodePorts = ((OMX_PORT_PARAM_TYPE)param).nPorts;
    if (nDecodePorts != 2) {
	fprintf(stderr, "Decode device has wrong number of ports: %d\n", nDecodePorts);
	exit(1);
    }

    setEncoding(decodeHandle, startDecodePortNumber, OMX_AUDIO_CodingVORBIS);
    setEncoding(decodeHandle, startDecodePortNumber+1, OMX_AUDIO_CodingPCM);

    printState(decodeHandle);;

    reconfig_component_port(decodeHandle, startDecodePortNumber);
    reconfig_component_port(decodeHandle, startDecodePortNumber+1);

    /* call to put decoder state into idle before allocating buffers */
    err = OMX_SendCommand(decodeHandle, OMX_CommandStateSet, OMX_StateIdle, NULL);
    if (err != OMX_ErrorNone) {
	fprintf(stderr, "Error on setting state to idle\n");
	exit(1);
    }
 
    /* ensure decoder ports are enabled */
    err = OMX_SendCommand(decodeHandle, OMX_CommandPortEnable, startDecodePortNumber, NULL);
    if (err != OMX_ErrorNone) {
	fprintf(stderr, "Error on setting port to enabled\n");
	exit(1);
    }
    err = OMX_SendCommand(decodeHandle, OMX_CommandPortEnable, startDecodePortNumber+1, NULL);
    if (err != OMX_ErrorNone) {
	fprintf(stderr, "Error on setting port to enabled\n");
	exit(1);
    }

    /* use default buffer sizes */
    inDecodeBufferSize = outDecodeBufferSize = 0;
    createBuffers(decodeHandle, startDecodePortNumber, &inDecodeBufferSize, inDecodeBuffers);
    createBuffers(decodeHandle, startDecodePortNumber+1, &outDecodeBufferSize, outDecodeBuffers);


    /* Make sure we've reached Idle state */
    waitFor(OMX_StateIdle);
    printf("Reached Idle state\n");

    /* Now try to switch to Executing state */
    err = OMX_SendCommand(decodeHandle, OMX_CommandStateSet, OMX_StateExecuting, NULL);
    if(err != OMX_ErrorNone){
	fprintf(stderr, "Error changing to Executing state\n");
	exit(1);
    }
    /* end decode setting */


    /* Render component */
    setHeader(&param, sizeof(OMX_PORT_PARAM_TYPE));
    err = OMX_GetParameter(renderHandle, OMX_IndexParamAudioInit, &param);
    if(err != OMX_ErrorNone){
	fprintf(stderr, "Error in getting OMX_PORT_PARAM_TYPE parameter\n", 0);
	exit(1);
    }
    startRenderPortNumber = ((OMX_PORT_PARAM_TYPE)param).nStartPortNumber;
    nRenderPorts = ((OMX_PORT_PARAM_TYPE)param).nPorts;
    if (nRenderPorts != 1) {
	fprintf(stderr, "Render device has wrong number of ports: %d\n", nRenderPorts);
	exit(1);
    }

    setEncoding(renderHandle, startRenderPortNumber, OMX_AUDIO_CodingPCM);

    printState(renderHandle);
    
     /* call to put state into idle before allocating buffers */
    err = OMX_SendCommand(renderHandle, OMX_CommandStateSet, OMX_StateIdle, NULL);
    if (err != OMX_ErrorNone) {
	fprintf(stderr, "Error on setting state to idle\n");
	exit(1);
    }

    /* ensure port is enabled */ 
    err = OMX_SendCommand(renderHandle, OMX_CommandPortEnable, startRenderPortNumber, NULL);
    if (err != OMX_ErrorNone) {
	fprintf(stderr, "Error on setting port to enabled\n");
	exit(1);
    }

    reconfig_component_port(renderHandle, startRenderPortNumber);

    /* set render buffer size to decoder out buffer size */
    inRenderBufferSize = outDecodeBufferSize;
    createBuffers(renderHandle, startRenderPortNumber, &inRenderBufferSize, inRenderBuffers);


    /* Make sure we've reached Idle state */
    waitFor(OMX_StateIdle);
    printf("Reached Idle state\n");

    /* Now try to switch renderer to Executing state */
    err = OMX_SendCommand(renderHandle, OMX_CommandStateSet, OMX_StateExecuting, NULL);
    if(err != OMX_ErrorNone){
	fprintf(stderr, "Error changing render to executing state\n");
	exit(1);
    }
    /* end render setting */


    /* debugging: print buffer pointers */
    for (n = 0; n < NUM_BUFFERS_USED; n++)
	printf("In decode buffer %d is %p\n", n, inDecodeBuffers[n]);
    for (n = 0; n < NUM_BUFFERS_USED; n++)
	printf("Out decode buffer %d is %p\n", n, outDecodeBuffers[n]);
    for (n = 0; n < NUM_BUFFERS_USED; n++)
	printf("In render buffer %d is %p\n", n, inRenderBuffers[n]);


    /* no buffers emptied yet */
    pEmptyBuffer = NULL;

    /* load  the decoder input buffers */
    for (n = 0; n < NUM_BUFFERS_USED; n++) {
	int data_read = read(inFd, inDecodeBuffers[n]->pBuffer, inDecodeBufferSize);
	inDecodeBuffers[n]->nFilledLen = data_read;
	printf("Read %d into buffer %p\n", data_read, inDecodeBuffers[n]);
	if (data_read <= 0) {
	    printf("In the %s no more input data available\n", __func__);
	    inDecodeBuffers[n]->nFilledLen = 0;
	    inDecodeBuffers[n]->nFlags = OMX_BUFFERFLAG_EOS;
	    bEOS=OMX_TRUE;
	}
    }
    
    /* fill the decoder output buffers */
    for (n = 0; n < NUM_BUFFERS_USED; n++) {
	outDecodeBuffers[n]->nFilledLen = 0;
	err = OMX_FillThisBuffer(decodeHandle, outDecodeBuffers[n]);
	if (err != OMX_ErrorNone) {
	    fprintf(stderr, "Error on filling buffer\n");
	    exit(1);
	}
    }

    /* empty the decoder input bufers */
    for (n = 0; n < NUM_BUFFERS_USED; n++) {
	err = OMX_EmptyThisBuffer(decodeHandle, inDecodeBuffers[n]);
	if (err != OMX_ErrorNone) {
	    fprintf(stderr, "Error on emptying buffer\n");
	    exit(1);
	}
    }

    pEmptyBuffer = inDecodeBuffers[0];
    emptyState = 1;

    waitFor(OMX_StateLoaded);
    printf("Buffers emptied\n");
    exit(0);
}
