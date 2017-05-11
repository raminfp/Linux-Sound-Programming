#include <alsa/asoundlib.h>
#include <alsa/mixer.h>
#include <stdlib.h>

int main(int argc, char **argv) {

    snd_mixer_t *mixer;
    snd_mixer_selem_id_t *ident;
    snd_mixer_elem_t *elem;
    long min, max;
    long old_volume, volume;

    snd_mixer_open(&mixer, 0);
    snd_mixer_attach(mixer, "default");
    snd_mixer_selem_register(mixer, NULL, NULL);
    snd_mixer_load(mixer);

    snd_mixer_selem_id_alloca(&ident);
    snd_mixer_selem_id_set_index(ident, 0);
    snd_mixer_selem_id_set_name(ident, "Master");
    elem = snd_mixer_find_selem(mixer, ident);
    snd_mixer_selem_get_playback_volume_range(elem, &min, &max);
    snd_mixer_selem_get_playback_volume(elem, 0, &old_volume);
    printf("Min %ld max %ld current volume %ld\n", min, max, old_volume);

    if (argc < 2) {
	fprintf(stderr, "Usage: %s volume (%ld - %ld)\n", argv[0], min, max);
	exit(1);
    }
    volume = atol(argv[1]);
    snd_mixer_selem_set_playback_volume_all(elem, volume);
    printf("Volume reset to %ld\n", volume);

    exit(0);
}

