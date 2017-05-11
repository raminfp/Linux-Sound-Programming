
import java.util.Vector;
import java.io.FileInputStream;
import java.io.*;
import java.nio.charset.Charset;
import java.nio.file.Files;
import java.nio.file.Path;
import java.nio.file.Paths;
import java.nio.file.SimpleFileVisitor;
import java.nio.file.FileVisitResult;
import java.nio.file.FileSystems;
import java.nio.file.attribute.*;


class Visitor
    extends SimpleFileVisitor<Path> {

    private Vector<SongInformation> songs;

    public Visitor(Vector<SongInformation> songs) {
	this.songs = songs;
    }

    @Override
    public FileVisitResult visitFile(Path file,
                                   BasicFileAttributes attr) {
	if (attr.isRegularFile()) {
	    String fname = file.getFileName().toString();
	    //System.out.println("Regular file " + fname);
	    if (fname.endsWith(".zip") || 
		fname.endsWith(".mp3") || 
		fname.endsWith(".kar")) {
		String root = fname.substring(0, fname.length()-4);
		//System.err.println(" root " + root);
		String parts[] = root.split(" - ", 3);
		if (parts.length != 3)
		    return java.nio.file.FileVisitResult.CONTINUE;

		String index = parts[0];
		String artist = parts[1];
		String title = parts[2];

		SongInformation info = new SongInformation(file,
							   index,
							   title,
							   artist);
		songs.add(info);
	    }
	}

        return java.nio.file.FileVisitResult.CONTINUE;
    }
}

public class SongTable {

    private static final String SONG_INFO_ROOT = "/server/KARAOKE/KARAOKE/";

    private static Vector<SongInformation> allSongs;

    public Vector<SongInformation> songs = 
	new Vector<SongInformation>  ();

    public static long[] langCount = new long[0x23];

    public SongTable(Vector<SongInformation> songs) {
	this.songs = songs;
    }

    public SongTable(String[] args) throws java.io.IOException, 
					   java.io.FileNotFoundException {
	if (args.length >= 1) {
	    System.err.println("Loading from " + args[0]);
	    loadTableFromSource(args[0]);
	    saveTableToStore();
	} else {
	    loadTableFromStore();
	}
    }

    private boolean loadTableFromStore() {
	try {
	    /*
	    String userHome = System.getProperty("user.home");
	    Path storePath = FileSystems.getDefault().getPath(userHome, 
							      ".karaoke",
							      "SongStore");
	    
	    File storeFile = storePath.toFile();
	    */
	    File storeFile = new File("/server/KARAOKE/SongStore"); 
	    
	    FileInputStream in = new FileInputStream(storeFile); 
	    ObjectInputStream is = new ObjectInputStream(in);
	    songs = (Vector<SongInformation>) is.readObject();
	    in.close();
	} catch(Exception e) {
	    System.err.println("Can't load store file " + e.toString());
	    return false;
	}
	return true;
    }

    private void saveTableToStore() {
	try {
	    /*
	    String userHome = System.getProperty("user.home");
	    Path storePath = FileSystems.getDefault().getPath(userHome, 
							      ".karaoke",
							      "SongStore");
	    File storeFile = storePath.toFile();
	    */
	    File storeFile = new File("/server/KARAOKE/SongStore");
	    FileOutputStream out = new FileOutputStream(storeFile); 
	    ObjectOutputStream os = new ObjectOutputStream(out);
	    os.writeObject(songs); 
	    os.flush(); 
	    out.close();
	} catch(Exception e) {
	    System.err.println("Can't save store file " + e.toString());
	}
    }

    private void loadTableFromSource(String dir) throws java.io.IOException, 
			      java.io.FileNotFoundException {

	Path startingDir = FileSystems.getDefault().getPath(dir);
	Visitor pf = new Visitor(songs);
	Files.walkFileTree(startingDir, pf);
    }

    public java.util.Iterator<SongInformation> iterator() {
	return songs.iterator();
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
	    songs = new SongTable(new String[] {SONG_INFO_ROOT});
	} catch(Exception e) {
	    System.err.println(e.toString());
	    System.exit(1);
	}

	System.out.println(songs.artistMatches("Tom Jones").toString());

	System.exit(0);
    }
}
