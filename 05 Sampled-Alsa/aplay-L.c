/**
 * aplay-L.c
 *
 * Code from aplay.c
 * does aplay -L
 * http://alsa-utils.sourcearchive.com/documentation/1.0.15/aplay_8c-source.html
 */

/*
 * Original notice:
 *
 *  Copyright (c) by Jaroslav Kysela <perex@perex.cz>
 *  Based on vplay program by Michael Beck
 *
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA
 *
 */


#include <stdio.h>
#include <stdlib.h>
#include <alsa/asoundlib.h>
#include <locale.h>

#define _(STR) STR

static void pcm_list(snd_pcm_stream_t stream )
{
      void **hints, **n;
      char *name, *descr, *descr1, *io;
      const char *filter;

      if (snd_device_name_hint(-1, "pcm", &hints) < 0)
            return;
      n = hints;
      filter = stream == SND_PCM_STREAM_CAPTURE ? "Input" : "Output";
      while (*n != NULL) {
            name = snd_device_name_get_hint(*n, "NAME");
            descr = snd_device_name_get_hint(*n, "DESC");
            io = snd_device_name_get_hint(*n, "IOID");
            if (io != NULL && strcmp(io, filter) == 0)
                  goto __end;
            printf("%s\n", name);
            if ((descr1 = descr) != NULL) {
                  printf("    ");
                  while (*descr1) {
                        if (*descr1 == '\n')
                              printf("\n    ");
                        else
                              putchar(*descr1);
                        descr1++;
                  }
                  putchar('\n');
            }
            __end:
                  if (name != NULL)
                        free(name);
            if (descr != NULL)
                  free(descr);
            if (io != NULL)
                  free(io);
            n++;
      }
      snd_device_name_free_hint(hints);
}


int main (int argc, char *argv[])
{
  printf("*********** CAPTURE ***********\n");
  pcm_list(SND_PCM_STREAM_CAPTURE);

  printf("\n\n*********** PLAYBACK ***********\n");
  pcm_list(SND_PCM_STREAM_PLAYBACK);
}
