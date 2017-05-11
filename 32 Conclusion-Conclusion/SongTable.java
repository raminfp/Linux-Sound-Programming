
package newmarch.songtable;

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
import java.net.URL;
import java.net.URLConnection;
import java.net.HttpURLConnection;

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

		SongInformation info = new SongInformation(file.toString(),
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
    private static final String SONG_STORE_DEFAULT = "http://192.168.1.101/KARAOKE/SongStore";
    private static final String SONG_STORE_ROOT = "http://192.168.1.101/KARAOKE/";

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
	    if (args[0].startsWith("-s")) {
		loadTableFromStore(SONG_STORE_ROOT + args[0].substring(2));
	    } else {	    
		System.err.println("Loading from " + args[0]);
		loadTableFromSource(args[0]);
		saveTableToStore(args[1]);
	    }
	} else {
	    loadTableFromStore(SONG_STORE_DEFAULT);
	}
    }

    public boolean loadTableFromStore(String urlStr) {
	try {
	    /*
	    String userHome = System.getProperty("user.home");
	    Path storePath = FileSystems.getDefault().getPath(userHome, 
							      ".karaoke",
							      "SongStore");
	    
	    File storeFile = storePath.toFile();
	    */
	   
	    URL url = new URL(urlStr);
	    
	    BufferedInputStream reader = new BufferedInputStream(url
								 .openConnection().getInputStream());
	    ObjectInputStream is = new ObjectInputStream(reader);
	    songs = (Vector<SongInformation>) is.readObject();
	    reader.close();
	} catch(Exception e) {
	    System.err.println("Can't load store file " + e.toString());
	    return false;
	}
	return true;
    }

    public void saveTableToStore(String urlStr) {
	try {
	    URL url = new URL(urlStr);
	    URLConnection connection = url.openConnection();
	    HttpURLConnection httpConnection = (HttpURLConnection) connection;
	    httpConnection.setDoOutput(true);
	    httpConnection.setRequestMethod("PUT");

	    BufferedOutputStream writer = new BufferedOutputStream(connection.getOutputStream());
	    ObjectOutputStream os = new ObjectOutputStream(writer);
	    os.writeObject(songs);
	    os.close();
	    writer.close();
	    int responseCode = httpConnection.getResponseCode();
	    String responseMsg = httpConnection.getResponseMessage();
	    System.out.println("Saved table to " + urlStr + " with code " + responseCode +
			       responseMsg + " size " + songs.size());
	   
	} catch(Exception e) {
	    System.err.println("Can't save store file " + e.toString());
	}
    }

    private void loadTableFromSource(String source) throws java.io.IOException, 
			      java.io.FileNotFoundException {

	Path path = FileSystems.getDefault().getPath(source);
	File f = path.toFile();
	if (f.isDirectory()) {
	    Visitor pf = new Visitor(songs);
	    Files.walkFileTree(path, pf);
	} else if (f.isFile()) {
	    loadTableFromFile(path);
	}
    }

    private void loadTableFromFile(Path path) throws java.io.IOException {
	BufferedReader in = new BufferedReader(new FileReader(path.toFile()));
	String fname;

	while ((fname = in.readLine()) != null) {
	    // System.out.println(fname);
	    if (fname.endsWith(".zip") || 
		fname.endsWith(".mp3") || 
		fname.endsWith(".kar")) {
		// lose extension
		String root = fname.substring(0, fname.length()-4);
		// lose /.../
		root = root.substring(root.lastIndexOf('/')+1);
		//System.err.println(" root " + root);
		String parts[] = root.split(" - ", 3);
		if (parts.length != 3)
		    continue;

		String index = parts[0];
		String artist = parts[1];
		String title = parts[2];

		SongInformation info = new SongInformation(fname,
							   index,
							   title,
							   artist);
		songs.add(info);
	    }
	}
    }

    public java.util.Iterator<SongInformation> iterator() {
	return songs.iterator();
    }

    public SongInformation getSongAt(int index) {
	if (index < 0 || index >= songs .size()) {
	    return null;
	}
	return songs.elementAt(index);
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
