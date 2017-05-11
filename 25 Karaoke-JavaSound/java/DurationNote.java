

public class DurationNote {
    public int note;
    public long startTick;
    public long endTick;
    public long duration;

    public DurationNote(long start, int value) {
	startTick = start;
	note = value;
    }

    public String toString() {
	return "note " + note + " " + startTick + " " + duration;
    }
}