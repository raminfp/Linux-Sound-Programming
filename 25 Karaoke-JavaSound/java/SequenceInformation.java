

import javax.sound.midi.*;
import java.util.Vector;

public class SequenceInformation {

    private static Sequence sequence = null;
    private static Vector<LyricLine> lyricLines = null;
    private static Vector<DurationNote> melodyNotes = null;
    private static int lang = -1;
    private static String title = null;
    private static String performer = null;
    private static int maxNote;
    private static int minNote;

    private static int melodyChannel = -1;// no such value // default for Songken

    public static void setSequence(Sequence seq) {
	if (sequence != seq) {
	    sequence = seq;
	    lyricLines = null;
	    melodyNotes = null;
	    lang = -1;
	    title = null;
	    performer = null;
	    maxNote = -1;
	    minNote = Integer.MAX_VALUE;
	}
    }

    public static long getTickLength() {
	return sequence.getTickLength();
    }

    public static int getMelodyChannel() {
	boolean firstNoteSeen[] = {false, false, false, false, false, false, false, false,
				   false, false, false, false, false, false, false, false};
	boolean possibleChannel[] = {false, false, false, false, false, false, false, false,
				   false, false, false, false, false, false, false, false};
	if (melodyChannel != -1) {
	    return melodyChannel;
	}

	if (lyricLines == null) {
	    lyricLines = getLyrics();
	}

	long startLyricTick = ((LyricLine) lyricLines.get(0)).startTick;
	Debug.printf("Lyrics start at %d\n", startLyricTick);

	Track[]	tracks = sequence.getTracks();
	for (int nTrack = 0; nTrack < tracks.length; nTrack++) {
	    // out("Track " + nTrack + ":");
	    // out("-----------------------");
	    Track track = tracks[nTrack];
	    for (int nEvent = 0; nEvent < track.size(); nEvent++) {
		MidiEvent evt = track.get(nEvent);
		MidiMessage msg = evt.getMessage();
		if (msg instanceof ShortMessage) {
		    ShortMessage smsg= (ShortMessage) msg;
		    int channel = smsg.getChannel();
		    if (firstNoteSeen[channel]) {
			continue;
		    }
		    if (smsg.getCommand() == Constants.MIDI_NOTE_ON) {
			long tick = evt.getTick();
			Debug.printf("First note on for channel %d at tick %d\n",
					  channel, tick);
			firstNoteSeen[channel] = true;
			if (Math.abs(startLyricTick - tick) < 10) {
			    // close enough - we hope!
			    melodyChannel = channel;
			    possibleChannel[channel] = true;
			    Debug.printf("Possible melody channel is %d\n", channel);
			}
			if (tick > startLyricTick + 11) {
			    break;
			}
		    }
		}			  
					 


		//output(event);
	    }
	    // out("---------------------------------------------------------------------------");
	}

	return melodyChannel;
    }

    public static int getLanguage() {
	return lang;
    }

    public static String getTitle() {
	return title;
    }

    public static String getPerformer() {
	return performer;
    }

    public static void dumpLyrics() {
	for (LyricLine lyric: lyricLines) {
	    System.out.println(lyric.line);
	}
    }


    /**
     * Build a vector of lyric lines
     * Each line has a start and an end tick
     * and a string for the lyrics in that line
     */
    public static Vector<LyricLine> getLyrics() {
	if (lyricLines != null) {
	    return lyricLines;
	}

	lyricLines = new Vector<LyricLine> ();
	LyricLine nextLyricLine = new LyricLine();
	StringBuffer buff = new StringBuffer();
	long ticks = 0L;

	Track[] tracks = sequence.getTracks();
	for (int nTrack = 0; nTrack < tracks.length; nTrack++) {
	    for (int n = 0; n < tracks[nTrack].size(); n++) {
		MidiEvent evt = tracks[nTrack].get(n);
		MidiMessage msg = evt.getMessage();
		ticks = evt.getTick();

		if (msg instanceof MetaMessage) {
		    Debug.println("Got a meta mesg in seq");
		    if (((MetaMessage) msg).getType() == Constants.MIDI_TEXT_TYPE) {
			MetaMessage message = (MetaMessage) msg;

			byte[] data = message.getData();
			String str = new String(data);
			Debug.println("Got a text mesg in seq \"" + str + "\" " + ticks);

			if (ticks == 0) {
			    if (str.startsWith("@L")) {
				lang = decodeLang(str.substring(2));
			    } else if (str.startsWith("@T")) {
				if (title == null) {
				    title = str.substring(2);
				} else {
				    performer = str.substring(2);
				}
			    }
				
			}
			if (ticks > 0) {
			    //if (str.equals("\r") || str.equals("\n")) {
			    if ((data[0] == '/') || (data[0] == '\\')) {
				if (buff.length() == 0) {
				    // blank line -  maybe at start of song
				    // fix start time from NO_TICK
				    nextLyricLine.startTick = ticks;
				} else {
				    nextLyricLine.line = buff.toString();
				    nextLyricLine.endTick = ticks;
				    lyricLines.add(nextLyricLine);
				    buff.delete(0, buff.length());
				    
				    nextLyricLine = new LyricLine();
				}
				buff.append(str.substring(1));
			    } else {
				if (nextLyricLine.startTick == Constants.NO_TICK) {
				    nextLyricLine.startTick = ticks;
				}
				buff.append(str);
			    }
			}
		    }
		}
	    }
	    // save last line (but only once)
	    if (buff.length() != 0) {
		nextLyricLine.line = buff.toString();
		nextLyricLine.endTick = ticks;
		lyricLines.add(nextLyricLine);
		buff.delete(0, buff.length());		    
	    }
	}
	if (Debug.DEBUG) {
	    dumpLyrics();
	}
	return lyricLines;
    }

