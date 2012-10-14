#
# Copyright (C) <2012> <Blair Wolfinger, Ravi Jagannathan, Thomas Pari, Todd Chu>
#
# Permission is hereby granted, free of charge, to any person obtaining a copy of this
# software and associated documentation files (the "Software"), to deal in the Software
# without restriction, including without limitation the rights to use, copy, modify,
# merge, publish, distribute, sublicense, and/or sell copies of the Software, and to
# permit persons to whom the Software is furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in all copies
# or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,
# INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR
# PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR
# ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
# ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
#
# Original Author: Ravi Jagannathan
#
include ../Makefile.include

INCDIR += -I.

.PHONY: all
all:  ae

ae: ae.c aeconf.c ../include/ae.h
	gcc $(CFLAGS) -I$(INCDIR) -c ae.c
	gcc $(CFLAGS) -I$(INCDIR) -c selfmon.c
	gcc $(CFLAGS) -o ae ae.o  selfmon.o $(OBJDIR)/*.o
	cp ae $(BINDIR)

#
# Remove the binaries.
.PHONY: clean
clean:
	(set -e; for DIR in $(DIRECTORIES); do pushd $$DIR; make clean; popd; done)
	rm -f *.o ae

