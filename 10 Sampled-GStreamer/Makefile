

# Requires libgstreamer1.0-dev but as at 27/10/16 Ubuntu repository
# is broken as it doesn't have
# https:http://archive.ubuntu.com/ubuntu/pool/main/g/gstreamer1.0/libgstreamer1.0-dev_1.8.1-1~ubuntu1_amd64.deb
# 1.8.0 -yes, 1.8.2 -yes, but not 1.8.1

# Guessing...:
LDLIBS = $(shell pkg-config --libs libgstreamer)
CFLAGS = -g $(shell pkg-config --cflags libgstreamer)

SRC = basic-tutorial-3.c


EXE = basic-tutorial-3


all: $(EXE)


clean:
	rm $(EXE)

zip:
	zip all.zip Makefile $(SRC)
