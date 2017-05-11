
%.class: %.java
	javac -g -deprecation  -classpath $(CLASSPATH) $<

CLASSPATH = .
CLASSES =  \
	DeviceInfo.class \
	DumpReceiver.class \
	DumpSequence.class \
	SimpleMidiPlayer.class \



all: $(CLASSES)


clean:
	rm *.class
