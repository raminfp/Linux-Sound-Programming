
package newmarch.songtable;

import java.nio.file.Path;
import java.io.Serializable;
import java.util.Vector;

public class SongInformation implements Serializable {

    private static final long serialVersionUID = -7465256749074820597L;

    // Public fields of each song record

    public String path;

    public String index;

    /**
     * song title in Unicode
     */
    public String title;

    /**
     * artist in Unicode
     */
    public String artist;



    public SongInformation(String path,
			   String index,
			   String title,
			   String artist) {
	this.path = path;
	this.index = index;
	this.title = title;
	this.artist = artist;
    }

    public String toString() {
	return "(" + index + ") " + artist + ": " + title;
    }

    public Vector<String> toVector() {
	Vector<String> v = new Vector<String>();
	v.add(index);
	v.add(artist);
	v.add(title);
	// will be hidden field
	v.add(path);

	return v;
    }

    public boolean titleMatch(String pattern) {
	return title.matches("(?i).*" + pattern + ".*");
    }

    public boolean artistMatch(String pattern) {
	return artist.matches("(?i).*" + pattern + ".*");
    }

    public boolean numberMatch(String pattern) {
	if (pattern.length() > 0 && Character.isDigit(pattern.charAt(0))) {
	    // user typed a number only, assume it is
	    // from the Karaoke songs
	    pattern = "SK-" + pattern;
	}
	return index.equalsIgnoreCase(pattern);
    }
}
