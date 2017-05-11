
import java.util.Vector;
import javax.swing.*;
import java.awt.*;
import javax.sound.midi.*;


public class PianoPanel extends JPanel {

    private final int HEIGHT = 100;
    private final int HEIGHT_OFFSET = 10;

    long timeStamp;
    private Vector<DurationNote> notes;
    private Vector<DurationNote> sungNotes;
    private int lastNoteDrawn = -1;
    private Sequencer sequencer;
    private Sequence sequence;
    private int maxNote;
    private int minNote;

    private Vector<DurationNote> unresolvedNotes = new Vector<DurationNote> ();

    private int playingNote = -1;

    public PianoPanel(Sequencer sequencer) {

	maxNote = SequenceInformation.getMaxMelodyNote();
	minNote = SequenceInformation.getMinMelodyNote();
	Debug.println("Max: " + maxNote + " Min " + minNote);
    }

    public Dimension getPreferredSize() {
	return new Dimension(1000, 120);
    }

    public void drawNoteOff(int note) {
	if (note < minNote || note > maxNote) {
	    return;
	}

	Debug.println("Note off played is " + note);
	if (note != playingNote) {
	    // Sometimes "note off" followed immediately by "note on"
	    // gets mixed up to "note on" followed by "note off".
	    // Ignore the "note off" since the next note has already
	    // been processed
	    Debug.println("Ignoring note off");
	    return;
	}
	playingNote = -1;
	repaint();
    }

    public void drawNoteOn(int note) {
	if (note < minNote || note > maxNote) {
	    return;
	}

	Debug.println("Note on played is " + note);
	playingNote = note;
	repaint();

    }



    private void drawPiano(Graphics g, int width, int height) {
	int noteWidth = width / (Constants.MIDI_NOTE_C8 - Constants.MIDI_NOTE_A0);
	for (int noteNum =  Constants.MIDI_NOTE_A0; // A0
	     noteNum <=  Constants.MIDI_NOTE_C8; // C8
	     noteNum++) {
	    
	    drawNote(g, noteNum, noteWidth);
	}
    }

    private void drawNote(Graphics g, int noteNum, int width) {
	if (isWhite(noteNum)) {
	    noteNum -= Constants.MIDI_NOTE_A0;
	    g.setColor(Color.WHITE);
	    g.fillRect(noteNum*width, HEIGHT_OFFSET, width, HEIGHT);
	    g.setColor(Color.BLACK);
	    g.drawRect(noteNum*width, HEIGHT_OFFSET, width, HEIGHT);
	} else {
	    noteNum -= Constants.MIDI_NOTE_A0;
	    g.setColor(Color.BLACK);
	    g.fillRect(noteNum*width, HEIGHT_OFFSET, width, HEIGHT);
	}
	if (playingNote != -1) {
	    g.setColor(Color.BLUE);
	    g.fillRect((playingNote - Constants.MIDI_NOTE_A0) * width, HEIGHT_OFFSET, width, HEIGHT);
	}	    
    }

    private boolean isWhite(int noteNum) {
	noteNum = noteNum % 12;
	switch (noteNum) {
	case 1:
	case 3:
	case 6:
	case 8:
	case 10:
	case 13: 
	    return false;
	default:
	    return true;
	}
    }

    @Override
    public void paintComponent(Graphics g) {

	int ht = getHeight();
	int width = getWidth();

	drawPiano(g, width, ht);

    }
}