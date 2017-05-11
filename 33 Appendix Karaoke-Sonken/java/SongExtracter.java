
import java.io.*;
import javax.sound.midi.*;
import java.nio.charset.Charset;

public class SongExtracter {
    private static final boolean DEBUG = false;

    private String[] dataFiles = new String[] {
	"DTSMUS00.DKD", "DTSMUS01.DKD", "DTSMUS02.DKD",
	"DTSMUS03.DKD", "DTSMUS04.DKD", "DTSMUS05.DKD",
	"DTSMUS06.DKD", "DTSMUS07.DKD"};
    private String superBlockFileName = dataFiles[0];
    private static final String DATADIR = "/home/newmarch/Music/karaoke/sonken/";
    private static final String SONGDIR ="/home/newmarch/Music/karaoke/sonken/songs/";
    //private static final String SONGDIR ="/server/KARAOKE/KARAOKE/Sonken/";
    private static final long SUPERBLOCK_OFFSET = 0x200;
    private static final long BLOCK_MULTIPLIER = 0x800;
    private static final long FILE_SIZE = 0x3F800000L;

    private static final int SIZE_UINT = 4;
    private static final int SIZE_USHORT = 2;

    private static final int ENGLISH = 12;

    public RawSong getRawSong(int songNumber) 
	throws java.io.IOException, 
	       java.io.FileNotFoundException {
	if (songNumber < 1) {
	    throw new FileNotFoundException();
	}

	// song number in files is one less than song number in books, so
	songNumber--;

	long locationIndexTable = getTableIndexFromSuperblock(songNumber);
	debug("Index table at %X\n", locationIndexTable);

	long locationSongDataBlock = getSongIndex(songNumber, locationIndexTable);

	// Now we are at the start of the data block
	return readRawSongData(locationSongDataBlock);

	//debug("Data block at %X\n", songStart);
    }

    private long getTableIndexFromSuperblock(int songNumber)
	throws java.io.IOException, 
	       java.io.FileNotFoundException {
	// index into superblock of table of song offsets
	int superBlockIdx = songNumber >> 8;

	debug("Superblock index %X\n", superBlockIdx);
	    
	File superBlockFile = new File(DATADIR + superBlockFileName);

        FileInputStream fstream = new FileInputStream(superBlockFile);

	fstream.skip(SUPERBLOCK_OFFSET + superBlockIdx * SIZE_UINT);
	debug("Skipping to %X\n", SUPERBLOCK_OFFSET + superBlockIdx*4);
	long superBlockValue = readUInt(fstream);

	// virtual address of the index table for this song
	long locationIndexTable = superBlockValue * BLOCK_MULTIPLIER;

	return locationIndexTable;
    }

    /*
     * Virtual address of song data block
     */
    private long getSongIndex(int songNumber, long locationIndexTable) 
	throws java.io.IOException, 
	       java.io.FileNotFoundException {
	// index of song into table of song ofsets
	int indexTableIdx = songNumber & 0xFF;
	debug("Index into index table %X\n", indexTableIdx);

	// translate virtual address to physical address
	int whichFile = (int) (locationIndexTable / FILE_SIZE);
	long indexTableStart =  locationIndexTable % FILE_SIZE;
	debug("Which file %d index into file %X\n", whichFile, indexTableStart);

	File songDataFile = new File(DATADIR + dataFiles[whichFile]);
        FileInputStream dataStream = new FileInputStream(songDataFile);
	dataStream.skip(indexTableStart + indexTableIdx * SIZE_UINT);
	debug("Song data index is at %X\n", indexTableStart + indexTableIdx*SIZE_UINT);

	long songStart = readUInt(dataStream) + indexTableStart;

	return songStart + whichFile * FILE_SIZE;
    }

