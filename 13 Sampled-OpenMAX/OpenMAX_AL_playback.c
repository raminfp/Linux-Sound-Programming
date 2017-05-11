/*
 * OpenMAX AL - Audio/Video Playback Example
 */
#include <stdio.h>
#include <stdlib.h>
#include "OpenMAXAL.h"
#define MAX_NUMBER_INTERFACES
5
#define MAX_NUMBER_OUTPUT_DEVICES 3
#define POSITION_UPDATE_PERIOD
1000 /* 1 sec */
/* Checks for error. If any errors exit the application! */
void CheckErr (XAresult res)
{
    if (res != XA_RESULT_SUCCESS)
	{
	    /* Debug printing to be placed here */
	    exit(1);
	}
}
void PlayEventCallback (
			XAPlayItf caller,
			void *
			pContext,
			XAuint32 playevent)
{
    /* Callback code goes here */
}
/*
 * Test audio/video playback from a 3GPP file.
 *
 * NOTE: For the purposes of this example, the implementation is assumed
 * to support the requisite audio and video codecs. Therefore, video and
 * audio decoder capabilities are NOT checked in this example.
 */
void TestAudioVideoPlayback (XAObjectItf engine)
{
    XAObjectItf
	player;
    XAObjectItf
	OutputMix;
    XAPlayItf
	playItf;
    XAEngineItf
	EngineItf;
    XAAudioIODeviceCapabilitiesItf AudioIODeviceCapabilitiesItf;
    XAAudioOutputDescriptor
	AudioOutputDescriptor;
    XAresult
	res;
    XADataSink
	XADataSink
	XADataLocator_OutputMix
	XADataLocator_NativeDisplay
	XAVolumeItf
	audioSink;
    videoSink;
    locator_outputmix;
    locator_displayregion;
    volumeItf;
    XADataSource
	avSource;
    XADataLocator_URI uri;
    XADataFormat_MIME mime;
    int i;
    char c;
    XAboolean
	required[MAX_NUMBER_INTERFACES];
    XAInterfaceID iidArray[MAX_NUMBER_INTERFACES];
    XAuint32
	XAint32
	XAboolean
	XAboolean
	XAuint32
	XANativeHandle
	OutputDeviceIDs[MAX_NUMBER_OUTPUT_DEVICES];
    numOutputs
	= 0;
    hfs_available
	= XA_BOOLEAN_FALSE;
    hfs_default
	= XA_BOOLEAN_FALSE;
    hfs_deviceID
	= 0;
    nativeWindowHandle = NULL;
    XANativeHandle nativeDisplayHandle = NULL;
    /* Get the XA Engine Interface, which is implicit */
    res = (*engine)->GetInterface(engine,
				  XA_IID_ENGINE, (void*) &EngineItf); CheckErr(res);
    /* Get the Audio IO DEVICE CAPABILITIES interface, which is also
     * implicit */
    res = (*engine)->GetInterface(engine,
				  XA_IID_AUDIOIODEVICECAPABILITIES,
				  (void*) &AudioIODeviceCapabilitiesItf); CheckErr(res);
    numOutputs = MAX_NUMBER_OUTPUT_DEVICES;
    res = (*AudioIODeviceCapabilitiesItf)->
	GetAvailableAudioOutputs(AudioIODeviceCapabilitiesItf,
				 &numOutputs, OutputDeviceIDs); CheckErr(res);
    /* Search for integrated handsfree loudspeaker */
    for (i = 0; i < numOutputs; i++)
	{
	    res = (*AudioIODeviceCapabilitiesItf)->
		QueryAudioOutputCapabilities(AudioIODeviceCapabilitiesItf,
					     OutputDeviceIDs[i], &AudioOutputDescriptor);
	    CheckErr(res);
	    if ((AudioOutputDescriptor.deviceConnection ==
		 XA_DEVCONNECTION_INTEGRATED) &&
		(AudioOutputDescriptor.deviceScope ==
		 XA_DEVSCOPE_ENVIRONMENT) &&
		(AudioOutputDescriptor.deviceLocation ==
		 XA_DEVLOCATION_HANDSET))
		{
		    hfs_deviceID = OutputDeviceIDs[i];
		    hfs_available = XA_BOOLEAN_TRUE;
		    break;
		}
	}
    /* If preferred output audio device is not available, no point in
     * continuing */
    if (!hfs_available)
	{
	    /* Appropriate error message here */
	    exit(1);
	}
    numOutputs = MAX_NUMBER_OUTPUT_DEVICES;
    res = (*AudioIODeviceCapabilitiesItf)->
	GetDefaultAudioDevices(AudioIODeviceCapabilitiesItf,
			       XA_DEFAULTDEVICEID_AUDIOOUTPUT, &numOutputs,
			       OutputDeviceIDs);
    CheckErr(res);
    /* Check whether Default Output devices include the handsfree
     * loudspeaker */
    for (i = 0; i < numOutputs; i++)
	{
	    if (OutputDeviceIDs[i] == hfs_deviceID)
		{
		    hfs_default = XA_BOOLEAN_TRUE;
		    break;
		}
	}
    /* Expect handsfree loudspeaker to be set as one of the default
     * output devices */
    if (!hfs_default)
	{
	    /* Debug printing to be placed here */
	    exit(1);
	}
    /* Initialize arrays required[] and iidArray[] */
    for (i = 0; i < MAX_NUMBER_INTERFACES; i++)
	{
	    required[i] = XA_BOOLEAN_FALSE;
	    iidArray[i] = XA_IID_NULL;
	}
    /* Set arrays required[] and iidArray[] for VOLUME interface */
    required[0] = XA_BOOLEAN_TRUE;
    iidArray[0] = XA_IID_VOLUME;
    /* Create Output Mix object to be used by player */
    res = (*EngineItf)->CreateOutputMix(EngineItf,
					&OutputMix, 1, iidArray, required); CheckErr(res);
    /* Realizing the Output Mix object in synchronous mode
       res = (*OutputMix)->Realize(OutputMix,
       XA_BOOLEAN_FALSE); CheckErr(res);
    */
    /* Get the volume interface on the output mix */
    res = (*OutputMix)->GetInterface(OutputMix,
				     XA_IID_VOLUME, (void*)&volumeItf); CheckErr(res);
    /* Setup the audio/video data source structure */
    uri.locatorType
	= XA_DATALOCATOR_URI;
    uri.URI
	= (XAchar *) "file:///avmedia.3gp";
    mime.formatType
	= XA_DATAFORMAT_MIME;
    mime.mimeType
	= (XAchar *) "video/3gpp";
    mime.containerType = XA_CONTAINERTYPE_3GPP; /* provided as a hint to
						 * the player */
    avSource.pLocator = (void*) &uri;
    avSource.pFormat = (void*) &mime;
    /* Setup the audio data sink structure */
    locator_outputmix.locatorType = XA_DATALOCATOR_OUTPUTMIX;
    locator_outputmix.outputMix
	= OutputMix;
    audioSink.pLocator
	= (void*) &locator_outputmix;
    audioSink.pFormat
	= NULL;
    /* Set nativeWindowHandle and nativeDisplayHandle to
     * platform-specific values here */
    /* nativeWindowHandle = <a platform-specific value>; */
    /* nativeDisplayHandle = <a platform-specific value>; */
    /* Setup the video data sink structure */
    locator_displayregion.locatorType = XA_DATALOCATOR_NATIVEDISPLAY;
    locator_displayregion.hWindow
	= nativeWindowHandle;
    locator_displayregion.hDisplay
	= nativeDisplayHandle;
    videoSink.pLocator
	= (void*) &locator_displayregion;
    videoSink.pFormat
	= NULL;
    /* Create the media player. pBankSrc is NULL as we have a non-MIDI
     * data source */
    res = (*EngineItf)->CreateMediaPlayer(EngineItf,
					  &player, &avSource, NULL, &audioSink, &videoSink, NULL, NULL,
					  1, iidArray, required); CheckErr(res);
    /* Realizing the player in synchronous mode */
    res = (*player)->Realize(player, XA_BOOLEAN_FALSE); CheckErr(res);
    /* Get play interface */
    res = (*player)->GetInterface(player,
				  XA_IID_PLAY, (void*) &playItf); CheckErr(res);
    /* Setup to receive position event callbacks */
    res = (*playItf)->RegisterCallback(playItf,
				       PlayEventCallback, NULL); CheckErr(res);
    /* Set notifications to occur after every 1 second - might be useful
     * in updating a progress bar */
    res = (*playItf)->SetPositionUpdatePeriod(playItf,
					      POSITION_UPDATE_PERIOD); CheckErr(res);
    res = (*playItf)->SetCallbackEventsMask(playItf,
					    XA_PLAYEVENT_HEADATNEWPOS); CheckErr(res);
    /* Before we start, set volume to -3dB (-300mB) */
    res = (*volumeItf)->SetVolumeLevel(volumeItf, -300); CheckErr(res);
    /* Play the media */
    res = (*playItf)->SetPlayState(playItf,
				   XA_PLAYSTATE_PLAYING); CheckErr(res);
    while ((c = getchar()) != 'q')
	{
	    XAuint32 playState;
	    switch(c)
		{
		case '1':
		    /* Play the media - if it is not already playing */
		    res = (*playItf)->GetPlayState(playItf,
						   &playState); CheckErr(res);
		    if (playState != XA_PLAYSTATE_PLAYING)
			{
			    res = (*playItf)->SetPlayState(playItf,
							   XA_PLAYSTATE_PLAYING); CheckErr(res);
			    455
				}
		    break;
		case '2':
		    /* Pause the media - if it is playing */
		    res = (*playItf)->GetPlayState(playItf,
						   &playState); CheckErr(res);
		    if (playState == XA_PLAYSTATE_PLAYING)
			{
			    res = (*playItf)->SetPlayState(playItf,
							   XA_PLAYSTATE_PAUSED); CheckErr(res);
			}
		    break;
		default:
		    break;
		}
	}
    /* Stop the media playback */
    res = (*playItf)->SetPlayState(playItf,
				   XA_PLAYSTATE_STOPPED); CheckErr(res);
    /* Destroy the player object */
    (*player)->Destroy(player);
    /* Destroy the output mix object */
    (*OutputMix)->Destroy(OutputMix);
}
int xa_main (void)
{
    XAresult
	res;
    XAObjectItf engine;
    /* Create OpenMAX AL engine in thread-safe mode */
    XAEngineOption EngineOption[] = {
	(XAuint32) XA_ENGINEOPTION_THREADSAFE,
	(XAuint32) XA_BOOLEAN_TRUE};
    res = xaCreateEngine(&engine,
			 1, EngineOption, 0, NULL, NULL); CheckErr(res);
    /* Realizing the AL Engine in synchronous mode */
    res = (*engine)->Realize(engine, XA_BOOLEAN_FALSE); CheckErr(res);
    TestAudioVideoPlayback(engine);
    /* Shutdown OpenMAX AL engine */
    (*engine)->Destroy(engine);
    exit(0);
}
