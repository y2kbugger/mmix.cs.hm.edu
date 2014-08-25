#
#   Makefile for mmoimg
#

# you may want to adjust these variables

# where are the mmixware sources
MMIXDIR = ../../../mmixware/trunk

# where to install files
INSTALLDIR = ../../../../html
CC = gcc

ifeq "$(OS)" "OSX"
# making fat binaries 
CFLAGS = -O2 -force_cpusubtype_ALL -mmacosx-version-min=10.4 -arch x86_64 -arch i386 -arch ppc
BINDIR=$(INSTALLDIR)/osx
endif

ifeq "$(OS)" "LINUX32"
CFLAGS = -O3 -static
BINDIR=$(INSTALLDIR)/bin
endif

# probably no changes are needed below

INCDIRS = -I $(MMIXDIR)


all: bin pdf

MMIXWAREEXE= mmoimg.exe

bin: .depend mmoimg
	for file in $(MMIXWAREEXE); do \
	  if test -r Release/$$file ; then \
	    cp Release/$$file $(INSTALLDIR)/exe ; \
	  else \
	    echo $$file missing ; \
	  fi \
	done

pdf: .depend mmoimg.pdf

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
	cp mmoimg.pdf $(INSTALLDIR)/doc

mmoimg: mmoimg.c
	$(CC) $(CFLAGS) mmoimg.c -o $(BINDIR)/mmoimg

TAGS:
	etags *.c *.h $(MMIXDIR)/*.c $(MMIXDIR)/*.h

.depend: mmoimg.c
	$(CC) $(INCDIRS) -MM *.c > .depend

include .depend
