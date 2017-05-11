

import javax.sound.sampled.*;

public class DeviceInfo {

    public static void main(String[] args) throws Exception {

	Mixer.Info[] minfoSet = AudioSystem.getMixerInfo();
	System.out.println("Mixers:");
	for (Mixer.Info minfo: minfoSet) {
	    System.out.println("   " + minfo.toString());

	    Mixer m = AudioSystem.getMixer(minfo);
	    System.out.println("    Mixer: " + m.toString());
	    System.out.println("      Source lines");
	    Line.Info[] slines = m.getSourceLineInfo();
	    for (Line.Info s: slines) {
		System.out.println("        " + s.toString());
	    }

	    Line.Info[] tlines = m.getTargetLineInfo();
	    System.out.println("      Target lines");
	    for (Line.Info t: tlines) {
		System.out.println("        " + t.toString());
	    }
	}
    }
}