

import javax.swing.*;
import java.awt.*;
import java.awt.event.*;
import javax.sound.midi.*;
import java.util.Vector;
import java.util.Map;
import java.io.*;


public class MidiGUI extends JFrame {
    //private GridLayout mgr = new GridLayout(3,1);
    private BorderLayout mgr = new BorderLayout();

    private PianoPanel pianoPanel;
    private MelodyPanel melodyPanel;

    private AttributedLyricPanel lyric1;
    private AttributedLyricPanel lyric2;
    private AttributedLyricPanel[] lyricLinePanels;
    private int whichLyricPanel = 0;

    private JPanel lyricsPanel = new JPanel();

    private Sequencer sequencer;
    private Sequence sequence;
    private Vector<LyricLine> lyricLines;

    private int lyricLine = -1;

    private boolean inLyricHeader = true;
    private Vector<DurationNote> melodyNotes;

    private Map<Character, String> pinyinMap;

    private int language;

    public MidiGUI(final Sequencer sequencer) {
	this.sequencer = sequencer;
	sequence = sequencer.getSequence();

	// get lyrics and notes from Sequence Info
	lyricLines = SequenceInformation.getLyrics();
	melodyNotes = SequenceInformation.getMelodyNotes();
	language = SequenceInformation.getLanguage();

	pianoPanel = new PianoPanel(sequencer);
	melodyPanel = new MelodyPanel(sequencer);

	pinyinMap = CharsetEncoding.loadPinyinMap();
	lyric1 = new AttributedLyricPanel(pinyinMap);
	lyric2 = new AttributedLyricPanel(pinyinMap);
	lyricLinePanels = new AttributedLyricPanel[] {
	    lyric1, lyric2};

	Debug.println("Lyrics ");

	for (LyricLine line: lyricLines) {
	    Debug.println(line.line + " " + line.startTick + " " + line.endTick +
			  " num notes " + line.notes.size());
	}

	getContentPane().setLayout(mgr);
	/*
	getContentPane().add(pianoPanel);
	getContentPane().add(melodyPanel);

	getContentPane().add(lyricsPanel);
	*/
	getContentPane().add(pianoPanel, BorderLayout.PAGE_START);
	getContentPane().add(melodyPanel,  BorderLayout.CENTER);

	getContentPane().add(lyricsPanel,  BorderLayout.PAGE_END);


	lyricsPanel.setLayout(new GridLayout(2, 1));
	lyricsPanel.add(lyric1);
	lyricsPanel.add(lyric2);
	setLanguage(language);

	setText(lyricLinePanels[whichLyricPanel], lyricLines.elementAt(0).line);

	Debug.println("First lyric line: " + lyricLines.elementAt(0).line);
	if (lyricLine < lyricLines.size() - 1) {
	    setText(lyricLinePanels[(whichLyricPanel+1) % 2], lyricLines.elementAt(1).line);
	    Debug.println("Second lyric line: " + lyricLines.elementAt(1).line);
	}

	// handle window closing
	setDefaultCloseOperation(JFrame.DO_NOTHING_ON_CLOSE);
	addWindowListener(new WindowAdapter() {
		public void windowClosing(WindowEvent e) {
		    sequencer.stop();
		    System.exit(0);
                }
            });

	// handle resize events
	addComponentListener(new ComponentAdapter() {
		public void componentResized(ComponentEvent e) {
		    Debug.printf("Component has resized to width %d, height %d\n",
				      getWidth(), getHeight());
		    // force resize of children - especially the middle MelodyPanel
		    e.getComponent().validate();        
		}
		public void componentShown(ComponentEvent e) {
		    Debug.printf("Component is visible with width %d, height %d\n",
				      getWidth(), getHeight());          
		}
	    });

	setSize(1600, 900);
	setVisible(true);
    }

    public void setLanguage(int lang) {
	lyric1.setLanguage(lang);
	lyric2.setLanguage(lang);
    }


    /**
     * A lyric starts with a header section
     * We have to skip over that, but can pick useful
     * data out of it
     */

    /**
     * header format is
     *   \@Llanguage code
     *   \@Ttitle
     *   \@Tsinger
     */
 
