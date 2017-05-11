/**
 * aplay-l.c
 *
 * Code from aplay.c
 *
 * does the same as aplay -l
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

// used by gettext for i18n, not needed here
#define _(STR) STR

static char *command;
#define error(...) do {\
      fprintf(stderr, "%s: %s:%d: ", command, __FUNCTION__, __LINE__); \
      fprintf(stderr, __VA_ARGS__); \
      putc('\n', stderr); \
} while (0)

static void device_list(snd_pcm_stream_t stream)
{
      snd_ctl_t *handle;
      int card, err, dev, idx;
      snd_ctl_card_info_t *info;
      snd_pcm_info_t *pcminfo;
      snd_ctl_card_info_alloca(&info);
      snd_pcm_info_alloca(&pcminfo);

      card = -1;
      if (snd_card_next(&card) < 0 || card < 0) {
            error(_("no soundcards found..."));
            return;
      }
      printf(_("**** List of %s Hardware Devices ****\n"),
             snd_pcm_stream_name(stream));
      while (card >= 0) {
            char name[32];
            sprintf(name, "hw:%d", card);
            if ((err = snd_ctl_open(&handle, name, 0)) < 0) {
                  error("control open (%i): %s", card, snd_strerror(err));
                  goto next_card;
            }
            if ((err = snd_ctl_card_info(handle, info)) < 0) {
                  error("control hardware info (%i): %s", card, snd_strerror(err));
                  snd_ctl_close(handle);
                  goto next_card;
            }
            dev = -1;
            while (1) {
                  unsigned int count;
                  if (snd_ctl_pcm_next_device(handle, &dev)<0)
                        error("snd_ctl_pcm_next_device");
                  if (dev < 0)
                        break;
                  snd_pcm_info_set_device(pcminfo, dev);
                  snd_pcm_info_set_subdevice(pcminfo, 0);
                  snd_pcm_info_set_stream(pcminfo, stream);
                  if ((err = snd_ctl_pcm_info(handle, pcminfo)) < 0) {
                        if (err != -ENOENT)
                              error("control digital audio info (%i): %s", card, snd_strerror(err));
                        continue;
                  }
                  printf(_("card %i: [%s,%i] %s [%s], device %i: %s [%s]\n"),
			 card, name, dev, snd_ctl_card_info_get_id(info), snd_ctl_card_info_get_name(info),
                        dev,
                        snd_pcm_info_get_id(pcminfo),
                        snd_pcm_info_get_name(pcminfo));
                  count = snd_pcm_info_get_subdevices_count(pcminfo);
                  printf( _("  Subdevices: %i/%i\n"),
                        snd_pcm_info_get_subdevices_avail(pcminfo), count);
                  for (idx = 0; idx < (int)count; idx++) {
                        snd_pcm_info_set_subdevice(pcminfo, idx);
                        if ((err = snd_ctl_pcm_info(handle, pcminfo)) < 0) {
                              error("control digital audio playback info (%i): %s", card, snd_strerror(err));
                        } else {
                              printf(_("  Subdevice #%i: %s\n"),
                                    idx, snd_pcm_info_get_subdevice_name(pcminfo));
                        }
                  }
            }
            snd_ctl_close(handle);
      next_card:
            if (snd_card_next(&card) < 0) {
                  error("snd_card_next");
                  break;
            }
      }
}


int main (int argc, char *argv[])
{
  device_list(SND_PCM_STREAM_CAPTURE);
  device_list(SND_PCM_STREAM_PLAYBACK);
}
