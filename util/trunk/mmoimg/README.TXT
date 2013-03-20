mmoimg - converting MMIX object files to image files

This page describes mmoimg, a modification of mmotype that can be used
to produce image files from mmo files.

Last update: Dec. 8, 2011 What is an mmo file?  The MMIXware
assembler, mmixal, produces MMIX object files, short mmo files, in a
special format which the instruction level simulator, mmix, can read
directly into memory.  What is an image file?  When the MMIX simulator
loads an mmo file into its virtual memory, it converts an mmo file
into a string of 264 byte, the memory image. The complete memory image
is too large to fit into a file of reasonable size, but fortunately,
most of the bytes in memory are zero. Hence, it is usually sufficient
to store only the important parts of the memory image in an image
file. The program mmoimg will consider only bytes that are specified
in the mmo file as important. Further, it will restrict the image file
to the memory range between a given base address and an upper bound.
How to use mmoimg?  The simplest use of mmoimg is like this:

mmoimg  file.mmo

This will produce file.img from file.mmo containing the memory image
created by file.mmo in the negative address range starting at 0x8000
0000 0000 0000.

The restriction to the negative address range is natural for MMIX
since negative addresses are mapped directly to physical addresses (by
ignoring the sign bit). Therefore the default base address is 0x8000
0000 0000 0000 and the default upper bound is 0xFFFF FFFF FFFF FFFF.

A different base address and upper bound can be specified on the
command line like this:

mmoimg -b 0 -u 2000000000000000 file.mmo

This will produce an image containing just the text segment of
MMIX. The MMIX text segment starts at address 0 and ends at
1FFFFFFFFFFFFFFF, just before the beginning of the data segment.

Still, the image file will not stretch all the way up from the base
address to the beginning of the data segment, but will contain only
those few bytes of the text segment that are specified in the mmo
file. It will, however, contain zero bytes from the given base address
to the first byte specified in the mmo file. This padding at the
beginning is useful to have an image file with a fixed start address
independent of the mmo file. There is no padding at the end of the
image file.

To get some more information about the mmo file, the options -l
(listing) and -v (verbose) can be used to get (plenty) of output while
mmoimg is running.  Example In some situations, it is desirable to use
the MMIX assembly language to specify code and data which is not
intended to be loaded into memory using the simulator. In these
situations, an image file, containing an exact image of the virtual
memory is often a more convenient format than the mmo file itself.

For example, the ROM simulator of the virtual motherboard project
(VMB) can be used to map any binary file at any address into
memory. To produce such a binary file containing MMIX code and data,
you can write a mmix source file, assemble it with mmixal into an mmo
file and then convert it to a binary image file using mmoimg. This is
described in the VMB Hello World example.  Sources mmoimg is written
as a change file mmoimg.ch. It is applied to mmotype.w like this:

ctangle mmotype.w mmoimg.ch mmoimg.c

to obtain mmoimg.c as output file. The file mmoimg.tex is produced in
the same way with cweave.  Binaries Executable Files can be found
here:

    * Win32 Executables
    * Linux Binaries
    * MAC OSX Executables

Documentation mmoimg is written using literate program. Hence, its
full documentation is itself: mmoimg.pdf.