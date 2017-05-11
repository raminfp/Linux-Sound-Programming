/*
 * KARConverter.java
 *
 * The output from decodnig the Sonken data is not in
 * the format required by the KAR "standard".
 * e.g. we need @T for the title,
 * and LYRIC events need to be changed to TEXT events
 * Tempo has to be changed too
 *
 */

import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;

import javax.sound.midi.MidiSystem;
import javax.sound.midi.InvalidMidiDataException;
import javax.sound.midi.Sequence;
import javax.sound.midi.Track;
import javax.sound.midi.MidiEvent;
import javax.sound.midi.MidiMessage;
import javax.sound.midi.ShortMessage;
import javax.sound.midi.MetaMessage;
import javax.sound.midi.SysexMessage;
import javax.sound.midi.Receiver;




public class KARConverter {
    private static int LYRIC = 5;
    private static int TEXT = 1;

    private static boolean firstLyricEvent = true;

    public static void main(String[] args) {
	if (args.length != 1) {
	    out("KARConverter: usage:");
	    out("\tjava KARConverter <file>");
	    System.exit(1);
	}
	/*
	 *	args[0] is the common prefix of the two files
	 */
	File	inFile = new File(args[0] + ".mid");
	File	outFile = new File(args[0] + ".kar");

	/*
	 *	We try to get a Sequence object, which the content
	 *	of the MIDI file.
	 */
	Sequence	inSequence = null;
	Sequence	outSequence = null;
	try {
	    inSequence = MidiSystem.getSequence(inFile);
	} catch (InvalidMidiDataException e) {
	    e.printStackTrace();
	    System.exit(1);
	} catch (IOException e) {
	    e.printStackTrace();
	    System.exit(1);
	}

	if (inSequence == null) {
	    out("Cannot retrieve Sequence.");
	} else {
	    try {
		outSequence = new Sequence(inSequence.getDivisionType(),
					   inSequence.getResolution());
	    } catch(InvalidMidiDataException e) {
		e.printStackTrace();
		System.exit(1);
	    }
		    
	    createFirstTrack(outSequence);
	    Track[]	tracks = inSequence.getTracks();
	    fixTrack(tracks[0], outSequence);
	}
	FileOutputStream outStream = null;
	try {
	    outStream = new FileOutputStream(outFile);
	    MidiSystem.write(outSequence, 1, outStream);
	} catch(Exception e) {
	    e.printStackTrace();
	    System.exit(1);
	}
    }


    public static void fixTrack(Track oldTrack, Sequence seq) {
	Track lyricTrack = seq.createTrack();
	Track dataTrack = seq.createTrack();

	int nEvent = fixHeader(oldTrack, lyricTrack);
	System.out.println("nEvent " + nEvent);
	for ( ; nEvent < oldTrack.size(); nEvent++) {
	    MidiEvent event = oldTrack.get(nEvent);
	    if (isLyricEvent(event)) {
		event = convertLyricToText(event);
		lyricTrack.add(event);
	    } else {
		dataTrack.add(event);
	    }
	}
    }

    public static int fixHeader(Track oldTrack, Track lyricTrack) {
	int nEvent;

	// events at 0-10 are meaningless
	// events at 11, 12 should be the language code,
	// but maybe at 12, 13
	nEvent = 11;
	MetaMessage lang1 = (MetaMessage) (oldTrack.get(nEvent).getMessage());
	String val = new String(lang1.getData());
	if (val.equals("@")) {
	    // try 12
	    lang1 = (MetaMessage) (oldTrack.get(++nEvent).getMessage());
	}		
	MetaMessage lang2 = (MetaMessage) (oldTrack.get(++nEvent).getMessage());
	String lang = new String(lang1.getData()) +
	    new String(lang2.getData());
	System.out.println("Lang " + lang);
	byte[] karLang = getKARLang(lang);

	MetaMessage msg = new MetaMessage();
	try {
	    msg.setMessage(TEXT, karLang, karLang.length);
	    MidiEvent evt = new MidiEvent(msg, 0L);
	    lyricTrack.add(evt);
	} catch(InvalidMidiDataException e) {
	}

	// song title is next
	StringBuffer titleBuff = new StringBuffer();
	for (nEvent = 15; nEvent < oldTrack.size(); nEvent++) {
	    MidiEvent event = oldTrack.get(nEvent);
	    msg = (MetaMessage) (event.getMessage());
	    String contents = new String(msg.getData());
	    if (contents.equals("@")) {
		break;
	    }
	    if (contents.equals("\r\n")) {
		continue;
	    }
	    titleBuff.append(contents);
	}
	String title = "@T" + titleBuff.toString();
	System.out.println("Title '" + title +"'");
	byte[] titleBytes = title.getBytes();

	msg = new MetaMessage();
	try {
	    msg.setMessage(TEXT, titleBytes, titleBytes.length);
	    MidiEvent evt = new MidiEvent(msg, 0L);
	    lyricTrack.add(evt);
	} catch(InvalidMidiDataException e) {
	}

	
	// skip the next 2 @'s
	for (int skip = 0; skip < 2; skip++) {
	    for (++nEvent; nEvent < oldTrack.size(); nEvent++) {
		MidiEvent event = oldTrack.get(nEvent);
		msg = (MetaMessage) (event.getMessage());
		String contents = new String(msg.getData());
		if (contents.equals("@")) {
		    break;
		}
	    }
	}

	// then the singer
	StringBuffer singerBuff = new StringBuffer();
	for (++nEvent; nEvent < oldTrack.size(); nEvent++) {
	    MidiEvent event = oldTrack.get(nEvent);
	    if (event.getTick() != 0) {
		break;
	    }
	    if (! isLyricEvent(event)) {
		break;
	    }

	    msg = (MetaMessage) (event.getMessage());
	    String contents = new String(msg.getData());
	    if (contents.equals("\r\n")) {
		continue;
	    }
	    singerBuff.append(contents);
	}
	String singer = "@T" + singerBuff.toString();
	System.out.println("Singer '" + singer +"'");

	byte[] singerBytes = singer.getBytes();

	msg = new MetaMessage();
	try {
	    msg.setMessage(1, singerBytes, singerBytes.length);
	    MidiEvent evt = new MidiEvent(msg, 0L);
	    lyricTrack.add(evt);
	} catch(InvalidMidiDataException e) {
	}

	return nEvent;
    }

