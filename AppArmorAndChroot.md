There is the 'ae' daemon installation script, named **_install\_chroot.sh_**. This script will create the chroot environment, install the ubuntu OS in the chroot. Then, it installs the 'ae' package and write the 'ae' daemon `AppArmor profile` to proper place and reload the apparmor.

This installation script is part of the tar file named **_ae-daemon.tgz_**.

# What does the install\_chroot.sh do #
  1. Installs the needed packages
    * dchroot
    * debootstrap
  1. Creates the chroot directory
    * /ae/jail
  1. Edit the chroot configuration file /etc/schroot/schroot.conf
    * [precise](precise.md)
    * description=Precise Gangolin
    * location=/ae/jail
    * priority=3
    * users=ae
    * groups=ae
    * root-groups=root
  1. Install the ubuntu OS in the chroot jail
  1. Create the 'ae' package config file directory
    * /etc/ae/certs
    * /etc/ae/exports
  1. Cross mount the following directories to chroot jail as read only
    * /proc
    * /dev/pts
    * /etc/ae/exports
  1. Copy the 'ae' package to the proper directories
    * /usr/local/bin will save the 'ae' daemon
    * /ae/jail/bin will save the monitor files
    * /etc/ae/certs will save the 'ae' daemon certificates
    * /etc/ae/exports will save the monitor configuration files
  1. Copy the 'ae' `AppArmor profile`. This is part of the 'ae' package
    * Copy file **_usr.local.bin.ae_** to directory **_/etc/apparmor.d/_**
  1. Then, drop the user to chroot jail as root, and install the necessary packages on top of the OS. This includes
    * vim
    * net\_tools
    * openssl
  1. Last step is to do ulimit inside the chroot jail. Below is the table of the tunable parameters picked.
| -c 0 | No core file |
|:-----|:-------------|
| -u 1000 | maximum children that this shell can spawn |
| -e 0 | Set the maximum "nice" priority |
| -r 0 | Set the real-time priority |
| -d 524288 | Limit the datasegment to half a gig |
| -i 100 | Limit the filesize written to to 10 Mb |
| -n 50 | Set number of open file descriptors |
| -q 10240 | Set the message queue size |
| -x 20 | Limit the virtual memory available to shell and the programs exec'ed from the shell to 20G |
| -v 2097152 | No file locking |




