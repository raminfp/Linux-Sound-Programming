

#include <gtk/gtk.h>

#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>

#define MIDI_FILE "54154.mid"
#define VIDEO_FILE "short.mpg"

GtkWidget *image;
GdkPixbuf *pixbuf;
int width, height;


/* FFMpeg vbls */
AVFormatContext *pFormatCtx = NULL;
AVCodecContext *pCodecCtx = NULL;
int videoStream;
struct SwsContext *sws_ctx = NULL;
AVCodec *pCodec = NULL;

static void pixmap_destroy_notify(guchar *pixels,
				  gpointer data) {
    printf("Ddestroy pixmap\n");
}

static gboolean draw_image(gpointer user_data) {
    // GtkImage *image = (GtkImage *) user_data;

    gtk_image_set_from_pixbuf((GtkImage *) image, pixbuf);
    gtk_widget_queue_draw(image);
}

static void *play_background(void *args) {

    int i;
    AVPacket packet;
    int frameFinished;
    AVFrame *pFrame = NULL;

    int oldSize;
    char *oldData;
    int bytesDecoded;

    AVFrame *picture_RGB;
    char *buffer;


    pFrame=av_frame_alloc();

    i=0;
    picture_RGB = av_frame_alloc();
    buffer = malloc (avpicture_get_size(PIX_FMT_RGB24, width, height));
    avpicture_fill((AVPicture *)picture_RGB, buffer, PIX_FMT_RGB24, width, height);

    while(av_read_frame(pFormatCtx, &packet)>=0) {
	if(packet.stream_index==videoStream) {
	    //printf("Frame %d\n", i++);
	    usleep(33670);  // 29.7 frames per second
	    // Decode video frame
	    avcodec_decode_video2(pCodecCtx, pFrame, &frameFinished,   &packet);
	    
	    sws_ctx = sws_getContext(pCodecCtx->width, pCodecCtx->height, 
				     pCodecCtx->pix_fmt, pCodecCtx->width, 
				     pCodecCtx->height, PIX_FMT_RGB24, 
				     SWS_BICUBIC, NULL, NULL, NULL);

	    if (frameFinished) {

		sws_scale(sws_ctx,  (uint8_t const * const *) pFrame->data, pFrame->linesize, 0, pCodecCtx->height, picture_RGB->data, picture_RGB->linesize);

		pixbuf = gdk_pixbuf_new_from_data(picture_RGB->data[0], GDK_COLORSPACE_RGB, 0, 8, width, height, picture_RGB->linesize[0], pixmap_destroy_notify, NULL);

		gdk_threads_add_idle(draw_image, NULL);
	    }
	    sws_freeContext(sws_ctx);
	}
	av_free_packet(&packet);
    }
    printf("Video over!\n");
    exit(0);
}

/* Called when the windows are realized
 */
static void realize_cb (GtkWidget *widget, gpointer data) {
    /* start the video playing in its own thread */
    pthread_t tid_video;
    pthread_create(&tid_video, NULL, play_background, NULL);
}

static gboolean delete_event( GtkWidget *widget,
                              GdkEvent  *event,
                              gpointer   data )
{
    /* If you return FALSE in the "delete-event" signal handler,
     * GTK will emit the "destroy" signal. Returning TRUE means
     * you don't want the window to be destroyed.
     * This is useful for popping up 'are you sure you want to quit?'
     * type dialogs. */

    g_print ("delete event occurred\n");

    /* Change TRUE to FALSE and the main window will be destroyed with
     * a "delete-event". */
    return FALSE;
}

/* Another callback */
static void destroy( GtkWidget *widget,
                     gpointer   data )
{
    gtk_main_quit ();
}

void init_ffmpeg() {
    /* FFMpeg stuff */

    AVFrame *pFrame = NULL;
    AVPacket packet;

    AVDictionary *optionsDict = NULL;


    av_register_all();

    if(avformat_open_input(&pFormatCtx, VIDEO_FILE, NULL, NULL)!=0)
	return; // Couldn't open file
  
    // Retrieve stream information
    if(avformat_find_stream_info(pFormatCtx, NULL)<0)
	return; // Couldn't find stream information
  
    // Dump information about file onto standard error
    av_dump_format(pFormatCtx, 0, VIDEO_FILE, 0);
  
    // Find the first video stream
    videoStream=-1;
    int i;
    for(i = 0; i < pFormatCtx->nb_streams; i++)
	if(pFormatCtx->streams[i]->codec->codec_type==AVMEDIA_TYPE_VIDEO) {
	    videoStream=i;
	    break;
	}
    if(videoStream==-1)
	return; // Didn't find a video stream

    for(i=0; i<pFormatCtx->nb_streams; i++)
	if(pFormatCtx->streams[i]->codec->codec_type==AVMEDIA_TYPE_AUDIO) {
	    printf("Found an audio stream too\n");
	    break;
	}

  
    // Get a pointer to the codec context for the video stream
    pCodecCtx=pFormatCtx->streams[videoStream]->codec;

    width =  pCodecCtx->width;
    height =  pCodecCtx->height;
  
    // Find the decoder for the video stream
    pCodec=avcodec_find_decoder(pCodecCtx->codec_id);
    if(pCodec==NULL) {
	fprintf(stderr, "Unsupported codec!\n");
	return; // Codec not found
    }
  
    // Open codec
    if(avcodec_open2(pCodecCtx, pCodec, &optionsDict)<0)
	return; // Could not open codec
}

void *play_gtk(void *args);

void *init_gtk(void *args) {
    /* GTK stuff now */

    printf(" GTK_MAJOR_VERSION is %d\n",  GTK_MAJOR_VERSION);

    GtkWidget *window;
    GtkWidget *box;
    
    printf("Initing gtk\n");
    if ( !gtk_init_check (0, NULL)) {
	printf("Gtk init failed\n");
    } else {
	printf("Inited Gtk ok\n");
    }

    window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
    
    g_signal_connect (window, "delete-event",
		      G_CALLBACK (delete_event), NULL);
        g_signal_connect (window, "destroy",
		      G_CALLBACK (destroy), NULL);

    g_signal_connect (window, "realize", G_CALLBACK (realize_cb), NULL);
    
    /* Sets the border width of the window. */
    gtk_container_set_border_width (GTK_CONTAINER (window), 10);

#if GTK_MAJOR_VERSION == 3
    box = gtk_box_new(TRUE, 1);
#endif

    gtk_widget_show(box);

    image = gtk_image_new();

    gtk_widget_show (image);
    
    gtk_box_pack_start (GTK_BOX (box), image, TRUE, TRUE, 0);
   
    gtk_container_add (GTK_CONTAINER (window), box);
    gtk_widget_show (window);
    
    play_gtk(NULL);
    printf("Finished initing gtk\n");
}


void *play_gtk(void *args) {
    printf("About to start gtk_main\n");
    gtk_main();
}
