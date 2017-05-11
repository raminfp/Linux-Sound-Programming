

public class Debug {

    public static final boolean DEBUG = false;

    public static void println(String str) {
	if (DEBUG) {
	    System.out.println(str);
	}
    }

    public static void printf(String format, Object... args) {
	if (DEBUG) {
	    System.out.printf(format, args);
	}
    }
}
	    
	    