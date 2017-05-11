
import java.util.Vector;
import javax.swing.*;
import java.awt.*;
import java.awt.event.*;
import javax.sound.midi.*;
import java.awt.image.BufferedImage;
import java.io.*;
import javax.imageio.*;

public class MelodyPanel extends JPanel {

    private static int DBL_BUF_SCALE = 2;
    private static final int NOTE_HEIGHT = 10;
    private static final int SLEEP_MSECS = 5;

    private long timeStamp;
    private Vector<DurationNote> notes;
    private Sequencer sequencer;
    private Sequence sequence;
    private int maxNote;
    private int minNote;
    private long tickLength = -1;
    private long currentTick = -1;
    private Image image = null;

    /**
     * The panel where the melody notes are shown in a
     * scrolling panel 
     */
    public MelodyPanel(Sequencer sequencer) {

	maxNote = SequenceInformation.getMaxMelodyNote();
	minNote = SequenceInformation.getMinMelodyNote();
	Debug.println("Max: " + maxNote + " Min " + minNote);
	notes = SequenceInformation.getMelodyNotes();
	this.sequencer = sequencer;
	tickLength = sequencer.getTickLength() + 1000; // hack to make white space at end, plus fix bug

	//new TickPointer().start();
	// handle resize events
	addComponentListener(new ComponentAdapter() {
		public void componentResized(ComponentEvent e) {
		    Debug.printf("Component melody panel has resized to width %d, height %d\n",
				      getWidth(), getHeight());          
		}
		public void componentShown(ComponentEvent e) {
		    Debug.printf("Component malody panel is visible with width %d, height %d\n",
				      getWidth(), getHeight());          
		}
	    });

    }

    /**
     * Redraw the melody image after each tick
     * to give a scrolling effect
     */
    private class TickPointer extends Thread {
	public void run() {
	    while (true) {
		currentTick = sequencer.getTickPosition();
		MelodyPanel.this.repaint();
		/*
		SwingUtilities.invokeLater(
					    new Runnable() {
						public void run() {
						    synchronized(MelodyPanel.this) {
						    MelodyPanel.this.repaint();
						    }
						}
					    });
		*/
		try {
		    sleep(SLEEP_MSECS);
		} catch (Exception e) {
		    // keep going
		    e.printStackTrace();
		}
	    }
	}

    }

    /**
     * Draw the melody into a buffer so we can just copy bits to the screen
     */
    private void drawMelody(Graphics g, int front, int width, int height) {
	try {
	g.setColor(Color.WHITE);
	g.fillRect(0, 0, width, height);
	g.setColor(Color.BLACK);

	String title = SequenceInformation.getTitle();
	if (title != null) {
	    //Font f = new Font("SanSerif", Font.ITALIC, 40);
	    Font f = new Font(Constants.CHINESE_FONT, Font.ITALIC, 40);
	    g.setFont(f);
	    int strWidth = g.getFontMetrics().stringWidth(title);
	    g.drawString(title, (front - strWidth/2), height/2);
	    Debug.println("Drawn title " + title);
	}

	for (DurationNote note: notes) {
	    long startNote = note.startTick;
	    long endNote = note.endTick;
	    int value = note.note;

	    int ht = (value - minNote) * (height - NOTE_HEIGHT) / (maxNote - minNote) + NOTE_HEIGHT/2;
	    // it's upside down
	    ht = height - ht;

	    long start = front + (int) (startNote * DBL_BUF_SCALE);
	    long end = front + (int) (endNote * DBL_BUF_SCALE);

	    drawNote(g, ht, start, end);
	    //g.drawString(title, (int)start, (int)height/2);	
	}
	} catch(Exception e) {
	    System.err.println("Drawing melody error " + e.toString());
	}
    }

    /**
     * Draw a horizontal bar to represent a nore
     */
    private void drawNote(Graphics g, int height, long start, long end) {
	Debug.printf("Drawing melody at start %d end %d height %d\n", start, end,  height - NOTE_HEIGHT/2);

	g.fillRect((int) start, height - NOTE_HEIGHT/2, (int) (end-start), NOTE_HEIGHT);
    }

    /**
     * Draw a vertical line in the middle of the screen to
     * represent where we are in the playing notes
     */
    private void paintTick(Graphics g, long width, long height) {
	long x = (currentTick * width) / tickLength;
	g.drawLine((int) width/2, 0, (int) width/2, (int) height);
	//System.err.println("Painted tcik");
    }

    // leave space at the front of the image to draw title, etc
    int front = 1000;

    /**
     * First time, draw the melody notes into an off-screen buffer
     * After that, copy a segment of the buffer into the image,
     * with the centre of the image the current note
     */
    @Override
    public void paintComponent(Graphics g) {
	int ht = getHeight();
	int width = getWidth();
	//int front = width / 2;

	synchronized(this) {
	if (image == null) {
	    /*
	     * We want to stretch out the notes so that they appear nice and wide on the screen.
	     * A DBL_BUF_SCALE of 2 does this okay. But then tickLength * DBL_BUF_SCALE may end
	     * up larger than an int, and we can't make a BufferedImage wider than MAXINT.
	     * So we may have to adjust DBL_BUF_SCALE.
	     *
	     * Yes, I know we ask Java to rescale images on the fly, but that costs in runtime.
	     */
	    
	    Debug.println("tick*DBLBUFSCALE " + tickLength * DBL_BUF_SCALE);
	    
	    if ((long) (tickLength * DBL_BUF_SCALE) > (long) Short.MAX_VALUE) {
		// DBL_BUF_SCALE = ((float)  Integer.MAX_VALUE) / ((float) tickLength);
		DBL_BUF_SCALE = 1;
		Debug.println("Adjusted DBL_BUF_SCALE to "+ DBL_BUF_SCALE);
	    }
	    
	    Debug.println("DBL_BUF_SCALE is "+ DBL_BUF_SCALE);

	    // draw melody into a buffered image
	    Debug.printf("New buffered img width %d ht %d\n", tickLength, ht);
	    image = new BufferedImage(front + (int) (tickLength * DBL_BUF_SCALE), ht, BufferedImage.TYPE_INT_RGB);
	    Graphics ig = image.getGraphics();
	    drawMelody(ig, front, (int) (tickLength * DBL_BUF_SCALE), ht);
	    new TickPointer().start();


	    try {
		File outputfile = new File("saved.png");
		ImageIO.write((BufferedImage) image, "png", outputfile);
	    } catch (Exception e) {
		System.err.println("Error in image write " + e.toString());
	    }

	}
	//System.err.printf("Drawing img from %d ht %d width %d\n", 
	//		  front + (int) (currentTick * DBL_BUF_SCALE - width/2), ht, width);
	
	boolean b = g.drawImage(image, 0, 0, width, ht, 
				front + (int) (currentTick * DBL_BUF_SCALE - width/2), 0, 
				front + (int) (currentTick * DBL_BUF_SCALE + width/2), ht,
		    null);
	/*System.out.printf("Ht of BI %d, width %d\n", ((BufferedImage)image).getHeight(), 
			  ((BufferedImage) image).getWidth());
	*/
	
	//if (b) System.err.println("Drawn ok"); else System.err.println("NOt drawn ok");
	paintTick(g, width, ht);
	}
    }

}
