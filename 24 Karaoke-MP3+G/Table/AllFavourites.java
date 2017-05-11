import java.awt.*;
import java.awt.event.*;
import javax.swing.*;
import java.util.Vector;
import java.nio.file.*;
import java.io.*;

public class AllFavourites extends JTabbedPane {
    private SongTableSwing songTable;

    public AllFavourites(SongTableSwing songTable) {
	this.songTable = songTable;

	loadFavourites();

	NewPanel newP = new NewPanel(this);
	addTab("NEW", null, newP);
    }

    private void loadFavourites() {
	String userHome = System.getProperty("user.home");
	/*
	Path favouritesPath = FileSystems.getDefault().getPath(userHome, 
							    ".karaoke",
							    "favourites");
	*/
	Path favouritesPath = FileSystems.getDefault().getPath("/server/KARAOKE/favourites");
	try {
	    DirectoryStream<Path> stream = 
		Files.newDirectoryStream(favouritesPath);
	    for (Path entry: stream) {
		int nelmts = entry.getNameCount();
		Path last = entry.subpath(nelmts-1, nelmts);
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
}
