
import java.util.Vector;

/**
 * Contains the text of a line.
 * And also the ticks for the start and end of the line.
 * The notes which belong to this text are included -
 * but the algorithm to set these is not accurate
 */
public class LyricLine {
    public long startTick = Constants.NO_TICK;
    public long endTick;
    public String line;
    public Vector<DurationNote> notes = new Vector<DurationNote> ();
}