    private RawSong readRawSongData(long locationSongDataBlock) 
	throws java.io.IOException {
	int whichFile = (int) (locationSongDataBlock / FILE_SIZE);
	long dataStart =  locationSongDataBlock % FILE_SIZE;
	debug("Which song file %d  into file %X\n", whichFile, dataStart);

	File songDataFile = new File(DATADIR + dataFiles[whichFile]);
        FileInputStream dataStream = new FileInputStream(songDataFile);
	dataStream.skip(dataStart);

	RawSong rs = new RawSong();
	rs.type = readUShort(dataStream);
	rs.compressedLyricLength = readUShort(dataStream);
	// discard next short
	readUShort(dataStream);
	rs.uncompressedLyricLength = readUShort(dataStream);
	debug("Type %X, cLength %X uLength %X\n", rs.type, rs.compressedLyricLength, rs.uncompressedLyricLength);

	// don't know what the next word is for, skip it
	//dataStream.skip(4);
	readUInt(dataStream);

	// get the compressed lyric
	rs.lyric = new byte[rs.compressedLyricLength];
	dataStream.read(rs.lyric);

	long toBoundary = 0;
	long songLength = 0;
	long uncompressedSongLength = 0;

	// get the song data
	if (rs.type == 0) {
	    // Midi file starts in 4 bytes time
	    songLength = readUInt(dataStream);
	    uncompressedSongLength = readUInt(dataStream);
	    System.out.printf("Song data length %d, uncompressed %d\n", 
			      songLength, uncompressedSongLength);
	    rs.uncompressedSongLength = uncompressedSongLength;

	    // next word is language again?
	    //toBoundary = 4;
	    //dataStream.skip(toBoundary);
	    readUInt(dataStream);
	} else {
	    // WMA starts on next 16-byte boundary
	    if( (dataStart + rs.compressedLyricLength + 12) % 16 != 0) {
		// dataStart already on 16-byte boundary, so just need extra since then
		toBoundary = 16 - ((rs.compressedLyricLength + 12) % 16);
		debug("Read lyric data to %X\n", dataStart + rs.compressedLyricLength + 12);
		debug("Length %X to boundary %X\n", rs.compressedLyricLength, toBoundary);
		dataStream.skip(toBoundary);
	    }
	    songLength = readUInt(dataStream);
	}

	rs.music = new byte[(int) songLength];
	dataStream.read(rs.music);

	return rs;
    }

    private long readUInt(InputStream is) throws IOException {
	long val = 0;
	for (int n = 0; n < SIZE_UINT; n++) {
	    int c = is.read();
	    val = (val << 8) + c;
	}
	debug("ReadUInt %X\n", val);
	return val;
    }

    private int readUShort(InputStream is) throws IOException {
	int val = 0;
	for (int n = 0; n < SIZE_USHORT; n++) {
	    int c = is.read();
	    val = (val << 8) + c;
	}
	debug("ReadUShort %X\n", val);
	return val;
    }

    void debug(String f, Object ...args) {
	if (DEBUG) {
	    System.out.printf("Debug: " + f, args);
	}
    }

    public Song getSong(RawSong rs) {
	Song song;
	if (rs.type == 0x8000) {
	    song = new WMASong(rs);
	} else {
	    song = new MidiSong(rs);
	}
	return song;
    }

    public static void main(String[] args) {
	if (args.length != 1) {
	    System.err.println("Usage: java SongExtractor <song numnber>");
	    System.exit(1);
	}

	SongExtracter se = new SongExtracter();
	try {
	    RawSong rs = se.getRawSong(Integer.parseInt(args[0]));
	    rs.dumpToFile(args[0]);

	    Song song = se.getSong(rs);
	    song.dumpToFile(args[0]);
	    song.dumpLyric();
	} catch(Exception e) {
	    e.printStackTrace();
	}
    }

    private class RawSong {
	/**
	 * type == 0x0 is Midi
	 * type == 0x8000 is WMA
	 */
	public int type;
	public int compressedLyricLength;
	public int uncompressedLyricLength;
	public long uncompressedSongLength; // only needed for compressed Midi
	public byte[] lyric;
	public byte[] music;

	public void dumpToFile(String fileName) throws IOException {
	    FileOutputStream fout = new FileOutputStream(SONGDIR + fileName + ".lyric");
	    fout.write(lyric);
	    fout.close();

	    fout = new FileOutputStream(SONGDIR + fileName + ".music");
	    fout.write(music);
	    fout.close();
	}
    }

