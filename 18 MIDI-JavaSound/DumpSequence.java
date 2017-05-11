/*
 *	DumpSequence.java
 *
 *	This file is part of jsresources.org
 */

/*
 * Copyright (c) 1999, 2000 by Matthias Pfisterer
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

/*
  |<---            this code is formatted to fit into 80 columns             --->|
*/

import java.io.File;
import java.io.IOException;

import javax.sound.midi.MidiSystem;
import javax.sound.midi.InvalidMidiDataException;
import javax.sound.midi.Sequence;
import javax.sound.midi.Track;
import javax.sound.midi.MidiEvent;
import javax.sound.midi.MidiMessage;
import javax.sound.midi.ShortMessage;
import javax.sound.midi.MetaMessage;
import javax.sound.midi.SysexMessage;
import javax.sound.midi.Receiver;

public class DumpSequence
{
    private static String[]	sm_astrKeyNames = {"C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B"};

    private static Receiver		sm_receiver = new DumpReceiver(System.out, true);




    public static void main(String[] args) {
	/*
	 *	We check that there is exactely one command-line
	 *	argument. If not, we display the usage message and
	 *	exit.
	 */
	if (args.length != 1) {
	    out("DumpSequence: usage:");
	    out("\tjava DumpSequence <midifile>");
	    System.exit(1);
	}
	/*
	 *	Now, that we're shure there is an argument, we take it as
	 *	the filename of the soundfile we want to play.
	 */
	String	strFilename = args[0];
	File	midiFile = new File(strFilename);

	/*
	 *	We try to get a Sequence object, which the content
	 *	of the MIDI file.
	 */
	Sequence	sequence = null;
	try {
	    sequence = MidiSystem.getSequence(midiFile);
	} catch (InvalidMidiDataException e) {
	    e.printStackTrace();
	    System.exit(1);
	} catch (IOException e) {
	    e.printStackTrace();
	    System.exit(1);
	}

	/*
	 *	And now, we output the data.
	 */
	if (sequence == null) {
	    out("Cannot retrieve Sequence.");
	} else {
	    out("---------------------------------------------------------------------------");
	    out("File: " + strFilename);
	    out("---------------------------------------------------------------------------");
	    out("Length: " + sequence.getTickLength() + " ticks");
	    out("Duration: " + sequence.getMicrosecondLength() + " microseconds");
	    out("---------------------------------------------------------------------------");
	    float	fDivisionType = sequence.getDivisionType();
	    String	strDivisionType = null;
	    if (fDivisionType == Sequence.PPQ) {
		strDivisionType = "PPQ";
	    } else if (fDivisionType == Sequence.SMPTE_24) {
		strDivisionType = "SMPTE, 24 frames per second";
	    } else if (fDivisionType == Sequence.SMPTE_25) {
		strDivisionType = "SMPTE, 25 frames per second";
	    } else if (fDivisionType == Sequence.SMPTE_30DROP) {
		strDivisionType = "SMPTE, 29.97 frames per second";
	    } else if (fDivisionType == Sequence.SMPTE_30) {
		strDivisionType = "SMPTE, 30 frames per second";
	    }

	    out("DivisionType: " + strDivisionType);

	    String	strResolutionType = null;
	    if (sequence.getDivisionType() == Sequence.PPQ) {
		strResolutionType = " ticks per beat";
	    } else {
		strResolutionType = " ticks per frame";
	    }
	    out("Resolution: " + sequence.getResolution() + strResolutionType);
	    out("---------------------------------------------------------------------------");
	    Track[]	tracks = sequence.getTracks();
	    for (int nTrack = 0; nTrack < tracks.length; nTrack++) {
		out("Track " + nTrack + ":");
		out("-----------------------");
		Track	track = tracks[nTrack];
		for (int nEvent = 0; nEvent < track.size(); nEvent++) {
		    MidiEvent	event = track.get(nEvent);
		    output(event);
		}
		out("---------------------------------------------------------------------------");
	    }
	}
    }


    public static void output(MidiEvent event) {
	MidiMessage	message = event.getMessage();
	long		lTicks = event.getTick();
	sm_receiver.send(message, lTicks);
    }


    private static void out(String strMessage) {
	System.out.println(strMessage);
    }
}
/*** DumpSequence.java ***/

