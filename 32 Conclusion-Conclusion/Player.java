
import java.awt.*;
import java.awt.event.*;
import javax.swing.event.*;
import javax.swing.*;
import java.io.*;
import java.net.*;


public class Player extends JFrame {

    private DefaultListModel model = new DefaultListModel();
    private JList queue;
    private JLabel playingLabel;

    public boolean isPlaying = false;
    private Process songPlayingProcess = null;

    // we have to keep track of the tempo rate, vlc won't tell us
    private double rate = 1.0;


    public static void main(String[] args) {
	new Player();
    }

    public Player() {
	Container contentPane = getContentPane();

	setTitle("Song Queue");
        Dimension screenSize = Toolkit.getDefaultToolkit().getScreenSize();
        setSize(400, 400);
        setDefaultCloseOperation(JFrame.EXIT_ON_CLOSE);

	contentPane.setLayout(new BorderLayout());

	queue = new JList(model);
	contentPane.add(queue, BorderLayout.CENTER);

        JPanel bottomPanel = new JPanel();
        bottomPanel.setLayout(new GridLayout(2, 1));
        add(bottomPanel, BorderLayout.SOUTH);

	playingLabel = new JLabel(" ");
	bottomPanel.add(playingLabel);

        JPanel controlPanel = new JPanel();
        bottomPanel.add(controlPanel);
        controlPanel.setLayout(new FlowLayout());

	JButton stopBtn = new JButton("Stop");
	JButton fasterBtn = new JButton("Faster");
	JButton slowerBtn = new JButton("Slower");

	controlPanel.add(stopBtn);
	controlPanel.add(fasterBtn);
	controlPanel.add(slowerBtn);

	stopBtn.addActionListener(new ActionListener(){
                public void actionPerformed(ActionEvent evt){
		    if (songPlayingProcess != null) {
			PrintStream writer = new PrintStream(songPlayingProcess.getOutputStream());
			writer.println("quit");
			writer.flush();
		    }
		}
	    });


	fasterBtn.addActionListener(new ActionListener(){
                public void actionPerformed(ActionEvent evt){
		    if (songPlayingProcess != null) {
			PrintStream writer = new PrintStream(songPlayingProcess.getOutputStream());
			rate += 0.03;
			writer.println("rate " + rate);
			writer.flush();
		    }
		}
	    });

	slowerBtn.addActionListener(new ActionListener(){
                public void actionPerformed(ActionEvent evt){
		    if (songPlayingProcess != null) {
			PrintStream writer = new PrintStream(songPlayingProcess.getOutputStream());
			rate -= 0.03;
			writer.println("rate " + rate);
			writer.flush();
		    }
		}
	    });

	/*
	model.addListDataListener(new ListDataListener() {


	    });
	*/
	setVisible(true);

	new Thread(new TCPListener()).start();
    }

    public void playSong(SongInformation info) {
	isPlaying = true;
	try {
	    if (info.path.endsWith(".zip")) {
		songPlayingProcess = Runtime.getRuntime().exec(new String[] {"bash", "playZip", info.path});
	    }  else if (info.path.endsWith(".kar")) {
		songPlayingProcess = Runtime.getRuntime().exec(new String[] {"bash", "playKar", info.path});
	    } else {
		isPlaying = false;
		return;
	    }
	    playingLabel.setText("Now playing: " + info.toString());
	    new Thread(new WaitForSongToEnd()).start();
	} catch(IOException e) {
	    System.err.println(e.toString());
	    isPlaying = false;
	}
    }

    public void playNextSong() {
	if (model.isEmpty()) {
	    return;
	}
	SongInformation info = (SongInformation) model.remove(0);
	playSong(info);
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

    class WaitForSongToEnd implements Runnable {

	public void run() {
	    try {
		songPlayingProcess.waitFor();
	    } catch(Exception e) {
		System.err.println(e.toString());
	    }
	    Player.this.isPlaying = false;
	    rate = 1.0;
	    songPlayingProcess = null;
	    System.out.println("Song finished!");
	    Player.this.playingLabel.setText(" ");
	    Player.this.playNextSong();
	}
    }

    class TCPListener implements Runnable {
	public int PORT = 13000;

	public void run() {
	    try {
		System.out.println("Listening...");
		ServerSocket s = new ServerSocket(PORT);
		while (true) {
		    Socket incoming = s.accept();
		    handleSocket(incoming);
		    incoming.close();
		}
	    } catch(IOException e) {
		System.err.println(e.toString());
	    }
	}

	public void handleSocket(Socket incoming) {
	    try {
		BufferedReader reader =
		    new BufferedReader(new InputStreamReader(
							 incoming.getInputStream()));

		String fname = reader.readLine();
		if (fname == null) {
		    return;
		}
		System.out.println("Echo: " + fname);
		
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
			return;
		    
		    String index = parts[0];
		    String artist = parts[1];
		    String title = parts[2];
		    
		    SongInformation info = new SongInformation(fname,
							       index,
							       title,
							       artist);
		    if (isPlaying) {
			model.addElement(info);
		    } else {
			Player.this.playSong(info);
		    }
		//songs.add(info);
	    }


	    } catch(IOException e) {
		System.err.println(e.toString());
	    }
	}
    }
}