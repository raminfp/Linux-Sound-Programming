
import java.nio.file.Path;
import java.io.Serializable;

public class SongInformation implements Serializable {


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



    public SongInformation(Path path,
			   String index,
			   String title,
			   String artist) {
	this.path = path.toString();
	this.index = index;
	this.title = title;
	this.artist = artist;
    }

    public String toString() {
	return "(" + index + ") " + artist + ": " + title;
    }

    public boolean titleMatch(String pattern) {
	return title.matches("(?i).*" + pattern + ".*");
    }

    public boolean artistMatch(String pattern) {
	return artist.matches("(?i).*" + pattern + ".*");
    }

    public boolean numberMatch(String pattern) {
	return index.equals(pattern);
    }
}