    private class Song {
	public int type;
	public byte[] lyric;
	public byte[] music;
	protected Sequence sequence;
	protected int language = -1;

	public Song(RawSong rs) {
	    type = rs.type;
	    lyric = decodeLyric(rs.lyric,
				rs.uncompressedLyricLength);
	}

	/**
	 * Raw lyric is LZW compressed. Decompress it
	 */
	public byte[] decodeLyric(byte[] compressedLyric, long uncompressedLength) {
	    // uclen is short by at least 2 - other code adds 10 so we do too
	    // TODO: change LZW to use a Vector to build result so we don't have to guess at length
	    byte[] result = new byte[(int) uncompressedLength + 10];
	    LZW lzw = new LZW();
	    int len = lzw.expand(compressedLyric, compressedLyric.length, result);
	    System.out.printf("uncompressedLength %d, actual %d\n", uncompressedLength, len);
	    lyric = new byte[len];
	    System.arraycopy(result, 0, lyric, 0, (int) uncompressedLength);
	    return lyric;
	}

	public void dumpToFile(String fileName) throws IOException {
	    FileOutputStream fout = new FileOutputStream(SONGDIR + fileName + ".decodedlyric");
	    fout.write(lyric);
	    fout.close();
	    
	    fout = new FileOutputStream(SONGDIR + fileName + ".decodedmusic");
	    fout.write(music);
	    fout.close();
	    
	    fout = new FileOutputStream(SONGDIR + fileName + ".mid");
	    if (sequence == null)  {
		System.out.println("Seq is null");
	    } else {
		// type is MIDI type 0
		MidiSystem.write(sequence, 0, fout);
	    }
	}

	public void dumpLyric() {
	    for (int n = 0; n < lyric.length; n += 4) {
		if (lyric[n] == '\r') {
		    System.out.println();
		} else {
		    System.out.printf("%c", lyric[n] & 0xFF);
		}
	    }
	    System.out.println();
	    System.out.printf("Language is %X\n", getLanguageCode()); 
	}

	/**
	 * Lyric contains the language code as string @00@NN in header section
	 */
	public int getLanguageCode() {
	    int lang = 0;

	    // Look for @00@NN and return NN
	    for (int n = 0; n < lyric.length-20; n += 4) {
		if (lyric[n] == (byte) '@' &&
		    lyric[n+4] == (byte) '0' &&
		    lyric[n+8] == (byte) '0' &&
		    lyric[n+12] == (byte) '@') {
		    lang = ((lyric[n+16]-'0') << 4) + lyric[n+20]-'0';
		    break;
		}
	    }
	    return lang;
	}

	/**
	 * Lyric is in a language specific encoding. Translate to Unicode UTF-8.
	 * Not all languages are handled because I don't have a full set of examples
	 */
	public byte[] lyricToUnicode(byte[] bytes) {
	    if (language == -1) {
		language = getLanguageCode();
	    }
	    switch (language) {
	    case SongInformation.ENGLISH:
		return bytes;

	    case SongInformation.KOREAN: {
 		Charset charset = Charset.forName("gb2312");
		String str = new String(bytes, charset);
		bytes = str.getBytes();
		System.out.println(str);
		return bytes;
	    }

	    case SongInformation.CHINESE1:
	    case SongInformation.CHINESE2:
	    case SongInformation.CHINESE8:
	    case SongInformation.CHINESE131:
	    case SongInformation.TAIWANESE3:
	    case SongInformation.TAIWANESE7:
	    case SongInformation.CANTONESE:
		Charset charset = Charset.forName("gb2312");
		String str = new String(bytes, charset);
		bytes = str.getBytes();
		System.out.println(str);
		return bytes;
	    }
	    // language not handled
	    return bytes;
	}

	public void durationToOnOff() {

	}

	public Track createSequence() {
	    Track track;

	    try {
		sequence = new Sequence(Sequence.PPQ, 30);
	    } catch(InvalidMidiDataException e) {
		// help!!!
	    }
	    track = sequence.createTrack();
	    addLyricToTrack(track);
	    return track;
	}

