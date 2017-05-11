
import java.nio.charset.Charset;
import java.util.Map;
import java.util.HashMap;
import java.io.*;


public class CharsetEncoding {

    public static boolean isChinese(int language) {
	switch (language) {
        case SongInformation.CHINESE1:
        case SongInformation.CHINESE2:
        case SongInformation.CHINESE8:
        case SongInformation.CHINESE131:
        case SongInformation.TAIWANESE3:
        case SongInformation.TAIWANESE7:
        case SongInformation.CANTONESE:
	    return true;
	}
	return false;
    }

    public static String toUnicode(int lang, byte[] bytes) {
	for (byte b: bytes) {
	    int n = 0;
	    if (b < 0) n = 256+b; else n = b;
	    Debug.printf (" %X",  n);
	}
	Debug.println("");

        switch (lang) {
        case SongInformation.ENGLISH:
        case SongInformation.ENGLISH146:
        case SongInformation.PHILIPPINE:
        case SongInformation.PHILIPPINE148:
            // case SongInformation.HINDI:
        case SongInformation.INDONESIAN:
        case SongInformation.SPANISH:
            return new String(bytes);

        case SongInformation.CHINESE1:
        case SongInformation.CHINESE2:
        case SongInformation.CHINESE8:
        case SongInformation.CHINESE131:
        case SongInformation.TAIWANESE3:
        case SongInformation.TAIWANESE7:
        case SongInformation.CANTONESE:
            //case SongInformation.KOREAN:

	    Charset charset = Charset.forName("gb2312");
	    return new String(bytes, charset);

        case SongInformation.KOREAN:
	    charset = Charset.forName("euckr");
	    return new String(bytes, charset);

        default:
            return "";
        }
    }

    public static Map<Character, String> loadPinyinMap() {
	Map<Character, String> pinyinMap = new  HashMap<Character, String> ();

	try {
	    BufferedReader in
		= new BufferedReader(new FileReader("pinyinmap.txt"));
	    String line;
	    while ((line = in.readLine()) != null) {
		pinyinMap.put(line.charAt(0), line.substring(2));
	    }
	} catch(IOException e) {
	    e.printStackTrace();
	}
	return pinyinMap;
    }

}