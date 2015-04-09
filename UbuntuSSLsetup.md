# openssl-dev and libcap-dev install on Ubuntu 12-04 #

Usually openssl-1.0.0 is installed with the Ubuntu installation.  If openssl-1.0.0 is not installed on your system, go to "Ubuntu Software Center" and install it.  You can use dpkg command to check whether openssl is installed by grep'ing the output.

Execute "dpkg --get-selections | grep libssl-dev" to check whether libssl-dev is installed.  If you do not have libssl-dev installed, you can install it using the command "sudo apt-get install libssl-dev".  During the installation process, it will warn that zlib1g-dev is not authenticated.  Type 'y' to continue to install.

After installation, make sure you have installed libssl-dev by executing the command "dpkg --get-selections | grep libssl-dev".


## _Below is an example:_ ##

**Sample output of apt-get of libssl-dev**

homedir$ sudo apt-get install libssl-dev

Reading package lists... Done
Building dependency tree
Reading state information... Done
The following packages were automatically installed and are no longer required:
> linux-headers-3.2.0-29 linux-headers-3.2.0-29-generic-pae
Use 'apt-get autoremove' to remove them.
The following extra packages will be installed:
> libssl-doc zlib1g-dev
The following NEW packages will be installed:
> libssl-dev libssl-doc zlib1g-dev
0 upgraded, 3 newly installed, 0 to remove and 49 not upgraded.
Need to get 2,616 kB of archives.
After this operation, 6,762 kB of additional disk space will be used.
Do you want to continue [Y/n]? **y**

WARNING: The following packages cannot be authenticated!
> zlib1g-dev
Install these packages without verification [y/N]? y
Get:1 http://us.archive.ubuntu.com/ubuntu/ precise/main zlib1g-dev i386 1:1.2.3.4.dfsg-3ubuntu4 [kB](162.md)
Get:2 http://us.archive.ubuntu.com/ubuntu/ precise-updates/main libssl-dev i386 1.0.1-4ubuntu5.5 [1,420 kB]
Get:3 http://us.archive.ubuntu.com/ubuntu/ precise-updates/main libssl-doc all 1.0.1-4ubuntu5.5 [1,034 kB]
Fetched 2,616 kB in 4s (614 kB/s)
Selecting previously unselected package zlib1g-dev.

(Reading database ... 224027 files and directories currently installed.)

Unpacking zlib1g-dev (from .../zlib1g-dev\_1%3a1.2.3.4.dfsg-3ubuntu4\_i386.deb) ...

Selecting previously unselected package libssl-dev.

Unpacking libssl-dev (from .../libssl-dev\_1.0.1-4ubuntu5.5\_i386.deb) ...

Selecting previously unselected package libssl-doc.

Unpacking libssl-doc (from .../libssl-doc\_1.0.1-4ubuntu5.5\_all.deb) ...

Processing triggers for man-db ...

Setting up zlib1g-dev (1:1.2.3.4.dfsg-3ubuntu4) ...

Setting up libssl-dev (1.0.1-4ubuntu5.5) ...

Setting up libssl-doc (1.0.1-4ubuntu5.5) ...

## Linux Capabilities Setup ##
'ae' daemon also uses Linux capabilities to not to operate as root.  To install this feature on Ubuntu 12.04, install libpcap-dev package using the command _apt-get install libcap-dev_.