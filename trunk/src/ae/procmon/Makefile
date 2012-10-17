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

.PHONY: all

all: procmon.o
	$(CP) procmon.pl procmon_conf util.pl $(BINDIR)
	$(CM) 755 $(BINDIR)/procmon.pl
	$(CM) 644 $(BINDIR)/procmon_conf
	$(CM) 644 $(BINDIR)/util.pl

procmon.o:
	$(SHA256CHK) procmon.c procmon_sha256.c $(PRODDIR) procmon.pl procmon_conf util.pl
	gcc $(CFLAGS) -I$(INCDIR) -c procmon_sha256.c
	$(CP) procmon_sha256.o procmon.o
	$(CP) procmon.o $(OBJDIR)

.PHONY: clean
clean:
	rm -f *.o
