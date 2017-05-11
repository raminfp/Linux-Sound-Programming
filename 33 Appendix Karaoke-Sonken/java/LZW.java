
/**
 * Based on code by Mark Nelson
 * http://marknelson.us/1989/10/01/lzw-data-compression/
 */

public class LZW {


    private final int BITS = 12;                   /* Setting the number of bits to 12, 13*/
    private final int HASHING_SHIFT = (BITS-8);    /* or 14 affects several constants.    */
    private final int MAX_VALUE = (1 << BITS) - 1; /* Note that MS-DOS machines need to   */
    private final int MAX_CODE = MAX_VALUE - 1;    /* compile their code in large model if*/
    /* 14 bits are selected.               */

    private final int TABLE_SIZE = 5021;           /* The string table size needs to be a */
    /* prime number that is somewhat larger*/
    /* than 2**BITS.                       */
    private final int NEXT_CODE = 257;

    private long[] prefix_code = new long[TABLE_SIZE];;        /* This array holds the prefix codes   */
    private int[] append_character = new int[TABLE_SIZE];  /* This array holds the appended chars */
    private int[] decode_stack; /* This array holds the decoded string */

    private int input_bit_count=0;
    private long input_bit_buffer=0; // must be 32 bits
    private int offset = 0;

    /*
    ** This routine simply decodes a string from the string table, storing
    ** it in a buffer.  The buffer can then be output in reverse order by
    ** the expansion program.
    */
    /* JN: returns size of buffer used 
     */
    private int decode_string(int idx, long code)
    {
	int i;

	i=0;
	while (code > (NEXT_CODE - 1))
	    {
		decode_stack[idx++] = append_character[(int) code];
		code=prefix_code[(int) code];
		if (i++>=MAX_CODE)
		    {
			System.err.printf("Fatal error during code expansion.\n");
			return 0;
		    }
	    }

	decode_stack[idx]= (int) code;

	return idx;
    }

    /*
    ** The following two routines are used to output variable length
    ** codes.  They are written strictly for clarity, and are not
    ** particularyl efficient.
    */

    long input_code(byte[] inputBuffer, int inputLength, int dummy_offset, boolean firstTime)
    {
	long return_value;

	//int pOffsetIdx = 0;
	if (firstTime)
	    {
		input_bit_count = 0;
		input_bit_buffer = 0;
	    }

	while (input_bit_count <= 24 && offset < inputLength)
	    {
		/*
		input_bit_buffer |= (long) inputBuffer[offset++] << (24 - input_bit_count);
		input_bit_buffer &= 0xFFFFFFFFL;
		System.out.printf("input buffer %d\n", (long) inputBuffer[offset]);
		*/
		// Java doesn't have unsigned types. Have to play stupid games when mixing
		// shifts and type coercions
		long val = inputBuffer[offset++];
		if (val < 0) {
		    val = 256 + val;
		}
		// System.out.printf("input buffer: %d\n", val);
		//		if ( ((long) inpu) < 0) System.out.println("Byte is -ve???");
		input_bit_buffer |= (((long) val) << (24 - input_bit_count)) & 0xFFFFFFFFL;
		//input_bit_buffer &= 0xFFFFFFFFL;
		// System.out.printf("input bit buffer %d\n", input_bit_buffer);

		/*
		if (input_bit_buffer < 0) {
		    System.err.println("Negative!!!");
		}
		*/

		input_bit_count  += 8;
	    }

	if (offset >= inputLength && input_bit_count < 12)
	    return MAX_VALUE;

	return_value       = input_bit_buffer >>> (32 - BITS);
	input_bit_buffer <<= BITS;
	input_bit_buffer &= 0xFFFFFFFFL;
	input_bit_count   -= BITS;

	return return_value;
    }

    void dumpLyric(int data)
    {
	System.out.printf("LZW: %d\n", data);
	if (data == 0xd)
	    System.out.printf("\n");	      
    }

    /*
    **  This is the expansion routine.  It takes an LZW format file, and expands
    **  it to an output file.  The code here should be a fairly close match to
    **  the algorithm in the accompanying article.
    */

    public int expand(byte[] intputBuffer, int inputBufferSize, byte[] outBuffer)
    {
	long next_code = NEXT_CODE;/* This is the next available code to define */
	long new_code;
	long old_code;
	int character;
	int string_idx;
	
	int offsetOut = 0;


	prefix_code      = new long[TABLE_SIZE];
	append_character = new int[TABLE_SIZE];
	decode_stack     = new int[4000];

	old_code= input_code(intputBuffer, inputBufferSize, offset, true);  /* Read in the first code, initialize the */
	character = (int) old_code;          /* character variable, and send the first */
	outBuffer[offsetOut++] = (byte) old_code;       /* code to the output file                */
	//outTest(output, old_code);
	// dumpLyric((int) old_code);

	/*
	**  This is the main expansion loop.  It reads in characters from the LZW file
	**  until it sees the special code used to inidicate the end of the data.
	*/
	while ((new_code=input_code(intputBuffer, inputBufferSize, offset, false)) != (MAX_VALUE))
	    {
		// dumpLyric((int)new_code);
		/*
		** This code checks for the special STRING+CHARACTER+STRING+CHARACTER+STRING
		** case which generates an undefined code.  It handles it by decoding
		** the last code, and adding a single character to the end of the decode string.
		*/

		if (new_code>=next_code)
		    {
			if (new_code > next_code)
			    {
				System.err.printf("Invalid code: offset:%X new:%X next:%X\n", offset, new_code, next_code);
				break;
			    }

			decode_stack[0]= (int) character;
			string_idx=decode_string(1, old_code);
		    }
		else
		    {
			/*
			** Otherwise we do a straight decode of the new code.
			*/
			string_idx=decode_string(0,new_code);
		    }

		/*
		** Now we output the decoded string in reverse order.
		*/
		character=decode_stack[string_idx];
		while (string_idx >= 0)
		    {
			int data = decode_stack[string_idx--]; 
			outBuffer[offsetOut] = (byte) data;
			//outTest(output, *string--);

			if (offsetOut % 4 == 0) {
			    //dumpLyric(data);
			}

			offsetOut++;
		    }

		/*
		** Finally, if possible, add a new code to the string table.
		*/
		if (next_code > 0xfff)
		    {
			next_code = NEXT_CODE;
			System.err.printf("*");
		    }

		// test code
		if (next_code > 0xff0 || next_code < 0x10f)
		    {
			Debug.printf("%02X ", new_code);
		    }

		prefix_code[(int) next_code]=old_code;
		append_character[(int) next_code] = (int) character;
		next_code++;

		old_code=new_code;
	    }
	Debug.printf("offset out is %d\n", offsetOut);
	return offsetOut;
    }
}