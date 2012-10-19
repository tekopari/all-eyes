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
# Original Author: Todd Chu
#
include ../Makefile.include

.PHONY: all

#The below line is for debugging
#CFLAGS=-I. -ggdb -Wall -D_FILE_OFFSET_BITS=64
CFLAGS=-Wall -D_FILE_OFFSET_BITS=64

LIBS=

#Uncomment the line below to compile on Mac
#LIBS=-liconv

aescrypt: aescrypt.o aes.o sha256.o password.o
	$(CC) $(CFLAGS) $(LIBS) -o $@ $^
	$(CP) aescrypt $(BINDIR)
	$(CM) 755 $(BINDIR)/aescrypt

test:
	@$(CC) -DTEST -o sha.test sha256.c
	@./sha.test
	@#rm sha.test
	@$(CC) -DTEST -o aes.test aes.c
	@./aes.test
	@#rm aes.test

testfile: aescrypt
	@echo "testing this 123..." > test.orig.txt
	./aescrypt -e -p "praxis" test.orig.txt
	@cp test.orig.txt.aes test.txt.aes
	./aescrypt -d -p "praxis" test.txt.aes
	cmp test.orig.txt test.txt

%.o: %.c %.h
	$(CC) $(CFLAGS) -c $*.c

install: aescrypt
	install -o root -g root -m 755 aescrypt /usr/bin

uninstall:
	rm -f ../bin/aescrypt

clean:
	rm -f *.o aescrypt test* *test

