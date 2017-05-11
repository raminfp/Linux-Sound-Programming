
%.class: %.java
	javac -g -deprecation  -classpath $(CLASSPATH) $<

CLASSPATH = .
CLASSES =  \
	Debug.class \
	SongInformation.class \
	LZW.class \
	SongExtracter.class \
	SongTable.class \
	SongTableSwing.class \
	KARConverter.class \


all: $(CLASSES)


clean:
	rm *.class
