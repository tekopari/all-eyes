#!/bin/bash

#
# INSTALL THE AEPROXY SERVICE
#

sudo /bin/chown root:root /etc/ae/certs/jssecacerts
sudo /bin/chmod 744 /etc/ae/certs/jssecacerts

sudo /bin/chown root:root /etc/ae/certs/keystore.jks
sudo /bin/chmod 744 /etc/ae/certs/keystore.jks

sudo /bin/chown root:root /usr/local/bin/aeproxy
sudo /bin/chmod 6755 /usr/local/bin/aeproxy

sudo /bin/chown root:root /usr/local/bin/AeProxy.jar
sudo /bin/chmod 755 /usr/local/bin/AeProxy.jar

sudo /bin/chown root:root /etc/init.d/aeproxyd
sudo /bin/chmod 755 /etc/init.d/aeproxyd

sudo /bin/ln -s -f /etc/init.d/aeproxyd /etc/rc0.d/K99aeproxyd
sudo /bin/ln -s -f /etc/init.d/aeproxyd /etc/rc1.d/K99aeproxyd
sudo /bin/ln -s -f /etc/init.d/aeproxyd /etc/rc2.d/S99aeproxyd
sudo /bin/ln -s -f /etc/init.d/aeproxyd /etc/rc3.d/S99aeproxyd
sudo /bin/ln -s -f /etc/init.d/aeproxyd /etc/rc4.d/S99aeproxyd
sudo /bin/ln -s -f /etc/init.d/aeproxyd /etc/rc5.d/S99aeproxyd
sudo /bin/ln -s -f /etc/init.d/aeproxyd /etc/rc6.d/K99aeproxyd

