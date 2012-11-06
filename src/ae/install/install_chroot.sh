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
remote_install_script=setup_ae.sh

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

echo "***** Create directory /etc/ae/ and file rc.ae ..."
sudo mkdir -p /etc/ae/certs
sudo mkdir -p /etc/ae/exports
sudo echo "#!/bin/bash" > rc.ae

echo "***** Cross mount the directory /proc and make it read only ..."
sudo /bin/umount $jail_dir/proc
sudo /bin/mount -o bind,ro /proc $jail_dir/proc
sudo echo "/bin/mount -o bind,ro /proc $jail_dir/proc" >> rc.ae

echo "***** Cross mount the directory /dev/pts ..."
sudo /bin/umount $jail_dir/dev/pts
sudo /bin/mount -o bind,ro /dev/pts $jail_dir/dev/pts
sudo echo "/bin/mount -o bind,ro /dev/pts $jail_dir/dev/pts" >> rc.ae

echo "***** Cross mount the directory /etc/ae/exports ..."
sudo mkdir -p $jail_dir/etc/ae/exports
sudo /bin/umount $jail_dir/etc/ae/exports
sudo /bin/mount -o bind,ro /etc/ae/exports $jail_dir/etc/ae/exports
sudo echo "/bin/mount -o bind,ro /etc/ae/exports $jail_dir/etc/ae/exports" >> rc.ae

echo "***** Modify the /etc/rc.local to include rc.ae ..."
sudo sed 's/^exit 0/\/etc\/ae\/rc.ae; exit 0/' /etc/rc.local > rc.local
sudo chmod 755 rc.ae
sudo chmod 755 rc.local
sudo mv rc.ae /etc/ae/rc.ae
sudo mv rc.local /etc/rc.local

echo "***** Create a user for chroot ..."
sudo useradd $user
sudo mkdir -p $jail_dir/home/$user
sudo chown $user:$user $jail_dir/home/$user

echo "***** Add lines at the end of script $remote_install_script ..."
sudo echo "echo \"\"" >> $remote_install_script
sudo echo "echo \"***************************************************************\"" >> $remote_install_script
sudo echo "echo \"***  Please issue command 'exit' to exit the chroot. Then,  ***\"" >> $remote_install_script
sudo echo "echo \"***  enter the following command to start the ae daemon     ***\"" >> $remote_install_script
sudo echo "echo \"***         sudo /usr/local/bin/ae -a -p                    ***\"" >> $remote_install_script
sudo echo "echo \"***************************************************************\"" >> $remote_install_script
sudo echo "echo \"\"" >> $remote_install_script
sudo echo "rm -f /bin/$remote_install_script" >> $remote_install_script

echo "***** Copy ae executables to /usr/local/bin and chroot's /bin ..."
sudo cp ae /usr/local/bin/.
sudo cp filemonConfig /usr/local/bin/.
src_dir=.
des_dir=$jail_dir/bin
sudo mkdir -p $des_dir
for file in $src_dir/*
do
   if [ "$file" != "$0" ] && [ "$file" != "$src_dir/read_me_first" ] && [ "$file" != "$src_dir/AppArmor_Profiles" ] && [ "$file" != "$src_dir/AeCerts" ] && [ "$file" != "$src_dir/MonConfig" ] && [ "$file" != "$src_dir/ae" ] && [ "$file" != "$src_dir/filemonConfig" ]
   then
      sudo cp $file $des_dir/.
      perm=$(sudo stat -c "%a" $file)
      sudo chown $user:$user $des_dir/$file
      sudo chmod $perm $des_dir/$file
   fi
done

echo "***** Copy Monitor config files to base kernel /etc/ae/exports ..."
src_dir=MonConfig
des_dir=/etc/ae/exports
sudo cp $src_dir/* $des_dir/.

echo "***** Copy ae-daemon cert files to base kernel /etc/ae/certs ..."
src_dir=AeCerts
des_dir=/etc/ae/certs
sudo cp $src_dir/* $des_dir/.

echo "***** Copy AppArmor Profiles to base kernel /etc/apparmor.d/ ..."
src_dir=AppArmor_Profiles
des_dir=/etc/apparmor.d
sudo cp $src_dir/* $des_dir/.

echo "***** Reload AppArmor Profiles ..."
sudo invoke-rc.d apparmor reload

echo ""
echo "***********************************************"
echo "***  YOU ARE NOW UNDER CHROOT AS ROOT USER"
echo "***"
echo "***  Please type the following command to"
echo "***  install the packages inside chroot:"
echo "***       /bin/$remote_install_script"
echo "***       exit"
echo "***********************************************"
echo ""
sudo chroot $jail_dir

