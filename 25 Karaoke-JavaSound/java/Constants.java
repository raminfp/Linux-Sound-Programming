

public class Constants {

    public static final String CHINESE_FONT = "WenQuanYi Zen Hei";
 
    /**
     * If there are many frames per buffer then the accuracy
     * of things like pitch detection improves.
     * On the other hand, latency is increased so the singer's
     * notes will be behind what they sing.
     * 512 gives a buffer size where latency seems okay and
     * pitch can be detected down to about 80Hz.
     *
     * But more accurate pitch detection happens if the pitch
     * buffer is larger, so PITCH_BUFFER_SCALE gives the scale factor
     */
    public static final int FRAMES_PER_BUFFER = 512;
    public static final int PITCH_BUFFER_SCALE = 4;
    public static final int PITCH_BUFFER_SIZE = PITCH_BUFFER_SCALE * FRAMES_PER_BUFFER;


    public static final int MIDI_NOTE_ON = 0x90;
    public static final int MIDI_NOTE_OFF = 0x80;

    // From http://www.phys.unsw.edu.au/jw/notes.html
    // Note names, MIDI numbers and frequencies
    public static final int MIDI_NOTE_A0 = 21;
    public static final int MIDI_NOTE_C8 = 108;

    public static final int NO_TICK = -1;

    public static final int MIDI_LYRIC_TYPE = 5;
    public static final int MIDI_TEXT_TYPE = 1;
    public static final int MIDI_END_OF_TRACK = 47;

    // This seems to be the tempo of the Songken
    // public static final int SONGKEN_TEMPO_IN_MPQ = 400000;
    public static final int SONGKEN_TEMPO_IN_MPQ = 0x070000;
}
