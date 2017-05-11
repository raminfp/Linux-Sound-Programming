############################################################################
# 'A Generic Makefile for Building Multiple main() Targets in $PWD'
# Author:  Robert A. Nader (2012)
# Email: naderra at some g
# Web: xiberix
############################################################################
#  The purpose of this makefile is to compile to executable all C source
#  files in CWD, where each .c file has a main() function, and each object
#  links with a common LDFLAG.
#
#  This makefile should suffice for simple projects that require building
#  similar executable targets.  For example, if your CWD build requires
#  exclusively this pattern:
#
#  cc -c $(CFLAGS) main_01.c
#  cc main_01.o $(LDFLAGS) -o main_01
#
#  cc -c $(CFLAGS) main_2..c
#  cc main_02.o $(LDFLAGS) -o main_02
#
#  etc, ... a common case when compiling the programs of some chapter,
#  then you may be interested in using this makefile.
#
#  What YOU do:
#
#  Set PRG_SUFFIX_FLAG below to either 0 or 1 to enable or disable
#  the generation of a .exe suffix on executables
#
#  Set CFLAGS and LDFLAGS according to your needs.
#
#  What this makefile does automagically:
#
#  Sets SRC to a list of *.c files in PWD using wildcard.
#  Sets PRGS BINS and OBJS using pattern substitution.
#  Compiles each individual .c to .o object file.
#  Links each individual .o to its corresponding executable.
#
###########################################################################
#
PRG_SUFFIX_FLAG := 0
#
FLUID=../../MIDI/FluidSynth/fluidsynth-1.1.6-new/
LDFLAGS :=   -L$(FLUID)/src/.libs -l fluidsynth -l glib-2.0 `pkg-config --libs gtk+-3.0  libavformat libavcodec libavutil libswscale` -lpthread -lX11
CFLAGS_INC :=   -I $(FLUID)/include/ -I $(FLUID)/src/midi/ -I $(FLUID)/src/utils/  `pkg-config --cflags gtk+-3.0`
CFLAGS := -g -Wall $(CFLAGS_INC)
#
## ==================- NOTHING TO CHANGE BELOW THIS LINE ===================
##
SRCS := $(wildcard *.c)
PRGS := $(patsubst %.c,%,$(SRCS))
PRG_SUFFIX=.exe
BINS := $(patsubst %,%$(PRG_SUFFIX),$(PRGS))
## OBJS are automagically compiled by make.
OBJS := $(patsubst %,%.o,$(PRGS))
##
all : $(BINS) zip
##
## For clarity sake we make use of:
.SECONDEXPANSION:
OBJ = $(patsubst %$(PRG_SUFFIX),%.o,$@)
ifeq ($(PRG_SUFFIX_FLAG),0)
		BIN = $(patsubst %$(PRG_SUFFIX),%,$@)
else
		BIN = $@
endif
## Compile the executables
%$(PRG_SUFFIX) : $(OBJS)
	$(CC) $(OBJ)  $(LDFLAGS) -o $(BIN)
##
## $(OBJS) should be automagically removed right after linking.
##
veryclean:
ifeq ($(PRG_SUFFIX_FLAG),0)
	$(RM) $(PRGS)
else
	$(RM) $(BINS)
endif
##
rebuild: all
##
zip:
	zip all.zip $(SRCS)
## eof Generic_Multi_Main_PWD.makefile
