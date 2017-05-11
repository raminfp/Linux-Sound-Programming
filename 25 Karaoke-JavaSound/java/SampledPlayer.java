


import java.io.IOException;

import javax.sound.sampled.Line;
import javax.sound.sampled.Mixer;
import javax.sound.sampled.AudioFormat;
import javax.sound.sampled.AudioInputStream;
import javax.sound.sampled.AudioSystem;
import javax.sound.sampled.DataLine;
import javax.sound.sampled.TargetDataLine;
import javax.sound.sampled.FloatControl;
import javax.sound.sampled.LineUnavailableException;
import javax.sound.sampled.SourceDataLine;
import javax.sound.sampled.Control;

import javax.swing.*;

public class SampledPlayer {

    private DisplayReceiver receiver;
    private Mixer mixer;

    public SampledPlayer(/* DisplayReceiver receiver, */ Mixer mixer) {
	this.receiver = receiver;
	this.mixer = mixer;
    }

    //This method creates and returns an
    // AudioFormat object for a given set of format
    // parameters.  If these parameters don't work
    // well for you, try some of the other
    // allowable parameter values, which are shown
    // in comments following the declarations.
    private static AudioFormat getAudioFormat(){
	float sampleRate = 44100.0F;
	//8000,11025,16000,22050,44100
	int sampleSizeInBits = 16;
	//8,16
	int channels = 1;
	//1,2
	boolean signed = true;
	//true,false
	boolean bigEndian = false;
	//true,false
	return new AudioFormat(sampleRate,
			       sampleSizeInBits,
			       channels,
			       signed,
			       bigEndian);
    }//end getAudioFormat

 

    public  void playAudio() throws Exception {
	AudioFormat audioFormat;
	TargetDataLine targetDataLine;
	
	audioFormat = getAudioFormat();
	DataLine.Info dataLineInfo =
	    new DataLine.Info(
			      TargetDataLine.class,
			      audioFormat);
	targetDataLine = (TargetDataLine)
	    AudioSystem.getLine(dataLineInfo);
	
	targetDataLine.open(audioFormat, 
			    audioFormat.getFrameSize() * Constants.FRAMES_PER_BUFFER);
	targetDataLine.start();
	
	/*
	for (Control control: targetDataLine.getControls()) {
	    System.out.println("Target control: " + control.getType());
	}
	*/

	playAudioStream(new AudioInputStream(targetDataLine), mixer);
    } // playAudioFile
     
    /** Plays audio from the given audio input stream. */
    public  void playAudioStream(AudioInputStream audioInputStream, Mixer mixer) {

	new AudioPlayer(audioInputStream, mixer).start();
    } // playAudioStream

    class AudioPlayer extends Thread {
	AudioInputStream audioInputStream;
	SourceDataLine dataLine;
	AudioFormat audioFormat;

	// YIN stuff
	// PitchProcessorWrapper ppw;