    public void setLyric(String txt) {
	Debug.println("Setting lyric to " + txt);
	if (inLyricHeader) {
	    if (txt.startsWith("@")) {
		Debug.println("Header: " + txt);
		return;
	    } else {
		inLyricHeader = false;
	    }
	}
	
	if ((lyricLine == -1) && (txt.charAt(0) == '\\')) {
	    lyricLine = 0;
	    colourLyric(lyricLinePanels[whichLyricPanel], txt.substring(1));
	    // lyricLinePanels[whichLyricPanel].colourLyric(txt.substring(1));
	    return;
	}
	
	if (txt.equals("\r\n") || (txt.charAt(0) == '/') || (txt.charAt(0) == '\\')) {
	    if (lyricLine < lyricLines.size() -1)
		Debug.println("Setting next lyric line to \"" + 
			      lyricLines.elementAt(lyricLine + 1).line + "\"");

	    final int thisPanel = whichLyricPanel;
	    whichLyricPanel = (whichLyricPanel + 1) % 2;
	    
	    Debug.println("Setting new lyric line at tick " + 
			  sequencer.getTickPosition());
	    
	    lyricLine++;

	    // if it's a \ r /, the rest of the txt should be the next  word to
	    // be coloured

	    if ((txt.charAt(0) == '/') || (txt.charAt(0) == '\\')) {
		Debug.println("Colouring newline of " + txt);
		colourLyric(lyricLinePanels[whichLyricPanel], txt.substring(1));
	    }

	    // Update the current line of text to show the one after next
	    // But delay the update until 0.25 seconds after the next line
	    // starts playing, to preserve visual continuity
	    if (lyricLine + 1 < lyricLines.size()) {
		/*
		  long startNextLineTick = lyricLines.elementAt(lyricLine).startTick;
		  long delayForTicks = startNextLineTick - sequencer.getTickPosition();
		  Debug.println("Next  current "  + startNextLineTick + " " + sequencer.getTickPosition());
		  float microSecsPerQNote = sequencer.getTempoInMPQ();
		  float delayInMicroSecs = microSecsPerQNote * delayForTicks / 24 + 250000L;
		*/

		final Vector<DurationNote> notes = lyricLines.elementAt(lyricLine).notes;

		final int nextLineForPanel = lyricLine + 1;

		if (lyricLines.size() >= nextLineForPanel) {
		    Timer timer = new Timer((int) 1000,
					    new ActionListener() {
						public void actionPerformed(ActionEvent e) {
						    if (nextLineForPanel >= lyricLines.size()) {
							return;
						    }
						    setText(lyricLinePanels[thisPanel], lyricLines.elementAt(nextLineForPanel).line);
						    //lyricLinePanels[thisPanel].setText(lyricLines.elementAt(nextLineForPanel).line);
						
						}
					    });
		    timer.setRepeats(false);
		    timer.start();
		} else {
		    // no more lines
		}		
	    }
	} else {
	    Debug.println("Playing lyric " + txt);
	    colourLyric(lyricLinePanels[whichLyricPanel], txt);
	    //lyricLinePanels[whichLyricPanel].colourLyric(txt);
	}
    }

    /**
     * colour the lyric of a panel.
     * called by one thread, makes changes in GUI thread
     */
    private void colourLyric(final AttributedLyricPanel p, final String txt) {
	SwingUtilities.invokeLater(new Runnable() {
		public void run() {
		    Debug.print("Colouring lyric \"" + txt + "\"");
		    if (p == lyric1) Debug.println(" on panel 1");
		    else Debug.println(" on panel 2");
		    p.colourLyric(txt);
		}
	    }
	    );
    }

    /**
     * set the lyric of a panel.
     * called by one thread, makes changes in GUI thread
     */
    private void setText(final AttributedLyricPanel p, final String txt) {
	SwingUtilities.invokeLater(new Runnable() {
		public void run() {
		    Debug.println("Setting text \"" + txt + "\"");
		    if (p == lyric1) Debug.println(" on panel 1");
		    else Debug.println(" on panel 2");
		    p.setText(txt);
		}
	    }
	    );
    }

    public void setNote(long timeStamp, int onOff, int note) {
	Debug.printf("Setting note in gui to %d\n", note);

	if (onOff == Constants.MIDI_NOTE_OFF) {
	    pianoPanel.drawNoteOff(note);
	} else if (onOff == Constants.MIDI_NOTE_ON) {
	    pianoPanel.drawNoteOn(note);
	}
    }
}


