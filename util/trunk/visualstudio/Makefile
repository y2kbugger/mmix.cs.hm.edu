#
#   Makefile for windos executables using MS Visual Studio
#


# where to install files
INSTALLDIR = ../../../../html

MMIXWAREEXE= mmix.exe mmmix.exe mmixal.exe mmotype.exe 

all: bin pdf

bin:
	for file in $(MMIXWAREEXE); do \
	  if test -r Release/$$file ; then \
	    cp Release/$$file $(INSTALLDIR)/exe ; \
	  else \
	    echo $$file missing ; \
	  fi \
	done

pdf:
	echo no documents here

clean:

