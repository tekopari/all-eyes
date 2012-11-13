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

######################################################################################################
# Description:
#    This makefile will call the make files under ../ae and ../proxy. It generates the tar file for 
#    the 'ae' daemon and the AeProxy.
######################################################################################################

SHELL := /bin/bash

DIRS = ae proxy/AeProxy/src

.PHONY: all
all: clean package install

package:
	(set -e; for DIR in $(DIRS); do pushd $$DIR; make package; popd; done)
	cp ae/ae-daemon.tgz .
	cp proxy/AeProxy/aeproxy.tar .
	tar cvfz ae-and-proxy.tgz ae-daemon.tgz aeproxy.tar
	rm ae-daemon.tgz
	rm aeproxy.tar

install:
	./install_ae-and-proxy.sh

# Remove the .o files.
.PHONY: clean
clean: 
	rm -f ae-and-proxy.tgz
	rm -f ae-daemon.tgz
	rm -f aeproxy.tar
	rm -rf product

