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
# File Name: install_alleyes.sh
# Description:
#    Run this script to install the tar file ae-and-proxy.tgz. This includes the following steps
#        (1) untar the file ae-and-proxy.tgz
#        (2) untar the sub-tar files
#        (3) travels each directory and install files
########################################################################################################
clear
mydir=`pwd`

echo  "****** set iptables rule ******"
sudo iptables -A INPUT -p tcp -s localhost --dport 6000 -j ACCEPT
sudo iptables -A INPUT -p tcp --dport 6000 -j DROP

echo  "****** Untab ae-and-proxy.tgz ..."
tar xvfz ae-and-proxy.tgz

echo "****** Install AeProxy ..."
cd /
sudo tar xvf ${mydir}/aeproxy.tar
sudo /etc/init.d/install_aeproxy.sh
sudo rm -f /etc/init.d/install_aeproxy.sh
sudo service aeproxyd start

echo "****** Install ae daemon ..."
cd  ${mydir}
tar xvfz ae-daemon.tgz
cd  ${mydir}/product
./install_chroot.sh
exit 0
