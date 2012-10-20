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
# File Name: install_chroot.sh
# Description:
#    Run this script to create a chroot environment under ubuntu OS install. After the chroot
#    is created, it will install the necessary packages including the All-Eyes' processes.
#
#    In order to successfully run the script, please do:
#        (1) Issue command 'make'
#        (2) Go to directory '<your_workspace>/all-eyes/src/ae/bin'
#        (3) Issue command './install_chroot.sh'
#
########################################################################################################
clear

jail_dir=/ae/jail
user=ae
install_pkg_script=get_pkg.sh

echo "***** CREATE CHROOT JAIL AT DIR $jail_dir *****"

echo "***** Install the required package ..."
sudo apt-get install dchroot
sudo apt-get install debootstrap

echo "***** Create the directory to be the jail ..."
sudo mkdir -p $jail_dir

echo "***** Edit chroot config file ..."
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

echo "***** Install the OS into the jail ..."
package_site=http://mirrors.rit.edu/ubuntu
sudo debootstrap --variant buildd --arch i386 precise $jail_dir $package_site

echo "***** Cross mount the directory /proc ..."
sudo mount -o bind /proc $jail_dir/proc

#echo "***** Cross mount the directory /dev/pts ..."
#sudo mount -o bind /dev/pts $jail_dir/dev/pts

echo "***** Create a user for chroot ..."
sudo useradd $user
sudo mkdir -p $jail_dir/home/$user
sudo chown $user:$user $jail_dir/home/$user

echo "***** Add lines at the end of script $install_pkg_script ..."
sudo echo "echo \"*********************************************************\"" >> $install_pkg_script
sudo echo "echo \"***  TO exit the chroot, please issue command 'exit'  ***\"" >> $install_pkg_script
sudo echo "echo \"*********************************************************\"" >> $install_pkg_script
sudo echo "rm -f /bin/$install_pkg_script" >> $install_pkg_script

echo "***** Copy All-Eyes files to chroot's /bin ..."
src_dir=.
des_dir=$jail_dir/bin
sudo mkdir -p $des_dir
for file in $src_dir/*
do
   if [ "$file" != "$0" ]
   then
      sudo cp $file $des_dir/.
      perm=$(sudo stat -c "%a" $file)
      sudo chown $user:$user $file
      sudo chmod $perm $file
   fi
done

echo ""
echo "***********************************************"
echo "***  YOU ARE NOW UNDER CHROOT AS ROOT USER"
echo "***"
echo "***  Please type the following command to"
echo "***  install the packages inside chroot:"
echo "***       $install_pkg_script"
echo "***       exit"
echo "***********************************************"
sudo chroot $jail_dir

