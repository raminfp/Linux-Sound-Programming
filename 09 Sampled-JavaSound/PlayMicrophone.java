
import java.io.File;
import java.io.IOException;

     
import javax.sound.sampled.AudioFormat;
import javax.sound.sampled.AudioInputStream;
import javax.sound.sampled.AudioSystem;
import javax.sound.sampled.DataLine;
import javax.sound.sampled.Line;
import javax.sound.sampled.Line.Info;
import javax.sound.sampled.TargetDataLine;
import javax.sound.sampled.FloatControl;
import javax.sound.sampled.LineUnavailableException;
import javax.sound.sampled.SourceDataLine;

public class PlayMicrophone {
    private static final int FRAMES_PER_BUFFER = 1024;

    public static void main(String[] args) throws Exception {
	/*
	 *	We check that there is exactely one command-line
	 *	argument. If not, we display the usage message and
	 *	exit.
	 */

	new PlayMicrophone().playAudio();
    }



    private void out(String strMessage)
    {
	System.out.println(strMessage);
    }

  //This method creates and returns an
  // AudioFormat object for a given set of format
  // parameters.  If these parameters don't work
  // well for you, try some of the other
  // allowable parameter values, which are shown
  // in comments following the declarations.
  private  AudioFormat getAudioFormat(){
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

    public void playAudio() throws Exception {
	AudioFormat audioFormat;
	TargetDataLine targetDataLine;
	
	audioFormat = getAudioFormat();
	DataLine.Info dataLineInfo =
	    new DataLine.Info(
			      TargetDataLine.class,
			      audioFormat);
	targetDataLine = (TargetDataLine)
	    AudioSystem.getLine(dataLineInfo);

	/*
	Line.Info lines[] = AudioSystem.getTargetLineInfo(dataLineInfo);
	for (int n = 0; n < lines.length; n++) {
	    System.out.println("Target " + lines[n].toString() + " " + lines[n].getLineClass());
	}
	targetDataLine = (TargetDataLine)
	    AudioSystem.getLine(lines[0]);
	*/
	
	targetDataLine.open(audioFormat, 
			    audioFormat.getFrameSize() * FRAMES_PER_BUFFER);
	targetDataLine.start();
	
	playAudioStream(new AudioInputStream(targetDataLine));

	/*
	File soundFile = new File( fileName );
     
	try {
	    // Create a stream from the given file.
	    // Throws IOException or UnsupportedAudioFileException
	    AudioInputStream audioInputStream = AudioSystem.getAudioInputStream( soundFile );
	    // AudioSystem.getAudioInputStream( inputStream ); // alternate audio stream from inputstream
	    playAudioStream( audioInputStream );
	} catch ( Exception e ) {
	    System.out.println( "Problem with file " + fileName + ":" );
	    e.printStackTrace();
	}
	*/
    } // playAudioFile
     
    /** Plays audio from the given audio input stream. */
    public void playAudioStream( AudioInputStream audioInputStream ) {
	// Audio format provides information like sample rate, size, channels.
	AudioFormat audioFormat = audioInputStream.getFormat();
	System.out.println( "Play input audio format=" + audioFormat );
     
	// Open a data line to play our type of sampled audio.
	// Use SourceDataLine for play and TargetDataLine for record.
	DataLine.Info info = new DataLine.Info( SourceDataLine.class, audioFormat );

	Line.Info lines[] = AudioSystem.getSourceLineInfo(info);
	for (int n = 0; n < lines.length; n++) {
	    System.out.println("Source " + lines[n].toString() + " " + lines[n].getLineClass());
	}

	if ( !AudioSystem.isLineSupported( info ) ) {
	    System.out.println( "Play.playAudioStream does not handle this type of audio on this system." );
	    return;
	}
     
	try {
	    // Create a SourceDataLine for play back (throws LineUnavailableException).
	    SourceDataLine dataLine = (SourceDataLine) AudioSystem.getLine( info );
	    // System.out.println( "SourceDataLine class=" + dataLine.getClass() );
     
	    // The line acquires system resources (throws LineAvailableException).
	    dataLine.open( audioFormat,
			   audioFormat.getFrameSize() * FRAMES_PER_BUFFER);
     
	    // Adjust the volume on the output line.
	    if( dataLine.isControlSupported( FloatControl.Type.MASTER_GAIN ) ) {
		FloatControl volume = (FloatControl) dataLine.getControl( FloatControl.Type.MASTER_GAIN );
		volume.setValue( 6.0F );
	    }
     
	    // Allows the line to move data in and out to a port.
	    dataLine.start();
     
	    // Create a buffer for moving data from the audio stream to the line.
	    int bufferSize = (int) audioFormat.getSampleRate() * audioFormat.getFrameSize();
	    bufferSize =  audioFormat.getFrameSize() * FRAMES_PER_BUFFER;
	    System.out.println("Buffer size: " + bufferSize);
	    byte [] buffer = new byte[ bufferSize ];
     
	    // Move the data until done or there is an error.
	    try {
		int bytesRead = 0;
		while ( bytesRead >= 0 ) {
		    bytesRead = audioInputStream.read( buffer, 0, buffer.length );
		    if ( bytesRead >= 0 ) {
			System.out.println( "Play.playAudioStream bytes read=" + bytesRead +
			", frame size=" + audioFormat.getFrameSize() + ", frames read=" + bytesRead / audioFormat.getFrameSize() );
			// Odd sized sounds throw an exception if we don't write the same amount.
			int framesWritten = dataLine.write( buffer, 0, bytesRead );
		    }
		} // while
	    } catch ( IOException e ) {
		e.printStackTrace();
	    }
     
	    System.out.println( "Play.playAudioStream draining line." );
	    // Continues data line I/O until its buffer is drained.
	    dataLine.drain();
     
	    System.out.println( "Play.playAudioStream closing line." );
	    // Closes the data line, freeing any resources such as the audio device.
	    dataLine.close();
	} catch ( LineUnavailableException e ) {
	    e.printStackTrace();
	}
    } // playAudioStream

}