    public static boolean isLyricEvent(MidiEvent event) {
	if (event.getMessage() instanceof MetaMessage) {
	    MetaMessage msg = (MetaMessage) (event.getMessage());
	    if (msg.getType() == LYRIC) {
		return true;
	    }
	}
	return false;
    }

    public static MidiEvent convertLyricToText(MidiEvent event) {
	if (event.getMessage() instanceof MetaMessage) {
	    MetaMessage msg = (MetaMessage) (event.getMessage());
	    if (msg.getType() == LYRIC) {
		byte[] newMsgData = null;
		if (firstLyricEvent) {
		    // need to stick a \ at the front
		    newMsgData = new byte[msg.getData().length + 1];
		    System.arraycopy(msg.getData(), 0, newMsgData, 1, msg.getData().length);
		    newMsgData[0] = '\\';
		    firstLyricEvent = false;
		} else {
		    newMsgData = msg.getData();
		    if ((new String(newMsgData)).equals("\r\n")) {
			newMsgData = "\\".getBytes();
		    }
		}
		try {
		    /*
		    msg.setMessage(TEXT, 
				   msg.getData(), 
				   msg.getData().length);
		    */
		    msg.setMessage(TEXT, 
				   newMsgData, 
				   newMsgData.length);
		} catch(InvalidMidiDataException e) {
		    e.printStackTrace();
		}
	    }
	}
	return event;
    }

    public static byte[] getKARLang(String lang) {
	System.out.println("lang is " + lang);
	if (lang.equals("12")) {
	    return "@LENG".getBytes();
	}
	
	// don't know any other language specs, so guess
	if (lang.equals("01")) {
	    return "@LCHI".getBytes();
	}
	if (lang.equals("02")) {
	    return "@LCHI".getBytes();
	}
	if (lang.equals("08")) {
	    return "@LCHI".getBytes();
	}
	if (lang.equals("09")) {
	    return "@LCHI".getBytes();
	}
	if (lang.equals("07")) {
	    return "@LCHI".getBytes();
	}
	if (lang.equals("")) {
	    return "@L".getBytes();
	}
	if (lang.equals("")) {
	    return "@LENG".getBytes();
	}
	if (lang.equals("")) {
	    return "@LENG".getBytes();
	}
	if (lang.equals("")) {
	    return "@LENG".getBytes();
	}
	if (lang.equals("")) {
	    return "@LENG".getBytes();
	}
	if (lang.equals("")) {
	    return "@LENG".getBytes();
	}


	return ("@L" + lang).getBytes();
    }


    public static void copyNotesTrack(Track oldTrack, Sequence seq) {
	Track newTrack = seq.createTrack();

	for (int nEvent = 0; nEvent < oldTrack.size(); nEvent++)
	    {
		MidiEvent event = oldTrack.get(nEvent);

		newTrack.add(event);
	    }
    }

    public static void createFirstTrack(Sequence sequence) {
	Track track = sequence.createTrack();
	MetaMessage msg1 = new MetaMessage();
	MetaMessage msg2 = new MetaMessage();

	byte data[] = "Soft Karaoke".getBytes();
	try {
	    msg1.setMessage(3, data, data.length);
	} catch(InvalidMidiDataException e) {
	    e.printStackTrace();
	    return;
	}
	MidiEvent event = new MidiEvent(msg1, 0L);
	track.add(event);

	data = "@KMIDI KARAOKE FILE".getBytes();
	try {
	    msg2.setMessage(1, data, data.length);
	} catch(InvalidMidiDataException e) {
	    e.printStackTrace();
	    return;
	}
	MidiEvent event2 = new MidiEvent(msg2, 0L);
	track.add(event2);
    }

    public static void output(MidiEvent event)
    {
	MidiMessage	message = event.getMessage();
	long		lTicks = event.getTick();
    }



    private static void out(String strMessage)
    {
	System.out.println(strMessage);
    }
}



/*** KARConverter.java ***/

