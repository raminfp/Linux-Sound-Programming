/*
 * KaraokePlayer.java
 *
 */

import javax.swing.*;
import javax.sound.sampled.*;

public class KaraokePlayerSampled {


    public static void main(String[] args) throws Exception {
	if (args.length != 1) {
	    System.err.println("KaraokePlayer: usage: " +
			       "KaraokePlayer <midifile>");
	    System.exit(1);
	}
	String	strFilename = args[0];

	Mixer.Info[] mixerInfo = AudioSystem.getMixerInfo();

	String[] possibleValues = new String[mixerInfo.length];
	for(int cnt = 0; cnt < mixerInfo.length; cnt++){
	    possibleValues[cnt] = mixerInfo[cnt].getName();
	}
	Object selectedValue = JOptionPane.showInputDialog(null, "Choose mixer", "Input",
							   JOptionPane.INFORMATION_MESSAGE, null,
							   possibleValues, possibleValues[0]);

	System.out.println("Mixer string selected " + ((String)selectedValue));


	Mixer mixer = null;
	for(int cnt = 0; cnt < mixerInfo.length; cnt++){
	    if (mixerInfo[cnt].getName().equals((String)selectedValue)) {
		mixer = AudioSystem.getMixer(mixerInfo[cnt]);
		System.out.println("Got a mixer");
		break;
	    }
	}//end for loop


	MidiPlayer midiPlayer = new MidiPlayer();
	midiPlayer.playMidiFile(strFilename);

        SampledPlayer sampledPlayer = new SampledPlayer(/* midiPlayer.getReceiver(), */ mixer);
        sampledPlayer.playAudio();
    }
}



