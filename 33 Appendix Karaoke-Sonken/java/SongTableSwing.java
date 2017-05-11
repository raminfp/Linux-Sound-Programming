
import java.awt.*;
import java.awt.event.*;
import javax.swing.event.*;
import javax.swing.*;
import javax.swing.SwingUtilities;
import java.util.regex.*;
import java.io.*;

public class SongTableSwing extends JPanel {
   private DefaultListModel model = new DefaultListModel();
    private JList list;
    private static SongTable allSongs;

    private JTextField numberField;
    private JTextField langField;
    private JTextField titleField;
    private JTextField artistField;

    // This font displays Asian and European characters.
    // It should be in your distro.
    // Fonts displaying all Unicode are zysong.ttf and Cyberbit.ttf
    // See http://unicode.org/resources/fonts.html
    private Font font = new Font("WenQuanYi Zen Hei", Font.PLAIN, 16);
    // font = new Font("Bitstream Cyberbit", Font.PLAIN, 16);
    
    private int findIndex = -1;

    /**
     * Describe <code>main</code> method here.
     *
     * @param args a <code>String</code> value
     */
    public static final void main(final String[] args) {
	allSongs = null;
	try {
	    allSongs = new SongTable();
	} catch(Exception e) {
	    System.err.println(e.toString());
	    System.exit(1);
	}

	JFrame frame = new JFrame();
	frame.setTitle("Song Table");
	frame.setSize(1000, 800);
	frame.setDefaultCloseOperation(JFrame.EXIT_ON_CLOSE);
	
	SongTableSwing panel = new SongTableSwing(allSongs);
	frame.getContentPane().add(panel);

	frame.setVisible(true);
    }

    public SongTableSwing(SongTable songs) {

	if (font == null) {
	    System.err.println("Can't fnd font");
	}
		
	int n = 0;
	java.util.Iterator<SongInformation> iter = songs.iterator();
	while(iter.hasNext()) {
	    model.add(n++, iter.next());
	    // model.add(n++, iter.next().toString());
	}

	BorderLayout mgr = new BorderLayout();
 
	list = new JList(model);
	// list = new JList(songs);
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

	JLabel numberLabel = new JLabel("Number");
	numberField = new JTextField(5);

	JLabel langLabel = new JLabel("Language");
	langField = new JTextField(8);

	JLabel titleLabel = new JLabel("Title");
	titleField = new JTextField(20);
	titleField.setFont(font);

	JLabel artistLabel = new JLabel("Artist");
	artistField = new JTextField(10);
	artistField.setFont(font);

	searchPanel.add(numberLabel);
	searchPanel.add(numberField);
	// searchPanel.add(langLabel);
	// searchPanel.add(langField);
	searchPanel.add(titleLabel);
	searchPanel.add(titleField);
	searchPanel.add(artistLabel);
	searchPanel.add(artistField);

	titleField.getDocument().addDocumentListener(new DocumentListener() {
		public void changedUpdate(DocumentEvent e) {
		    // rest find to -1 to restart any find searches
		    findIndex = -1;
		    // System.out.println("reset find index");
		}
		public void insertUpdate(DocumentEvent e) {
		    findIndex = -1;
		    // System.out.println("reset insert find index");
		}
		public void removeUpdate(DocumentEvent e) {
		    findIndex = -1;
		    // System.out.println("reset remove find index");
		}
	    }
	    );
	artistField.getDocument().addDocumentListener(new DocumentListener() {
		public void changedUpdate(DocumentEvent e) {
		    // rest find to -1 to restart any find searches
		    findIndex = -1;
		    // System.out.println("reset insert find index");
		}
		public void insertUpdate(DocumentEvent e) {
		    findIndex = -1;
		    // System.out.println("reset insert find index");
		}
		public void removeUpdate(DocumentEvent e) {
		    findIndex = -1;
		    // System.out.println("reset insert find index");
		}
	    }
	    );

	titleField.addActionListener(new ActionListener(){
                public void actionPerformed(ActionEvent e){
		    filterSongs();
                }});
	artistField.addActionListener(new ActionListener(){
                public void actionPerformed(ActionEvent e){
		    filterSongs();
                }});
	numberField.addActionListener(new ActionListener(){
                public void actionPerformed(ActionEvent e){
		    filterSongs();
                }});

	JPanel buttonPanel = new JPanel();
	bottomPanel.add(buttonPanel);
	buttonPanel.setLayout(new FlowLayout());

	JButton find = new JButton("Find");
	JButton filter = new JButton("Filter");
	JButton reset = new JButton("Reset");
	JButton play = new JButton("Play");
	buttonPanel.add(find);
	buttonPanel.add(filter);
	buttonPanel.add(reset);
	buttonPanel.add(play);

	find.addActionListener(new ActionListener() {
		public void actionPerformed(ActionEvent e) {
		    findSong();
		}
	    });

	filter.addActionListener(new ActionListener() {
		public void actionPerformed(ActionEvent e) {
		    filterSongs();
		}
	    });

	reset.addActionListener(new ActionListener() {
		public void actionPerformed(ActionEvent e) {
		    resetSongs();
		}
	    });

	play.addActionListener(new ActionListener() {
		public void actionPerformed(ActionEvent e) {
		    playSong();
		}
	    });
 
     }