	public void addMsgToTrack(MidiMessage msg, Track track, long tick) {
	    MidiEvent midiEvent = new MidiEvent(msg, tick);

	    
	    // No need to sort or delay insertion. From the Java API
	    // "The list of events is kept in time order, meaning that this
	    // event inserted at the appropriate place in the list"
	    track.add(midiEvent);
	}

	/**
	 * return byte as int, converting to unsigned if needed
	 */
	protected int ub2i(byte b) {
	    return  b >= 0 ? b : 256 + b;
	}

	public void addLyricToTrack(Track track) {
	    long lastDelay = 0;
	    int offset = 0;
	    int data0;
	    int data1;
	    final int LYRIC = 0x05;
	    MetaMessage msg;

	    while (offset < lyric.length-4) {
		int data3 = ub2i(lyric[offset+3]);
		int data2 = ub2i(lyric[offset+2]);
		data0 = ub2i(lyric[offset]);
		data1 = ub2i(lyric[offset+1]);

		long delay = (data3 << 8) + data2;

		offset += 4;
		byte[] data;
		int len;
		long tick;

		// 	System.out.printf("Lyric offset %X char %X after %d with delay %d made of %d %d\n", offset, data0, lastDelay, delay, lyric[offset-1], lyric[offset-2]);

		if (data1 == 0) {
		    data = new byte[] {(byte) data0}; //, (byte) MetaMessage.META};
		} else {
		    data = new byte[] {(byte) data0, (byte) data1}; // , (byte) MetaMessage.META};
		}
		data = lyricToUnicode(data);
		    
		msg = new MetaMessage();

		if (delay > 0) {
		    tick = delay;
		    lastDelay = delay;
		} else {
		    tick = lastDelay;
		}
		
		try {
		    msg.setMessage(LYRIC, data, data.length);
		} catch(InvalidMidiDataException e) {
		    e.printStackTrace();
		    continue;
		}
		addMsgToTrack(msg, track, tick);
	    }
	}

    }

    private class WMASong extends Song {

	public WMASong(RawSong rs) {
	    // We want to decode the lyric, but just copy the music data
	    super(rs);
	    music = rs.music;
	    createSequence();
	}

	public void dumpToFile(String fileName) throws IOException {
	    System.out.println("Dumping WMA to " + fileName + ".wma");
	    super.dumpToFile(fileName);
	    FileOutputStream fout = new FileOutputStream(fileName + ".wma");
	    fout.write(music);
	    fout.close();
	}

    }

    private class MidiSong extends Song {

        private String[] keyNames = {"C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B"};

	public MidiSong(RawSong rs) {
	    // We want the decoded lyric plus also need to decode the music
	    // and then turn it into a Midi sequence
	    super(rs);
	    decodeMusic(rs);
	    createSequence();
	}

	public void dumpToFile(String fileName) throws IOException {
	    System.out.println("Dumping Midi to " + fileName);
	    super.dumpToFile(fileName);
	}

        public String getKeyName(int nKeyNumber)
        {
	    if (nKeyNumber > 127)
                {
		    return "illegal value";
                }
	    else
                {
		    int     nNote = nKeyNumber % 12;
		    int     nOctave = nKeyNumber / 12;
		    return keyNames[nNote] + (nOctave - 1);
                }
        }

	public byte[] decodeMusic(RawSong rs) {
	    byte[]  compressedMusic = rs.music;
	    long uncompressedSongLength = rs.uncompressedSongLength;

	    // TODO: change LZW to use a Vector to build result so we don't have to guess at length
	    byte[] expanded = new byte[(int) uncompressedSongLength + 20];
	    LZW lzw = new LZW();
	    int len = lzw.expand(compressedMusic, compressedMusic.length, expanded);
	    System.out.printf("Uncompressed %d, Actual %d\n", compressedMusic.length, len);
	    music = new byte[len];
	    System.arraycopy(expanded, 0, music, 0, (int) len);


	    return music;
	}

	public Track createSequence() {
	    Track track = super.createSequence();
	    addMusicToTrack(track);
	    return track;
	}