	AudioPlayer( AudioInputStream audioInputStream, Mixer mixer) {
	    this.audioInputStream = audioInputStream;

	    // Set to nearly max, like Midi sequencer does
	    Thread curr = Thread.currentThread();
	    Debug.println("Priority on sampled: " + curr.getPriority());
	    int priority = Thread.NORM_PRIORITY
		+ ((Thread.MAX_PRIORITY - Thread.NORM_PRIORITY) * 3) / 4;
	    curr.setPriority(priority);
	    Debug.println("Priority now on sampled: " + curr.getPriority());

	    // Audio format provides information like sample rate, size, channels.
	    audioFormat = audioInputStream.getFormat();
	    Debug.println( "Play input audio format=" + audioFormat );
     
	    // Open a data line to play our type of sampled audio.
	    // Use SourceDataLine for play and TargetDataLine for record.

	    if (mixer == null) {
		System.out.println("can't find a mixer");
	    } else {
		Line.Info[] lines = mixer.getSourceLineInfo();
		if (lines.length >= 1) {
		    try {
			dataLine = (SourceDataLine) AudioSystem.getLine(lines[0]);
			System.out.println("Got a source line for " + mixer.toString());
		    } catch(Exception e) {
		    }
		} else {
		    System.out.println("no source lines for this mixer " + mixer.toString());
		}
	    }

		for (Control control: mixer.getControls()) {
		    System.out.println("Mixer control: " + control.getType());
		}

		

	    DataLine.Info info = null;
	    if (dataLine == null) { 
		info = new DataLine.Info( SourceDataLine.class, audioFormat );
		if ( !AudioSystem.isLineSupported( info ) ) {
		    System.out.println( "Play.playAudioStream does not handle this type of audio on this system." );
		    return;
		}
	    }
     
	    try {
		// Create a SourceDataLine for play back (throws LineUnavailableException).
		if (dataLine == null) {	
		    dataLine = (SourceDataLine) AudioSystem.getLine( info );
		}
		Debug.println( "SourceDataLine class=" + dataLine.getClass() );
     
		// The line acquires system resources (throws LineAvailableException).
		dataLine.open( audioFormat,
			       audioFormat.getFrameSize() * Constants.FRAMES_PER_BUFFER);
     
		for (Control control: dataLine.getControls()) {
		    System.out.println("Source control: " + control.getType());
		}
		// Adjust the volume on the output line.
		if( dataLine.isControlSupported( FloatControl.Type.VOLUME) ) {
		    // if( dataLine.isControlSupported( FloatControl.Type.MASTER_GAIN ) ) {
		    //FloatControl volume = (FloatControl) dataLine.getControl( FloatControl.Type.MASTER_GAIN );		    
		    FloatControl volume = (FloatControl) dataLine.getControl( FloatControl.Type.VOLUME);
		    System.out.println("Max vol " + volume.getMaximum());
		    System.out.println("Min vol " + volume.getMinimum());
		    System.out.println("Current vol " + volume.getValue());
		    volume.setValue( 60000.0F );
		    System.out.println("New vol " + volume.getValue());
		} else {
		    System.out.println("Volume control not supported");
		}
		if (dataLine.isControlSupported( FloatControl.Type.REVERB_RETURN)) {
		    System.out.println("reverb return supported");
		} else {
		    System.out.println("reverb return not supported");
		}
		if (dataLine.isControlSupported( FloatControl.Type.REVERB_SEND)) {
		    System.out.println("reverb send supported");
		} else {
		    System.out.println("reverb send not supported");
		}
     

	    } catch ( LineUnavailableException e ) {
		e.printStackTrace();
	    }

	    // ppw = new PitchProcessorWrapper(audioInputStream, receiver);
	}


	public void run() {
     
	    // Allows the line to move data in and out to a port.
	    dataLine.start();
	    
	    // Create a buffer for moving data from the audio stream to the line.
	    int bufferSize = (int) audioFormat.getSampleRate() * audioFormat.getFrameSize();
	    bufferSize =  audioFormat.getFrameSize() * Constants.FRAMES_PER_BUFFER;
	    Debug.println("Buffer size: " + bufferSize);
	    byte [] buffer = new byte[bufferSize];
	    
	    try {
		int bytesRead = 0;
		while ( bytesRead >= 0 ) {
		    bytesRead = audioInputStream.read( buffer, 0, buffer.length );
		    if ( bytesRead >= 0 ) {
			int framesWritten = dataLine.write( buffer, 0, bytesRead );
			// ppw.write(buffer, bytesRead);
		    }
		} // while
	    } catch ( IOException e ) {
		e.printStackTrace();
	    }
     
	    // Continues data line I/O until its buffer is drained.
	    dataLine.drain();
     
	    Debug.println( "Sampled player closing line." );
	    // Closes the data line, freeing any resources such as the audio device.
	    dataLine.close();
	}
    }

    // Turn into a GUI version or pick up from prefs
    public void listMixers() {
	try{
	    Mixer.Info[] mixerInfo = 
		AudioSystem.getMixerInfo();
	    System.out.println("Available mixers:");
	    for(int cnt = 0; cnt < mixerInfo.length;
		cnt++){
		System.out.println(mixerInfo[cnt].
				   getName());
		
		Mixer mixer = AudioSystem.getMixer(mixerInfo[cnt]);
		Line.Info[] sourceLines = mixer.getSourceLineInfo();
		for (Line.Info s: sourceLines) {
		    System.out.println("  Source line: " + s.toString());
		}
		Line.Info[] targetLines = mixer.getTargetLineInfo();
		for (Line.Info t: targetLines) {
		    System.out.println("  Target line: " + t.toString());
		}
		
   
	    }//end for loop
	} catch(Exception e) {
	}
    }
}