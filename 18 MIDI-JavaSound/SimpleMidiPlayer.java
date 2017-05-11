/*
 *	SimpleMidiPlayer.java
 *
 *	This file is part of jsresources.org
 */

/*
 * Copyright (c) 1999 - 2001 by Matthias Pfisterer
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * - Redistributions of source code must retain the above copyright notice,
 *   this list of conditions and the following disclaimer.
 * - Redistributions in binary form must reproduce the above copyright
 *   notice, this list of conditions and the following disclaimer in the
 *   documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
 * OF THE POSSIBILITY OF SUCH DAMAGE.
 */

import java.io.File;
import java.io.IOException;

import javax.sound.midi.InvalidMidiDataException;
import javax.sound.midi.MidiSystem;
import javax.sound.midi.MidiUnavailableException;
import javax.sound.midi.MetaEventListener;
import javax.sound.midi.MetaMessage;
import javax.sound.midi.Sequence;
import javax.sound.midi.Sequencer;
import javax.sound.midi.Synthesizer;
import javax.sound.midi.Receiver;
import javax.sound.midi.Transmitter;
import javax.sound.midi.MidiChannel;
import javax.sound.midi.ShortMessage;

public class SimpleMidiPlayer {

    private static Sequencer sm_sequencer = null;
    private static Synthesizer sm_synthesizer = null;



    public static void main(String[]args) {

	if (args.length == 0 || args[0].equals("-h")) {
	    printUsageAndExit();
	}

	String strFilename = args[0];
	File midiFile = new File(strFilename);

	/*
	 *      We read in the MIDI file to a Sequence object.
	 */
	Sequence sequence = null;
	try {
	    sequence = MidiSystem.getSequence(midiFile);
	}
	catch(InvalidMidiDataException e) {
	    e.printStackTrace();
	    System.exit(1);
	}
	catch(IOException e) {
	    e.printStackTrace();
	    System.exit(1);
	}

	/*
	 *      Now, we need a Sequencer to play the sequence.
	 *      Here, we simply request the default sequencer.
	 *      With an argument of false, it does not create
	 *      a default syntesizer.
	 */
	try {
	    sm_sequencer = MidiSystem.getSequencer(false);
	}
	catch(MidiUnavailableException e) {
	    e.printStackTrace();
	    System.exit(1);
	}
	if (sm_sequencer == null) {
	    out("SimpleMidiPlayer.main(): can't get a Sequencer");
	    System.exit(1);
	}

	try {
	    sm_sequencer.open();
	}
	catch(MidiUnavailableException e) {
	    e.printStackTrace();
	    System.exit(1);
	}

	/*
	 *      Next step is to tell the Sequencer which
	 *      Sequence it has to play.
	 */
	try {
	    sm_sequencer.setSequence(sequence);
	}
	catch(InvalidMidiDataException e) {
	    e.printStackTrace();
	    System.exit(1);
	}


	Receiver synthReceiver = null;
	if (!(sm_sequencer instanceof Synthesizer)) {
	    /*
	     *      We try to get the default synthesizer, open()
	     *      it and chain it to the sequencer with a
	     *      Transmitter-Receiver pair.
	     */
	    try {
		sm_synthesizer = MidiSystem.getSynthesizer();
		sm_synthesizer.open();
		synthReceiver = sm_synthesizer.getReceiver();
		Transmitter seqTransmitter = sm_sequencer.getTransmitter();
		seqTransmitter.setReceiver(synthReceiver);
	    }
	    catch(MidiUnavailableException e) {
		e.printStackTrace();
	    }
	}

	/*
	 *      Now, we can start playing
	 */
	sm_sequencer.start();

	try {
	    Thread.sleep(5000);
	}
	catch(InterruptedException e) {
	    e.printStackTrace();
	}
    }

    private static void printUsageAndExit() {
	out("SimpleMidiPlayer: usage:");
	out("\tjava SimpleMidiPlayer <midifile>");
	System.exit(1);
    }

    private static void out(String strMessage) {
	System.out.println(strMessage);
    }
}

