package newmarch.songtable;

import java.awt.*;
import java.awt.event.*;
import javax.swing.*;
import java.util.Vector;
import java.nio.file.*;
import java.io.*;
import java.net.URL;

public class AllFavourites extends JTabbedPane {
    public static String FAVOURITES_DIR = "http://192.168.1.101:8000/KARAOKE/favourites/";

    private SongTableSwing songTable;
    public Vector<FavouritesInfo> favourites = new Vector<FavouritesInfo>();

    public AllFavourites(SongTableSwing songTable) {
	this.songTable = songTable;

	loadFavourites();

	NewPanel newP = new NewPanel(this);
	addTab("NEW", null, newP);
    }

    private void loadFavourites() {
	//String userHome = System.getProperty("user.home");
	/*
	Path favouritesPath = FileSystems.getDefault().getPath(userHome, 
							    ".karaoke",
							    "favourites");
	*/

	try {
	    URL url = new URL(FAVOURITES_DIR);
	    InputStreamReader reader = new InputStreamReader(url
							     .openConnection().getInputStream());
	    BufferedReader in = new BufferedReader(reader);
	    String line = in.readLine();
	    while (line != null) {
		if (line.startsWith(".")) {
		    // ignore .htacess etc
		    continue;
		}
		favourites.add(new FavouritesInfo(null, line, null));
		line = in.readLine();
	    }
	    in.close();
	    reader.close();

	    for (FavouritesInfo f: favourites) {
		// TODO checkout the Songtable constructors - messy 
		f.songTable = new SongTable(new Vector<SongInformation>());
		f.songTable.loadTableFromStore(FAVOURITES_DIR +
					   f.owner);

		Favourites fav = new Favourites(songTable, 
						f.songTable, 
						f.owner);
		addTab(f.owner, null, fav, f.owner);
	    }
	} catch(Exception e) {
	    System.out.println(e.toString());
	}

	/*
	Path favouritesPath = FileSystems.getDefault().getPath(FAVOURITES_DIR);
	try {
	    DirectoryStream<Path> stream = 
		Files.newDirectoryStream(favouritesPath);
	    for (Path entry: stream) {
		int nelmts = entry.getNameCount();
		Path last = entry.subpath(nelmts-1, nelmts);
		if (last.toString().startsWith(".")) {
		    // ignore .htaccess etc
		    continue;
		}
		System.err.println("Favourite: " + last.toString());
		File storeFile = entry.toFile();
		
		FileInputStream in = new FileInputStream(storeFile); 
		ObjectInputStream is = new ObjectInputStream(in);
		Vector<SongInformation> favouriteSongs = 
		    (Vector<SongInformation>) is.readObject();
		in.close();
		for (SongInformation s: favouriteSongs) {
		    System.err.println("Fav: " + s.toString());
		}

		SongTable favouriteSongsTable = new SongTable(favouriteSongs);
		Favourites f = new Favourites(songTable, 
					      favouriteSongsTable, 
					      last.toString());
		addTab(last.toString(), null, f, last.toString());
		System.err.println("Loaded favs " + last.toString());
	    }
	} catch(Exception e) {
	    System.err.println(e.toString());
	}
	*/
    }

    class NewPanel extends JPanel {
	private JTabbedPane pane;

	public NewPanel(final JTabbedPane pane) {
	    this.pane = pane;

	    setLayout(new FlowLayout());
	    JLabel nameLabel = new JLabel("Name of new person");
	    final JTextField nameField = new JTextField(10);
	    add(nameLabel);
	    add(nameField);

	    nameField.addActionListener(new ActionListener(){
		    public void actionPerformed(ActionEvent e){
			String name = nameField.getText();

			SongTable songs = new SongTable(new Vector<SongInformation>());
			Favourites favs = new Favourites(songTable, songs, name);
			
			pane.addTab(name, null, favs);
		    }});

	}
    }

    class FavouritesInfo {
	public SongTable songTable;
	public String owner;
	public Image image;
	
	public FavouritesInfo(SongTable songTable,
			      String owner,
			      Image image) {
	    this.songTable = songTable;
	    this.owner = owner;
	    this.image = image;
	}
    }

}
