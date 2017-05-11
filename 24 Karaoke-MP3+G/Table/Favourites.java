
import java.awt.*;
import java.awt.event.*;
import javax.swing.event.*;
import javax.swing.*;
import javax.swing.SwingUtilities;
import java.util.regex.*;
import java.io.*;
import java.nio.file.FileSystems;
import java.nio.file.*;

public class Favourites extends JPanel {
    private DefaultListModel model = new DefaultListModel();
    private JList list;

    // whose favoutites these are
    private String user;

    // songs in this favourites list
    private final SongTable favouriteSongs;

    // pointer back to main song table list
    private final SongTableSwing songTable;

    // This font displays Asian and European characters.
    // It should be in your distro.
    // Fonts displaying all Unicode are zysong.ttf and Cyberbit.ttf
    // See http://unicode.org/resources/fonts.html
    private Font font = new Font("WenQuanYi Zen Hei", Font.PLAIN, 16);
    
    private int findIndex = -1;

    public Favourites(final SongTableSwing songTable, 
		      final SongTable favouriteSongs, 
		      String user) {
	this.songTable = songTable;
	this.favouriteSongs = favouriteSongs;
	this.user = user;

	if (font == null) {
	    System.err.println("Can't find font");
	}
		
	int n = 0;
	java.util.Iterator<SongInformation> iter = favouriteSongs.iterator();
	while(iter.hasNext()) {
	    model.add(n++, iter.next());
	}

	BorderLayout mgr = new BorderLayout();
 
	list = new JList(model);
	list.setFont(font);
	JScrollPane scrollPane = new JScrollPane(list);

	setLayout(mgr);
	add(scrollPane, BorderLayout.CENTER);

	JPanel bottomPanel = new JPanel();
	bottomPanel.setLayout(new GridLayout(2, 1));
	add(bottomPanel, BorderLayout.SOUTH);

	JPanel searchPanel = new JPanel();
	bottomPanel.add(searchPanel);
	searchPanel.setLayout(new FlowLayout());

	JPanel buttonPanel = new JPanel();
	bottomPanel.add(buttonPanel);
	buttonPanel.setLayout(new FlowLayout());

	JButton addSong = new JButton("Add song to list");
	JButton deleteSong = new JButton("Delete song from list");
	JButton play = new JButton("Play");

	buttonPanel.add(addSong);
	buttonPanel.add(deleteSong);
	buttonPanel.add(play);

	play.addActionListener(new ActionListener() {
		public void actionPerformed(ActionEvent e) {
		    playSong();
		}
	    });

	deleteSong.addActionListener(new ActionListener() {
		public void actionPerformed(ActionEvent e) {
		    SongInformation song = (SongInformation) list.getSelectedValue();
		    model.removeElement(song);
		    favouriteSongs.songs.remove(song);
		    saveToStore();
		}
	    });

	addSong.addActionListener(new ActionListener() {
		public void actionPerformed(ActionEvent e) {
		    SongInformation song = songTable.getSelection();
		    model.addElement(song);
		    favouriteSongs.songs.add(song);
		    saveToStore();
		}
	    });
     }

    private void saveToStore() {
	try {
	    /*
	    String userHome = System.getProperty("user.home");
	    Path storePath = FileSystems.getDefault().getPath(userHome, 
							      ".karaoke",
							      "favourites",
							      user);
	    File storeFile = storePath.toFile();
	    */
	    File storeFile = new File("/server/KARAOKE/favourites/" + user);
	    FileOutputStream out = new FileOutputStream(storeFile); 
	    ObjectOutputStream os = new ObjectOutputStream(out);
	    os.writeObject(favouriteSongs.songs); 
	    os.flush(); 
	    out.close();
	} catch(Exception e) {
	    System.err.println("Can't save favourites file " + e.toString());
	}
    }


    /**
     * "play" a song by printing its file path to standard out.
     * Can be used in a pipeline this way
     */
    public void playSong() {
	SongInformation song = (SongInformation) list.getSelectedValue();
	if (song == null) {
	    return;
	}
	System.out.println(song.path.toString());
    }


    class SongInformationRenderer extends JLabel implements ListCellRenderer {

	public Component getListCellRendererComponent(
						      JList list,
						      Object value,
						      int index,
						      boolean isSelected,
						      boolean cellHasFocus) {
	    setText(value.toString());
	    return this;
	}
    }
}
