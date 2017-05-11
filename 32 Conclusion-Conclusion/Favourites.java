
package newmarch.songtable;

import java.awt.*;
import java.awt.event.*;
import javax.swing.event.*;
import javax.swing.*;
import javax.swing.table.*;
import javax.swing.SwingUtilities;
import java.util.regex.*;
import java.io.*;
import java.nio.file.FileSystems;
import java.nio.file.*;
import java.net.Socket;
import java.net.InetAddress;
import java.util.Vector;

public class Favourites extends JPanel {
    private DefaultListModel model = new DefaultListModel();
    private JList list;

    private JTable table;
    private DefaultTableModel tmodel = new DefaultTableModel();

    // whose favorites these are
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


	System.out.println("Favourites for user: " + user);
	
	tmodel.addColumn("ID");
	tmodel.addColumn("Artist");
	tmodel.addColumn("Title");
	tmodel.addColumn("Path");
	
	int n = 0;
	java.util.Iterator<SongInformation> iter = favouriteSongs.iterator();
	while(iter.hasNext()) {
	    SongInformation info = iter.next();
	    if (info == null)
		continue;
	    /*
	    System.out.println("UID for SongInformation " + 
			       ObjectStreamClass.lookup(info.getClass()).getSerialVersionUID());
	    */
	    model.add(n++, info);

	    tmodel.addRow(info.toVector());
	}



	BorderLayout mgr = new BorderLayout();
 
	list = new JList(model);
	list.setFont(font);

	table = new JTable(tmodel);
	table.setRowSorter(new TableRowSorter(tmodel));
	// hide column 3
	table.removeColumn(table.getColumnModel().getColumn(3));
	table.setFillsViewportHeight(true);
	table.setFont(font);
	table.getColumnModel().getColumn(0).setPreferredWidth(100);
	table.getColumnModel().getColumn(0).setMaxWidth(100);
	table.getColumnModel().getColumn(1).setPreferredWidth(200);
	table.getColumnModel().getColumn(1).setMaxWidth(200);
	table.setRowHeight(24);

	//JScrollPane scrollPane = new JScrollPane(list);
	JScrollPane scrollPane = new JScrollPane(table);

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
		    /*
		    SongInformation song = (SongInformation) list.getSelectedValue();
		    model.removeElement(song);
		    favouriteSongs.songs.remove(song);
		    */

		    int realIndex = table.convertRowIndexToModel(table.getSelectedRow());
		    tmodel.removeRow(realIndex);
		    favouriteSongs.songs.removeElementAt(realIndex);
		    saveToStore();
		}
	    });

	addSong.addActionListener(new ActionListener() {
		public void actionPerformed(ActionEvent e) {
		    SongInformation song = songTable.getSelection();
		    //model.addElement(song);
		    tmodel.addRow(song.toVector());
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

	    //favouriteSongs.saveTableToStore("/server/KARAOKE/favourites/" + user);
	    favouriteSongs.saveTableToStore(AllFavourites.FAVOURITES_DIR + user);

	    /*
	    File storeFile = new File("/server/KARAOKE/favourites/" + user);
	    FileOutputStream out = new FileOutputStream(storeFile); 
	    ObjectOutputStream os = new ObjectOutputStream(out);
	    os.writeObject(favouriteSongs.songs); 
	    os.flush(); 
	    out.close();
	    */
	} catch(Exception e) {
	    System.err.println("Can't save favourites file " + e.toString());
	}
    }


    /**
     * "play" a song by printing its file path to standard out.
     * Can be used in a pipeline this way
     */
    public void playSong() {
	/*
	SongInformation song = (SongInformation) list.getSelectedValue();
	if (song == null) {
	    return;
	}
	System.out.println(song.path.toString());
	*/
	String path = table.getModel().getValueAt(
			      table.convertRowIndexToModel(
							   table.getSelectedRow()), 3).toString();
	System.out.println(path);

	String SERVERIP = "192.168.1.110"; 
	int SERVERPORT = 13000;
	PrintWriter out;
	
	try {
	    InetAddress serverAddr = InetAddress.getByName(SERVERIP);
	    Socket socket = new Socket(serverAddr, SERVERPORT);
				 
	    //send the message to the server
	    out = new PrintWriter(
				  new BufferedWriter(
						     new OutputStreamWriter(socket.getOutputStream())), 
				  true);
	  
	    // Avoid println - on Windows it is \r\n
	    //out.print(song.path + "\n");
	    out.print(path + "\n");
	    out.flush();
	    socket.close();
				
	} catch (Exception e) {
	    System.err.println(e.toString());
	}

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
