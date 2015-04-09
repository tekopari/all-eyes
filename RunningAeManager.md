# Loading aeManager Project within eclipse #

Before loading aeManager Project within eclipse, please follow the instructions from page https://code.google.com/p/all-eyes/wiki/EclipseC_SVNSetup and  https://code.google.com/p/all-eyes/wiki/AndroidDevelopmentEnvironmentSetup to install the eclipse and the Android development environment.

There are two options for setting up the aeManager code base within eclipse.  The user could either a) checkout the code outside of eclipse and then import it, or b) checkout the code within eclipse.
Following describes the two options

## Import the _aeManager_ Project ##
  1. Start eclipse, select the workspace directory where you plan to checkout the code
  1. Select **_File_** then **_Import..._**
  1. Click General
  1. Double Click **_Existing Projects into Workspace_**
  1. For the **_Select root directory_** browse to the location of the source checked out via svn. Select the **_all-eyes/src/aemgr/aeManager_** directory as the root.
  1. Click the **_Copy projects into workspace_** option
  1. Lastly click **_Finish_** and the project will be added to your eclipse workspace
  1. **WARNING:** Multiple errors will be reported indicating compiler version is less than 1.5. To resolve this, go to **_Window/Preference/Java/Compiler_**, and change **_Compiler compliance level_** from **_1.6_** to **_1.4_**, and **_Apply_**. Then **_switch_** this back from **_1.4_** and **_1.6_** and **_Apply_**. This toggling seems to work-around a bug.

## Create _aeManager_ Project within eclipse ##
For this option, please follow the steps defined at:  https://code.google.com/p/all-eyes/wiki/EclipseC_SVNSetup


# Running and Configuring the _aeManager_ #
  1. In eclipse click on the _aeManager_ project and then click in top eclipse menu "Project" and select "clean".
  1. In eclipse right click on the top level _aeManager_ project in the project explorer
  1. Select the _Run As_ then _Android Application_
  1. The Android VM will start and the _aeManager_ application will be installed
  1. Once the VM starts unlock the VM and you will see the _aeManager_ warning banner.
  1. Select “Start” that's on the _aeManager_ warning banner screen
  1. Click the **_Settings_** **(on the droid device/VM)**
  1. Click the **_Username_** and enter your username then click the **_OK_** button
  1. Click the **_IP Address_** and enter **_alleyes.poly.edu_** then click the **_OK_** button
  1. Click the **_Port_** and enter **_8080_** then click the “OK” button
  1. Click the back arrow to return to the ae Manager Menu
  1. Important: If your development machine is hosting a web server stop it.

# Configuring the Android VM for Local Testing #
  1. In order to connect to the AE Proxy in the development environment one has to create a static IP address on the Android VM for **_alleyes.poly.edu_** domain.
  1. Start a **_terminal_** on the Linux development environment
  1. Run **_ifconfig_** to get the IP address of the Linux host running the aeProxy. These instructions will refer to this IP address as AE\_PROXY\_IP
  1. Start a terminal session on the running android VM with the command: **_adb shell_**
  1. Become super-user with the command **_su_**
  1. Make the filesystem writable with the command: **_mount –o rw,remount –t yaffs2 /dev/block/mtdblock3   /system_**
  1. Invoke the command: **_cd  /system/etc_**
  1. Create a hosts entry in the hosts file with the command replacing AE\_PROXY\_IP with the IP address found in step 3 above: **_echo AE\_PROXY\_IP alleyes.poly.edu  >> hosts_**
  1. Type **_exit_** twice to exit the adb shell
  1. Go back to AeManager App, and click on "View Events".  Click "Refresh", if ae-daemon is sending events to the AeManager, events will show up in this window.  For example:

![http://all-eyes.googlecode.com/svn/trunk/docs/AeMgrEventsList.png](http://all-eyes.googlecode.com/svn/trunk/docs/AeMgrEventsList.png)

**(NOTE:  If events do not show, first verify the AeManager is connecting to the AeProxy.  This can be viewed in the "LogCat" within eclipse)**

  1. User then can click on a specific event to obtain more details, and is provided ability to act upon the requested 'action'.

![http://all-eyes.googlecode.com/svn/trunk/docs/AeMgrSpecificEvent.png](http://all-eyes.googlecode.com/svn/trunk/docs/AeMgrSpecificEvent.png)


# Installing the _aeManager_ on an Android device #
  1. Build the application for release
  1. Attach the _aeManager.apk_ file found in your workspace in the _aeManager/bin_ directory to an email.  For added protection you can sign the email.
  1. Send the email to the owner of the Android device
  1. Open the email message on the Android-powered device
  1. The Android system will recognize the APK and display an _Install_ button.
  1. Install your application by clicking the _Install_ button.