    public void findSong() {
	String number = numberField.getText();
	String language = langField.getText();
	String title = titleField.getText();
	String artist = artistField.getText();

	if (number.length() != 0) {
	    try {

		long num = Integer.parseInt(number) - 1;
		for (int n = 0; n < model.getSize(); n++) {
		    SongInformation info = (SongInformation) model.getElementAt(n);
		    if (info.number == num) {
			list.setSelectedIndex(n);
			list.ensureIndexIsVisible(n);
			return;
		    }
		}
	    } catch(Exception e) {
		System.err.println("Not a number");
		numberField.setText("");
	    }

	    return;
	}

	/*
	System.out.println("Title " + title + title.length() + 
			   "artist " + artist + artist.length() +
			   " find start " + findIndex +
			   " model size " + model.getSize());
	if (title.length() == 0 && artist.length() == 0) {
	    System.err.println("no search terms");
	    return;
	}
	*/

	//System.out.println("Search " + searchStr + " from index " + findIndex);
	for (int n = findIndex + 1; n < model.getSize(); n++) {
	    SongInformation info = (SongInformation) model.getElementAt(n);
	    //System.out.println(info.toString());

	    if ((title.length() != 0) && (artist.length() != 0)) {
		if (info.titleMatch(title) && info.artistMatch(artist)) {
		    // System.out.println("Found " + info.toString());
			findIndex = n;
			list.setSelectedIndex(n);
			list.ensureIndexIsVisible(n);
			break;
		}
	    } else {
		if ((title.length() != 0) && info.titleMatch(title)) {
		    // System.out.println("Found " + info.toString());
		    findIndex = n;
		    list.setSelectedIndex(n);
		    list.ensureIndexIsVisible(n);
		    break;
		} else if ((artist.length() != 0) && info.artistMatch(artist)) {
		    // System.out.println("Found " + info.toString());
		    findIndex = n;
		    list.setSelectedIndex(n);
		    list.ensureIndexIsVisible(n);
		    break;

		}
	    }

	}
    }

    public void filterSongs() {
	String title = titleField.getText();
	String artist = artistField.getText();
	String number = numberField.getText();
	SongTable filteredSongs = allSongs;

	if (allSongs == null) {
	    // System.err.println("Songs is null");
	    return;
	}

	if (title.length() != 0) {
	    filteredSongs = filteredSongs.titleMatches(title);
	}
	if (artist.length() != 0) {
	    filteredSongs = filteredSongs.artistMatches(artist);
	}
	if (number.length() != 0) {
	    filteredSongs = filteredSongs.numberMatches(number);
	}

	model.clear();
	int n = 0;
	java.util.Iterator<SongInformation> iter = filteredSongs.iterator();
	while(iter.hasNext()) {
	    model.add(n++, iter.next());
	}
    }

    public void resetSongs() {
	artistField.setText("");
	titleField.setText("");
	numberField.setText("");
	model.clear();
	int n = 0;
	java.util.Iterator<SongInformation> iter = allSongs.iterator();
	while(iter.hasNext()) {
	    model.add(n++, iter.next());
	}
    }
    /**
     * "play" a song by printing its id to standard out.
     * Can be used in a pipeline this way
     */
    public void playSong() {
	SongInformation song = (SongInformation) list.getSelectedValue();
	if (song == null) {
	    return;
	}
	long number = song.number + 1;
	System.out.println("" + number);
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