    /**
     * gets a vector of lyric notes
     * side-effect: sets last tick
     */
    public static Vector<DurationNote> getMelodyNotes() {
	if (melodyChannel == -1) {
	    getMelodyChannel();
	}

	if (melodyNotes != null) {
	    return melodyNotes;
	}

	melodyNotes = new Vector<DurationNote> ();
	Vector<DurationNote> unresolvedNotes = new Vector<DurationNote> ();
   
	Track[] tracks = sequence.getTracks();
	for (int nTrack = 0; nTrack < tracks.length; nTrack++) {
	    for (int n = 0; n < tracks[nTrack].size(); n++) {
		MidiEvent evt = tracks[nTrack].get(n);
		MidiMessage msg = evt.getMessage();
		long ticks = evt.getTick();

		if (msg instanceof ShortMessage) {
		    ShortMessage smsg= (ShortMessage) msg;
		    if (smsg.getChannel() == melodyChannel) {
			int note = smsg.getData1();
			if (note < Constants.MIDI_NOTE_A0 || note > Constants.MIDI_NOTE_C8) {
			    continue;
			}

			if (smsg.getCommand() == Constants.MIDI_NOTE_ON) {
			    // note on
			    DurationNote dnote = new DurationNote(ticks, note);
			    melodyNotes.add(dnote);
			    unresolvedNotes.add(dnote);

			} else if (smsg.getCommand() == Constants.MIDI_NOTE_OFF) {
			    // note off
			    for (int m = 0; m < unresolvedNotes.size(); m++) {
				DurationNote dnote = unresolvedNotes.elementAt(m);
				if (dnote.note == note) {
				    dnote.duration = ticks - dnote.startTick;
				    dnote.endTick = ticks;
				    unresolvedNotes.remove(m);
				}
			    }
				    
			}

		    }
		}
	    }
	}
	return melodyNotes;
    }

    public static int getMaxMelodyNote() {
	if (maxNote == -1) {
	    getMaxMin();
	}
	return maxNote;
    }

    public static int getMinMelodyNote() {
	if (minNote == Integer.MAX_VALUE) {
	    getMaxMin();
	}
	return minNote;
    }

    private static void getMaxMin() {
	StringBuffer buff = new StringBuffer();

	Track[] tracks = sequence.getTracks();
	for (int nTrack = 0; nTrack < tracks.length; nTrack++) {
	    for (int n = 0; n < tracks[nTrack].size(); n++) {
		MidiEvent evt = tracks[nTrack].get(n);
		MidiMessage msg = evt.getMessage();
		long ticks = evt.getTick();

		if (msg instanceof ShortMessage) {
		    ShortMessage smsg= (ShortMessage) msg;
		    if (smsg.getChannel() == melodyChannel) {
			if (smsg.getCommand() != Constants.MIDI_NOTE_OFF) {
			    // not note on
			    continue;
			}
			int note = smsg.getData1();
			if (note < 21 || note > 107) {
			    continue;
			}
			//Debug.println("Note is " + note);
			if (note > maxNote) {
			    Debug.println("Setting max to "+ note);
			    maxNote = note;
			} else if (note < minNote) {
			    minNote = note;
			}
		    }
		}
	    }
	}
    }

    private static int decodeLang(String str) {
	if (str.equals("ENG")) {
	    return SongInformation.ENGLISH;
	}
	if (str.equals("CHI")) {
	    return SongInformation.CHINESE1;
	}
	return -1;
    }
}
