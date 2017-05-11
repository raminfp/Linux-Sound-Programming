/**
 * Jan Newmarch
 */

#include <stdio.h>
#include <stdlib.h>
#include <alsa/asoundlib.h>
	      

void info(char *dev_name, snd_pcm_stream_t stream) {
  snd_pcm_hw_params_t *hw_params;
  int err;
  snd_pcm_t *handle;
  unsigned int max;
  unsigned int min;
  unsigned int val;
  unsigned  int dir;
  snd_pcm_uframes_t frames;

  if ((err = snd_pcm_open (&handle, dev_name, stream, 0)) < 0) {
    fprintf (stderr, "cannot open audio device %s (%s)\n", 
	     dev_name,
	     snd_strerror (err));
    return;
  }
		   
  if ((err = snd_pcm_hw_params_malloc (&hw_params)) < 0) {
    fprintf (stderr, "cannot allocate hardware parameter structure (%s)\n",
	     snd_strerror (err));
    exit (1);
  }
				 
  if ((err = snd_pcm_hw_params_any (handle, hw_params)) < 0) {
    fprintf (stderr, "cannot initialize hardware parameter structure (%s)\n",
	     snd_strerror (err));
    exit (1);
  }
  
  if ((err = snd_pcm_hw_params_get_channels_max(hw_params, &max)) < 0) {
    fprintf (stderr, "cannot  (%s)\n",
	     snd_strerror (err));
    exit (1);
  }
  printf("max channels %d\n", max);

  if ((err = snd_pcm_hw_params_get_channels_min(hw_params, &min)) < 0) {
    fprintf (stderr, "cannot get channel info  (%s)\n",
	     snd_strerror (err));
    exit (1);
  }
  printf("min channels %d\n", min);

  /*
  if ((err = snd_pcm_hw_params_get_sbits(hw_params)) < 0) {
      fprintf (stderr, "cannot get bits info  (%s)\n",
	       snd_strerror (err));
      exit (1);
  }
  printf("bits %d\n", err);
  */

  if ((err = snd_pcm_hw_params_get_rate_min(hw_params, &val, &dir)) < 0) {
    fprintf (stderr, "cannot get min rate (%s)\n",
	     snd_strerror (err));
    exit (1);
  }
  printf("min rate %d hz\n", val);

  if ((err = snd_pcm_hw_params_get_rate_max(hw_params, &val, &dir)) < 0) {
    fprintf (stderr, "cannot get max rate (%s)\n",
	     snd_strerror (err));
    exit (1);
  }
  printf("max rate %d hz\n", val);

  
  if ((err = snd_pcm_hw_params_get_period_time_min(hw_params, &val, &dir)) < 0) {
    fprintf (stderr, "cannot get min period time  (%s)\n",
	     snd_strerror (err));
    exit (1);
  }
  printf("min period time %d usecs\n", val);

  if ((err = snd_pcm_hw_params_get_period_time_max(hw_params, &val, &dir)) < 0) {
    fprintf (stderr, "cannot  get max period time  (%s)\n",
	     snd_strerror (err));
    exit (1);
  }
  printf("max period time %d usecs\n", val);

  if ((err = snd_pcm_hw_params_get_period_size_min(hw_params, &frames, &dir)) < 0) {
    fprintf (stderr, "cannot  get min period size  (%s)\n",
	     snd_strerror (err));
    exit (1);
  }
  printf("min period size in frames %lu\n", frames);

  if ((err = snd_pcm_hw_params_get_period_size_max(hw_params, &frames, &dir)) < 0) {
    fprintf (stderr, "cannot  get max period size (%s)\n",
	     snd_strerror (err));
    exit (1);
  }
  printf("max period size in frames %lu\n", frames);

  if ((err = snd_pcm_hw_params_get_periods_min(hw_params, &val, &dir)) < 0) {
    fprintf (stderr, "cannot  get min periods  (%s)\n",
	     snd_strerror (err));
    exit (1);
  }
  printf("min periods per buffer %d\n", val);

  if ((err = snd_pcm_hw_params_get_periods_max(hw_params, &val, &dir)) < 0) {
    fprintf (stderr, "cannot  get min periods (%s)\n",
	     snd_strerror (err));
    exit (1);
  }
  printf("max periods per buffer %d\n", val);

  if ((err = snd_pcm_hw_params_get_buffer_time_min(hw_params, &val, &dir)) < 0) {
    fprintf (stderr, "cannot get min buffer time (%s)\n",
	     snd_strerror (err));
    exit (1);
  }
  printf("min buffer time %d usecs\n", val);

  if ((err = snd_pcm_hw_params_get_buffer_time_max(hw_params, &val, &dir)) < 0) {
    fprintf (stderr, "cannot get max buffer time  (%s)\n",
	     snd_strerror (err));
    exit (1);
  }
  printf("max buffer time %d usecs\n", val);

  if ((err = snd_pcm_hw_params_get_buffer_size_min(hw_params, &frames)) < 0) {
    fprintf (stderr, "cannot get min buffer size (%s)\n",
	     snd_strerror (err));
    exit (1);
  }
  printf("min buffer size in frames %lu\n", frames);

  if ((err = snd_pcm_hw_params_get_buffer_size_max(hw_params, &frames)) < 0) {
    fprintf (stderr, "cannot get max buffer size  (%s)\n",
	     snd_strerror (err));
    exit (1);
  }
  printf("max buffer size in frames %lu\n", frames);
}

int main (int argc, char *argv[])
{
  int i;
  int err;
  int buf[128];
  FILE *fin;
  size_t nread;
  unsigned int rate = 44100;	

  if (argc != 2) {
    fprintf(stderr, "Usage: %s card\n", argv[0]);
    exit(1);
  }

  printf("*********** CAPTURE ***********\n");
  info(argv[1], SND_PCM_STREAM_CAPTURE);

  printf("*********** PLAYBACK ***********\n");
  info(argv[1], SND_PCM_STREAM_PLAYBACK);

  exit (0);
}
