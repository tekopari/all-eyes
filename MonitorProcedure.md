# PROCEDURE FOR WRITING MONITOR #

## Integrated or Stand-alone Monitor ##
  1. For monitors written in C and integrated as part of 'ae' daemon, write the monitor and compile it as a object file that is linked with the 'ae' daemon together.
  1. For monitors written in languages, like Perl, Python, Shell, Java script, write the monitor to be a stand-alone process.  In the same directory of the monitor, there is a small C file that acts as the wrapper between the monitor and the 'ae' daemon. This C file calls the monitor and is compiled as a object file to be linked with the 'ae' daemon.
  1. For stand-alone monitor, the Makefile under the same monitor directory calculates the SHA256 checksum of the stand-alone monitor and its related files. The checksum values are compiled into the small C file, the wrapper code, and linked with the 'ae' daemon. When the 'ae' daemon spawns the stand-alone monitor, it will recalculate the checksum(s) can compare the checksum values before launching the stand-alone monitor.

## Available Monitors ##
As of this writing, the available monitors are listed in the table below:
| **Name** | **Type** | **Description** | **Language** |
|:---------|:---------|:----------------|:-------------|
|selfmon | Integrated | Currently ensures the 'ae' daemon alive through the heart beat message | C|
| socketmon | Stand-alone | Check a given list of listening ports (white list) and a list of non-active ports (black list) | Perl |
| procmon | Stand-alone | Check the given list of expected running processes | Perl |
| filemon | Integrated | Ensure the file checksum value is unchanged | C |

## Procedure of Writing the Integrated Monitor ##
To explain the process more clearly, let's use the example of writing an integrated monitor called **_sensormon_**.
  1. Create a monitor directory under **_all-eyes/src/ae_**, name it to be **_sensormon_**.
  1. Copy files from the existing directory **_all-eyes/src/ae/filemon/`*`_** and replace the name **_filemon_** to be **_sensormon_**.
  1. Modify the **_Makefile_** to replace the name **_filemon_** to be **_sensormon_**.
  1. Modify the C and its related files to fit the need for **_sensormon_**. Please refer to the existing monitor code for how to write the monitor flow. The heat of the monitor is to send and receive message to/from 'ae' daemon where the monitor sends message to 'ae' daemon by writing the message to **_STDOUT_** and receives message from 'ae' daemon by reading the message from **_STDIN_**.
  1. Please note that the monitor should use **_STDERR_** for displaying its internal message. This is because the **_STDOUT_** and **_STDIN_** have been consumed for communicating with the 'ae' daemon.
  1. The monitor message protocol is defined in page **AeMonitorProtocol**
  1. Modify the following files under **_all-eyes/src/ae/daemon_**:
    * **_aeconf.h_**  - to include the **_sensormon_** structure
    * **_aedaemon.h_**  - increase the total number of monitor structure counter **_MAXMONITORS_**
  1. Modify the file **_all-eyes/src/ae/include/ae.h_** to declare **_sensormon_** there

## Procedure of Writing the Stand-alone Monitor ##
To explain the process more clearly, let's use the example of writing a stand-alone monitor called **_mymon_**.
  1. Create a monitor directory under **_all-eyes/src/ae_**, name it to be **_mymon_**.
  1. Copy the small C file, the wrapper, from the existing directory **_all-eyes/src/ae/socketmon/socketmon.c_** and rename it to be **_mymon.c_**. Modify the file to replace the word socketmon to be mymon.
  1. Copy the file **_Makefile_** from the existing directory **_socketmon/Makefile_**. Modify the file to replace the file names to be the file names for **_mymon_**. Please note that the **_Makefile_** includes the **_sha256chk_** to generate the sha256 checksum values for a given set of files and a new C file named **_mymon\_sha256.c_** is generated to includes the checksum values.
  1. Create the file the stand-alone monitor source file named **_mymon.{ext}_**. Please refer to the existing monitor code for how to write the monitor flow. The heat of the monitor is to send and receive message to/from 'ae' daemon where the monitor sends message to 'ae' daemon by writing the message to **_STDOUT_** and receives message from 'ae' daemon by reading the message from **_STDIN_**.
  1. Please note that the monitor should use **_STDERR_** for displaying its internal message. This is because the **_STDOUT_** and **_STDIN_** have been consumed for communicating with the 'ae' daemon.
  1. The monitor message protocol is defined in page **AeMonitorProtocol**
  1. Modify the following files under **_all-eyes/src/ae/daemon_**:
    * **_aeconf.h_**  - to include the **_mymon_** structure
    * **_aedaemon.h_**  - increase the total number of monitor structure counter **_MAXMONITORS_**
  1. Modify the file **_all-eyes/src/ae/include/ae.h_** to declare **_mymon_** there
  1. Modify the **`AppArmor`** profile **_all-eyes/src/ae/apparmor/usr.local.bin.ae_** to add the line about the **_/ae/jail/bin/mymon ix,_**. Please note that for each file used by **_mymon_**, there should be a line in this apparmor profile.

## Monitor Configuration File ##
  1. In case that the monitor needs to read its configuration file, this configuration file should be placed in directory **_/etc/ae/exports_**. This directory is a common place for all monitor configuration files. Please see page MonitorConfiguration for monitor configuration details.
  1. Modify the **`AppArmor`** profile **_all-eyes/src/ae/apparmor/usr.local.bin.ae_** to add the line about the new configuration file. For example, it can be **_/ae/jail/etc/ae\_exports/mymon\_conf r,_**.

## Testing ##
  1. For stand-alone monitor, you can test it by running the monitor stand-alone by (1) use the keyboard for entering the message that will be sent to the monitor, (2) the output message from monitor will be displayed on screen.
  1. For non stand-alone monitor (written in C), the testing should be done with the 'ae' daemon together.