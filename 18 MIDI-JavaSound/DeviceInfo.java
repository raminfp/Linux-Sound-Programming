

import javax.sound.midi.*;
import java.util.*;

public class DeviceInfo {

    public static void main(String[] args) throws Exception {
	MidiDevice.Info[] devices;

	/*
	MidiDevice.Info[] info = p.getDeviceInfo();
	for (int m = 0; m < info.length; m++) {
	    System.out.println(info[m].toString());
	}
	*/

	System.out.println("MIDI devices:");
	devices = MidiSystem.getMidiDeviceInfo();
	for (MidiDevice.Info info: devices) {
	    System.out.println("    Name: " + info.toString() + 
			       ", Decription: " +
			       info.getDescription() + 
			       ", Vendor: " +
			       info.getVendor());
	    MidiDevice device = MidiSystem.getMidiDevice(info);
	    if (! device.isOpen()) {
		device.open();
	    }
	    if (device instanceof Sequencer) {
		System.out.println("        Device is a sequencer");
	    }
	    if (device instanceof Synthesizer) {
		System.out.println("        Device is a synthesizer");
	    }
	    System.out.println("        Open receivers:");
	    List<Receiver> receivers = device.getReceivers();
	    for (Receiver r: receivers) {
		System.out.println("            " + r.toString());
	    }
	    try {
		System.out.println("\n        Default receiver: " + 
				   device.getReceiver().toString());

		System.out.println("\n        Open receivers now:");
		receivers = device.getReceivers();
		for (Receiver r: receivers) {
		    System.out.println("            " + r.toString());
		}
	    } catch(MidiUnavailableException e) {
		System.out.println("        No default receiver");
	    }
	
	    System.out.println("\n        Open transmitters:");
	    List<Transmitter> transmitters = device.getTransmitters();
	    for (Transmitter t: transmitters) {
		System.out.println("            " + t.toString());
	    }
	    try {
		System.out.println("\n        Default transmitter: " + 
				   device.getTransmitter().toString());

		System.out.println("\n        Open transmitters now:");
		transmitters = device.getTransmitters();
		for (Transmitter t: transmitters) {
		    System.out.println("            " + t.toString());
		}
	    } catch(MidiUnavailableException e) {
		System.out.println("        No default transmitter");
	    }
	    device.close();
	}

	
	Sequencer sequencer = MidiSystem.getSequencer();
	System.out.println("Default system sequencer is " + 
			   sequencer.getDeviceInfo().toString() +
			   " (" + sequencer.getClass() + ")");

	Synthesizer synthesizer = MidiSystem.getSynthesizer();
	System.out.println("Default system synthesizer is " + 
			   synthesizer.getDeviceInfo().toString() +
			   " (" + synthesizer.getClass() + ")");

    }
}
