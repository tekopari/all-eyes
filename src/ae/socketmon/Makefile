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
all: test product

test: socketmon_test.pl socketmon_test.o

product: socketmon_product.pl socketmon_product.o

socketmon_test.pl:
	sudo mkdir -p $(CONFDIR)
	$(CP) socketmon.pl util.pl $(BINDIR)
	sudo $(CP) socketmon_conf $(CONFDIR)
	$(CM) 755 $(BINDIR)/socketmon.pl
	$(CM) 644 $(BINDIR)/util.pl
	sudo $(CM) 644 $(CONFDIR)/socketmon_conf

socketmon_product.pl:
	sudo mkdir -p $(CONFDIR)
	$(CP) socketmon.pl util.pl $(PRODDIR)
	sudo $(CP) socketmon_conf $(CONFDIR)
	$(CM) 755 $(PRODDIR)/socketmon.pl
	$(CM) 644 $(PRODDIR)/util.pl
	sudo $(CM) 644 $(CONFDIR)/socketmon_conf

socketmon_test.o:
	$(SHA256CHK) socketmon.c socketmon_sha256.c ./ -t socketmon.pl util.pl
	gcc $(CFLAGS) -I$(INCDIR) -c socketmon_sha256.c
	$(MV) socketmon_sha256.o $(OBJDIR)/socketmon_test.o

socketmon_product.o:
	$(SHA256CHK) socketmon.c socketmon_sha256.c /bin/ -t socketmon.pl util.pl
	gcc $(CFLAGS) -I$(INCDIR) -DPRODUCTION=1 -c socketmon_sha256.c
	$(MV) socketmon_sha256.o $(OBJDIR)/socketmon_product.o

.PHONY: clean
clean:
	rm -f *.o
	rm -f *_sha256.c
