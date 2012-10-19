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
# File Name: setup_chroot.sh
# Description:
#    Run this script to create a chroot environment under ubuntu OS install. After the chroot
#    is created, it will install the necessary packages for running the All-Eyes processes.
########################################################################################################
clear

jail_dir=/ae/jail
echo "*** CREATE CHROOT JAIL AT DIR $jail_dir ***"

echo "*** Install the required package ..."
sudo adp-get install dchroot
sudo adp-get install debootstrap

echo "*** Create the directory to be the jail ..."
sudo mkdir -p $jail_dir

echo "*** Edit chroot config file ..."
conf_file=/etc/schroot/schroot.conf
sudo echo "[precise]" >> $conf_file
sudo echo "description=Precise Gangolin" >> $conf_file
sudo echo "location=$jail_dir" >> $conf_file
sudo echo "priority=3" >> $conf_file
sudo echo "users=ae" >> $conf_file
sudo echo "groups=ae" >> $conf_file
sudo echo "root-groups=root" >> $conf_file

echo "*** Install the OS into the jail ..."
package_site=http://mirrors.rit.edu/ubuntu
sudo debootstrap --variant buildd --arch i386 precise $jail_dir $package_site

echo "*** Cross mount the /proc directory ..."
sudo mount -o bind /proc $jail_dir/proc

echo "*** Create a user for chroot ..."
sudo useradd ravi
sudo mkdir -p $jail_dir/home/ae
sudo chown ae:ae $jail_dir/home/ae 

echo "*** Let's go inside the chroot ..."
sudo chroot $jail_dir

echo "*** Install necessary packages under the chroot ..."
apt-get install vim
apt-get install net-tools
apt-get install openssl
apt-get install apparmor
apt-get install apparmor-profiles 

echo "*** Done!"

