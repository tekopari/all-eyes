#!/bin/bash
########################################################################################################
# copyright (C) <2012> <Blair Wolfinger, Ravi Jagannathan, Thomas Pari, Todd Chu>
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
########################################################################################################
#
# File Name: conf_limit.sh
# Description:
#     This script is called by "setup_ae.sh". The purpose of this script is
#     to setup the system resource limit
#
########################################################################################################

echo "*** Set ulimit ..."

#################################################
# No core file; better test the program before
# it gets jailed for deployment.
#################################################
ulimit -c 0

#################################################
# Maximum children that this shell can spawn.
#################################################
ulimit -u 1000

#################################################
# Set the maximum "nice" priority and real-time
# priority to be 0 (zero), the default value.
# Maximum size of data segment, in Kbytes, even
# though OOM will catch it.
#################################################
ulimit -e 0
ulimit -r 0

#################################################
# Limit the datazegment to half a gig, 50% of
# the VM size in which Ubuntu is running.
#################################################
ulimit -d 524288

#################################################
# Limit the filesize written to  10 Mb.  The below
# number is in 512 byte blocks ulimit -f 2000
# Reduce the number of pending signals to 50 as
# somebody keep on sending signals there by making
# us vulnerable to just processing signals.
#################################################
ulimit -i 100

#################################################
# Reduce the number of open file descriptors
# ulimit -n 50 # Reduce the message queue size
# as our Turing machine doesn't use messages.
# Why waste memory?
#################################################
ulimit -q 10240

#################################################
# Limit the virtual memory available to shell
# and the programs exec'ed from the shell to
# 2G ulimit -v 2097152 # No file locking.
#################################################
ulimit -x 20

