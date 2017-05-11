
%.class: %.java
	javac -g -deprecation  -classpath $(CLASSPATH) $<

CLASSPATH = .
CLASSES =  \
	Constants.class \
	CharsetEncoding.class \
	SequenceInformation.class \
	Debug.class \
	Lyric.class \
	MidiPlayer.class \
	SampledPlayer.class \
	KaraokePlayer.class\
	KaraokePlayerSampled.class\
	SongInformation.class \
	LyricLine.class \
	AttributedLyricPanel.class \
	DurationNote.class \
	MelodyPanel.class \
	PianoPanel.class \
	MidiGUI.class \
	DisplayReceiver.class \




all: $(CLASSES) jar
	zip all.zip *.java

jar: $(CLASSES)
	jar cfm KaraokePlayer.jar Manifest.txt *.class

clean:
	rm *.class
