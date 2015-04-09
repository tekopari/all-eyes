# Android Development Environment Setup #

**NOTE1:**  Developers must use the eclipse version mentioned below.  Please do not use the eclipse installed using "Ubuntu Software Center".  We used "Juno Service Release 1" versin of Eclipse Java EE IDE for our development.

**NOTE2:**  Building and bringing up Android aeManager on a Ubuntu Virtual Machine could be challenging.  During the development, we had better luck developing and running Android aeManager application on Ubuntu directly running on X86 hardware.

To setup the android development environment you need to:

## Download and install the Android SDK ##
  1. sudo apt-get install openjdk-6-jre openjdk-6-jdk icedtea6-plugin
  1. wget http://dl.google.com/android/android-sdk_r20-linux.tgz
  1. tar -xvzf android-sdk\_r20-linux.tgz

## Update the Android SDK ##
  1. cd ~/android-sdk-linux/tools
  1. ./android
  1. Install any Android updates for Android 4.1 (if there are any available)
  1. After it finished close the Android SDK Manager then restart it
  1. Install any Android updates that might have been missed

## Setup your environment ##
  1. Edit the .bashrc script in your home directory
  1. Add the following: export PATH=${PATH}:~/android-sdk-linux/tools
  1. Add the following: export PATH=${PATH}:~/android-sdk-linux/platform-tools
  1. Log out of your account
  1. Log back in

## Download and install eclipse ##
  1. Navigate to http://www.eclipse.org/downloads/
  1. Eclipse IDE for Java EE Developers (32 bit)
  1. Save the file
  1. Extract the software to your home directory with the command "tar –xvzf eclipse-jee-juno-SR1-linux-gtk.tgz"
  1. cd eclipse
  1. Start eclipse with the command "./eclipse &"

## Download the ADT Plugin ##
  1. Start Eclipse, then select Help > Install New Software.
  1. Click Add, in the top-right corner.
  1. In the Add Repository dialog enter "ADT Plugin" for the Name
  1. In the Add Repository dialog enter https://dl-ssl.google.com/android/eclipse/ for the Location.
  1. Click OK.
  1. Select the checkbox next to Developer Tools and click Next.
  1. In the Install dialog Click Next.
  1. Read and accept the license agreements, then click Finish.
  1. Restart eclipse when it asks
  1. Once Eclipse restarts, you must specify the location of your Android SDK directory. Select Use existing SDKs.
  1. Browse to the Android SDK you installed in your home directory.
  1. Click Next.

## Check for updates ##
  1. Select Help then Check for Updates.
  1. If there are no updates available, a dialog will say so.
  1. If there are updates available, select the updates then click Next.
  1. In the Update Details dialog, click Next.
  1. Read and accept the license agreement and then click Finish.
  1. Restart Eclipse.

## Setup Android Virtual Device Manager ##
  1. Click on Window > Open Perspective > Java
  1. Click on Window > Android Virtual Device Manager
  1. Click on NEW, and enter:
    1. Name:  {user defined}
    1. Target:  Android 4.1.2 -API Level 16
    1. CPU:  ARM
    1. Use remaining defaults
    1. Click on "Create AVD"