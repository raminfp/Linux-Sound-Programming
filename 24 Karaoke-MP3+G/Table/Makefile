
%.class: %.java
	javac -g -deprecation  -classpath $(CLASSPATH) $<

CLASSPATH = .
CLASSES =  \
	SongInformation.class \
	SongTable.class \
	Favourites.class \
	AllFavourites.class \
	SongTableSwing.class \

all: $(CLASSES) jar zip

jar: $(CLASSES)
	jar cfm SongTable.jar Manifest.txt $(shell echo '*.class')

zip:
	zip all.zip *.java

clean:
	rm *.class
