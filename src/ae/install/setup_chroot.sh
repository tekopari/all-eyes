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
user=ae
echo "*** CREATE CHROOT JAIL AT DIR $jail_dir ***"

echo "*** Install the required package ..."
sudo apt-get install dchroot
sudo apt-get install debootstrap

echo "*** Create the directory to be the jail ..."
sudo mkdir -p $jail_dir

echo "*** Edit chroot config file ..."
conf_file=/etc/schroot/schroot.conf
sudo chmod 666 $conf_file
sudo echo "[precise]" >> $conf_file
sudo echo "description=Precise Gangolin" >> $conf_file
sudo echo "location=$jail_dir" >> $conf_file
sudo echo "priority=3" >> $conf_file
sudo echo "users=ae" >> $conf_file
sudo echo "groups=ae" >> $conf_file
sudo echo "root-groups=root" >> $conf_file
sudo chmod 644 $conf_file

echo "*** Install the OS into the jail ..."
package_site=http://mirrors.rit.edu/ubuntu
sudo debootstrap --variant buildd --arch i386 precise $jail_dir $package_site

echo "*** Cross mount the /proc directory ..."
sudo mount -o bind /proc $jail_dir/proc

echo "*** Create a user for chroot ..."
sudo useradd $user
sudo mkdir -p $jail_dir/home/$user
sudo chown $user:$user $jail_dir/home/$user

echo "*** Create shellcript for executing apt-get under chroot ..."
cmd="apt-get install"
tmp_script=myscript.sh
echo $cmd vim > $tmp_script
echo $cmd net-tools >> $tmp_script
echo $cmd openssl >> $tmp_script
echo $cmd apparmor >> $tmp_script
echo $cmd apparmor-profiles >> $tmp_script
echo rm $tmp_script >> $tmp_script

echo "*** Move shellcript to chroot ..."
sudo mv $tmp_script $jail_dir/
sudo chown root:root $jail_dir/$tmp_script

echo ""
echo "***********************************************"
echo "***  YOU ARE NOW UNDER CHROOT AS ROOT USER  ***"
echo "***                                         ***"
echo "***  Please type the following command to   ***"
echo "***  install the packages inside chroot:    ***"
echo "***       chmod +x $tmp_script              ***"
echo "***       ./$tmp_script                     ***"
echo "***       exit                              ***"
echo "***********************************************"
sudo chroot $jail_dir

