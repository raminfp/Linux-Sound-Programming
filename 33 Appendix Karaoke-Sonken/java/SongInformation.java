


public class SongInformation {


    // Public fields of each song record
    /**
     *  Song number in the file, one less than in songbook
     */
    public long number;

    /**
     * song title in Unicode
     */
    public String title;

    /**
     * artist in Unicode
     */
    public String artist;

    /**
     * integer value of language code
     */
    public int language;

    public static final int  KOREAN = 0;
    public static final int  CHINESE1 = 1;
    public static final int  CHINESE2 = 2;
    public static final int  TAIWANESE3 = 3 ;
    public static final int  JAPANESE = 4;
    public static final int  RUSSIAN = 5;
    public static final int  THAI = 6;
    public static final int  TAIWANESE7 = 7;
    public static final int  CHINESE8 = 8;
    public static final int  CANTONESE = 9;
    public static final int  ENGLISH = 0x12;
    public static final int  VIETNAMESE = 0x13;
    public static final int  PHILIPPINE = 0x14;
    public static final int  TURKEY = 0x15;
    public static final int  SPANISH = 0x16;
    public static final int  INDONESIAN = 0x17;
    public static final int  MALAYSIAN = 0x18;
    public static final int  PORTUGUESE = 0x19;
    public static final int  FRENCH = 0x20;
    public static final int  INDIAN = 0x21;
    public static final int  BRASIL = 0x22;
    public static final int  CHINESE131 = 131;
    public static final int  ENGLISH146 = 146;
    public static final int  PHILIPPINE148 = 148;

    public SongInformation(long number,
			   String title,
			   String artist,
			   int language) {
	this.number = number;
	this.title = title;
	this.artist = artist;
	this.language = language;
    }

    public String toString() {
	return "" + (number+1) + " (" + language + ") \"" + title + "\" " + artist;
    }

    public boolean titleMatch(String pattern) {
	// System.out.println("Pattern: " + pattern);
	return title.matches("(?i).*" + pattern + ".*");
    }

    public boolean artistMatch(String pattern) {
	return artist.matches("(?i).*" + pattern + ".*");
    }

    public boolean numberMatch(String pattern) {
	Long n;
	try {
	    n = Long.parseLong(pattern) - 1;
	    //System.out.println("Long is " + n);
	} catch(Exception e) {
	    //System.out.println(e.toString());
	    return false;
	}
	return number == n;
    }


    public boolean languageMatch(int lang) {
	return language == lang;
    }
}
