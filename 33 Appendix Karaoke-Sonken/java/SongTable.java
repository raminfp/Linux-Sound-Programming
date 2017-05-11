
import java.util.Vector;
import java.io.FileInputStream;
import java.io.*;
import java.nio.charset.Charset;

// public class SongTable implements java.util.Iterator {
// public class SongTable extends  Vector<SongInformation> {
public class SongTable {

    private static final String SONG_INFO_FILE = "/home/newmarch/Music/karaoke/sonken/DTSMUS20.DKD";
    private static final long INFO_START = 0x9F23;

    public static final int ENGLISH = 0x12;
    
    private static Vector<SongInformation> allSongs;

    private Vector<SongInformation> songs = 
	new Vector<SongInformation>  ();

    public static long[] langCount = new long[0x23];

    public SongTable(Vector<SongInformation> songs) {
	this.songs = songs;
    }

    public SongTable() throws java.io.IOException, 
			      java.io.FileNotFoundException {
	FileInputStream fstream = new FileInputStream(SONG_INFO_FILE);
	fstream.skip(INFO_START);
	while (true) {
	    int len;
	    int lang;
	    long number;

	    len = fstream.read();
	    lang = fstream.read();
	    number = readShort(fstream);
	    if (len == 0xFF && lang == 0xFF && number == 0xFFFFL) {
		break;
	    }
	    byte[] bytes = new byte[len - 4];
	    fstream.read(bytes);
	    int endTitle;
	    // find null at end of title
	    for (endTitle = 0; bytes[endTitle] != 0; endTitle++)
		;
	    byte[] titleBytes = new byte[endTitle];
	    byte[] artistBytes = new byte[len - endTitle - 6];

	    System.arraycopy(bytes, 0, titleBytes, 0, titleBytes.length);
	    System.arraycopy(bytes, endTitle + 1,
			     artistBytes, 0, artistBytes.length);
	    String title = toUnicode(lang, titleBytes);
	    String artist = toUnicode(lang, artistBytes);
	    // System.out.printf("artist: %s, title: %s, lang: %d, number %d\n", artist, title, lang, number);
	    SongInformation info = new SongInformation(number,
						       title,
						       artist,
						       lang);
	    songs.add(info);

	    if (lang > 0x22) {
		//System.out.println("Illegal lang value " + lang + " at song " + number);
	    } else {
		langCount[lang]++;
	    }
	}
	allSongs = songs;
    }

    public void dumpTable() {
	for (SongInformation song: songs) {
	    System.out.println("" + (song.number+1) + " - " +
			       song.artist + " - " +
			       song.title);
	}
    }

    public java.util.Iterator<SongInformation> iterator() {
	return songs.iterator();
    }

    private int readShort(FileInputStream f)  throws java.io.IOException {
	int n1 = f.read();
	int n2 = f.read();
	return (n1 << 8) + n2;
    }

    private String toUnicode(int lang, byte[] bytes) {
	switch (lang) {
	case SongInformation.ENGLISH:
	case SongInformation.ENGLISH146:
	case SongInformation.PHILIPPINE:
	case SongInformation.PHILIPPINE148:
	    // case SongInformation.HINDI:
	case SongInformation.INDONESIAN:
	case SongInformation.SPANISH:
	    return new String(bytes);

	case SongInformation.CHINESE1:
	case SongInformation.CHINESE2:
	case SongInformation.CHINESE8:
	case SongInformation.CHINESE131:
	case SongInformation.TAIWANESE3:
	case SongInformation.TAIWANESE7:
	case SongInformation.CANTONESE:
            Charset charset = Charset.forName("gb2312");
            return new String(bytes, charset);

	case SongInformation.KOREAN:
	    charset = Charset.forName("euckr");
            return new String(bytes, charset);

	default:
	    return "";
	}
    }

    public SongInformation getNumber(long number) {
	for (SongInformation info: songs) {
	    if (info.number == number) {
		return info;
	    }
	}
	return null;
    }

    public SongTable titleMatches( String pattern) {
	Vector<SongInformation> matchSongs = 
	    new Vector<SongInformation>  ();

	for (SongInformation song: songs) {
	    if (song.titleMatch(pattern)) {
		matchSongs.add(song);
	    }
	}
	return new SongTable(matchSongs);
    }

     public SongTable artistMatches( String pattern) {
	Vector<SongInformation> matchSongs = 
	    new Vector<SongInformation>  ();

	for (SongInformation song: songs) {
	    if (song.artistMatch(pattern)) {
		matchSongs.add(song);
	    }
	}
	return new SongTable(matchSongs);
    }

      public SongTable numberMatches( String pattern) {
	Vector<SongInformation> matchSongs = 
	    new Vector<SongInformation>  ();

	for (SongInformation song: songs) {
	    if (song.numberMatch(pattern)) {
		matchSongs.add(song);
	    }
	}
	return new SongTable(matchSongs);
    }

    public String toString() {
	StringBuffer buf = new StringBuffer();
	for (SongInformation song: songs) {
	    buf.append(song.toString() + "\n");
	}
	return buf.toString();
    }
	
    public static void main(String[] args) {
	// for testing
	SongTable songs = null;
	try {
	    songs = new SongTable();
	} catch(Exception e) {
	    System.err.println(e.toString());
	    System.exit(1);
	}
	songs.dumpTable();
	System.exit(0);

	// Should print "54151 Help Yourself Tom Jones"
	System.out.println(songs.getNumber(54150).toString());

	// Should print "18062 伦巴(恋歌) 伦巴"
	System.out.println(songs.getNumber(18061).toString());

	System.out.println(songs.artistMatches("Tom Jones").toString());
	/* Prints
54151 Help Yourself Tom Jones
50213 Daughter Of Darkness Tom Jones
23914 DELILAH Tom Jones
52834 Funny Familiar Forgotten Feelings Tom Jones
54114 Green green grass of home Tom Jones
54151 Help Yourself Tom Jones
55365 I (WHO HAVE NOTHING) TOM JONES
52768 I Believe Tom Jones
55509 I WHO HAVE NOTHING TOM JONES
55594 I'll Never Fall Inlove Again Tom Jones
55609 I'm Coming Home Tom Jones
51435 It's Not Unusual Tom Jones
55817 KISS Tom Jones
52842 Little Green Apples Tom Jones
51439 Love Me Tonight Tom Jones
56212 My Elusive Dream TOM JONES
56386 ONE DAY SOON Tom Jones
22862 THAT WONDERFUL SOUND Tom Jones
57170 THE GREEN GREEN GRASS OF HOME TOM JONES
57294 The Wonderful Sound Tom Jones
23819 TILL Tom Jones
51759 What's New Pussycat Tom Jones
52862 With These Hands Tom Jones
57715 Without Love Tom Jones
57836 You're My World Tom Jones
	*/

	for (int n = 1; n < langCount.length; n++) {
	    if (langCount[n] != 0) {
		System.out.println("Count: " + langCount[n] + " of lang " + n);
	    }
	}

	// Check Russian, etc
	System.out.println("Russian " + '\u0411');
	System.out.println("Korean " + '\u0411');
	System.exit(0);
    }
}