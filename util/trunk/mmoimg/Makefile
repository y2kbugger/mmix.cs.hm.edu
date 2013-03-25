#
#   Makefile for mmoimg
#

# you may want to adjust these variables

# where are the mmixware sources
MMIXDIR = ../../../mmixware/trunk

# where to install files
INSTALLDIR = ../../../../html

#CFLAGS =  -O0  -Wall 
CFLAGS = -O3  -Wall 
CC = gcc

ifdef OSX
# making fat binaries 
CFLAGS = -O2 -force_cpusubtype_ALL -mmacosx-version-min=10.4 -arch x86_64 -arch i386 -arch ppc
CC = gcc
endif

# probably no changes are needed below

INCDIRS = -I $(MMIXDIR)


all: bin doc

bin: .depend mmoimg
doc: .depend mmoimg.pdf

install: all
	cp mmoimg $(INSTALLDIR)/bin
	cp mmoimg.pdf $(INSTALLDIR)/doc
	if test -x mmoimg.exe; then cp mmoimg.exe $(INSTALLDIR)/exe; fi


clean:
	rm -f *~ *.bak
	rm -f *.log *.toc *.scn *.idx *.o
	rm -f mmoimg mmoimg.exe mmoimg.tex mmoimg.pdf mmoimg.c 
	rm -f boilerplate.w

boilerplate.w: $(MMIXDIR)/boilerplate.w
	cp $(MMIXDIR)/boilerplate.w .

mmoimg.c: $(MMIXDIR)/mmotype.w mmoimg.ch boilerplate.w
	ctangle $(MMIXDIR)/mmotype.w mmoimg.ch mmoimg.c

mmoimg.tex:  $(MMIXDIR)/mmotype.w mmoimg.ch
	cweave  $(MMIXDIR)/mmotype.w mmoimg.ch mmoimg.tex

mmoimg.pdf: mmoimg.tex
	pdftex mmoimg.tex
	pdftex mmoimg.tex

mmoimg: mmoimg.c
	$(CC) $(CFLAGS) mmoimg.c -o mmoimg

TAGS:
	etags *.c *.h $(MMIXDIR)/*.c $(MMIXDIR)/*.h

.depend: mmoimg.c
	$(CC) $(INCDIRS) -MM *.c > .depend

include .depend