	public void addMusicToTrack(Track track) {
	    int timeLine = 0;
	    int offset = 0;
	    int midiChannelNumber = 1;

	    /* From http://board.midibuddy.net/showpost.php?p=533722&postcount=31
	       Block of 5 bytes :
	       xx xx xx xx xx
	       1st byte = Delay Time
	       2nd byte = Delay Time when the velocity will be 0, 
	       this one will generate another midi event 
	       with velocity 0 (see above).
	       3nd byte = Event, for example 9x : Note On for channel x+1,
	       cx for PrCh, bx for Par, ex for Pitch Bend....
	       4th byte = Note
	       5th byte = Velocity
	    */
	    System.out.println("Adding music to track");
	    while (offset < music.length - 5) {

		int startDelayTime = ub2i(music[offset++]);
		int endDelayTime = ub2i(music[offset++]);
		int event = ub2i(music[offset++]);
		int data1 = ub2i(music[offset++]);
		int data2 = ub2i(music[offset++]);


		int tick = timeLine + startDelayTime;
		System.out.printf("Offset %X event %X timeline %d\n", offset, event & 0xFF, tick);

		ShortMessage msg = new ShortMessage();
		ShortMessage msg2 = null;

		try {
		    // For Midi event types see http://www.midi.org/techspecs/midimessages.php
		    switch (event & 0xF0) {
		    case ShortMessage.CONTROL_CHANGE:  // Control Change 0xB0
		    case ShortMessage.PITCH_BEND:  // Pitch Wheel Change 0xE0
			msg.setMessage(event, data1, data2);
			/*
			  writeChannel(midiChannelNumber, chunk[2], false);
			  writeChannel(midiChannelNumber, chunk[3], false);
			  writeChannel(midiChannelNumber, chunk[4], false);
			*/
			break;

		    case ShortMessage.PROGRAM_CHANGE: // Program Change 0xC0
		    case ShortMessage.CHANNEL_PRESSURE: // Channel Pressure (After-touch) 0xD0
			msg.setMessage(event, data1, 0);
			break;

		    case 0x00:
			// case 0x90:
			// Note on
			int note = data1;
			int velocity = data2;

			/* We have to generate a pair of note on/note off.
			   The C code manages getting the order of events
			   done correctly by keeping a list of note off events
			   and sticking them into the Midi sequence when appropriate.
			   The Java add() looks after timing for us, so we'll
			   generate a note off first and add it, and then do the note on
			*/
			System.out.printf("Note on %s at %d, off at %d at offset %X channel %d\n", 
					  getKeyName(note),
					  tick, tick + endDelayTime, offset, (event &0xF)+1);
			// ON
			msg.setMessage(ShortMessage.NOTE_ON | (event & 0xF),
				       note, velocity);

			// OFF
			msg2 = new ShortMessage();
			msg2.setMessage(ShortMessage.NOTE_OFF  | (event & 0xF), 
					note, velocity);

			break;

		    case 0xF0: // System Exclusive
			// We'll write the data as is to the buffer
			offset -= 3;
			// msg = SysexMessage();
			while (music[offset] != (byte) 0xF7) // bytes only go upto 127 GRRRR!!!
			    {
				//				writeChannel(midiChannelNumber, midiData[midiOffset], false);
				System.out.printf("sysex: %x\n", music[offset]);
				offset++;
				if (offset >= music.length) {
				    System.err.println("Run off end of array while processing Sysex");
				    break;
				}
			    }
			//			writeChannel(midiChannelNumber, midiData[midiOffset], false);
			offset++;
			System.out.printf("Ignoring sysex %02X\n", event);

			// ignore the message for now
			continue;
			// break;

		    default:
			System.out.printf("Unrecognized code %02X\n", event);
			continue;
		    }
		} catch(InvalidMidiDataException e) {
		    e.printStackTrace();
		}

		addMsgToTrack(msg, track, tick);
		if (msg2 != null ) {
		    if (endDelayTime <= 0) System.out.println("Start and end at same time");
		    addMsgToTrack(msg2, track, tick + endDelayTime);
		    msg2 = null;
		}

		timeLine = tick;
	    }
	}
    }
}
