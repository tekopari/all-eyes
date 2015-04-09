Each monitor can have its own configuration file. This configuration file is saved under the directory /etc/ae/exports of the base kernel. The content of the configuration file is entirely depended upon the monitor's design itself. Currently, we have the following list of monitors that requires the monitor configuration file.

  * socketmon
  * procmon
  * filemon

In general, once you have modified the monitor configuration file, it requires the restart of the 'ae' daemon.

```
```

# The _socketmon_ Configuration File #

The monitor **_socketmon_** requires its own configuration file. This configuration file is named **_socketmon\_conf_** and is saved under the directory **_/etc/ae/exports_** of the base kernel. This file describes the white list TCP ports and black list TCP ports that the **_socketmon_** will monitor for.  Below is the description of how to use this configuration file.

For each line it must conform to the format:

> _**`<mode> = <proto>:<port>:<process>:<action>`**_

| **Field Name** | **Description** |
|:---------------|:----------------|
| _`<mode>`_ | The mode: **_white\_port_, _black\_port_** |
| _`<proto>`_ | The protocol: tcp, udp |
| _`<port>`_ | Integer value of a valid port |
| _`<process>`_ | The process name |
| _`<action>`_ | Action to be taken: **_ignore_, _logmsg_, _blank_** for ignore |

Examples:
```
black_port = tcp:123:ntpd:logmsg
black_port = udp:22:sshd:logmsg
black_port = tcp:22:sshd:logmsg
white_port = tcp:80:httpd:ignore
white_port = tcp:8080:httpd:logmsg
```

Modification to this configuration file requires the restart of the 'ae' daemon.

### The _sockemon_ is running in PERSISTENT mode ###
There is a log file stored at _/tmp/socketmon.log_ under chroot environment. This file logs all protocol messages between this monitor and the 'ae' daemon. The maximum file size is 255MB. Once it is over the maximum file size limit, the file will be cleared before logging the next message.

```
```

# The _procmon_ Configuration File #

The monitor **_procmon_** requires its own configuration file. This configuration file is named **_procmon\_conf_** and is saved under the directory **_/etc/ae/exports_** of the base kernel. This file describes the processes that the **_procmon_** will monitor for.  Below is the description of how to use this configuration file.

For each line it must conform to the format:

> _**`<mode> = <proc_name>:<process>:<action>`**_

| **Field Name** | **Description** |
|:---------------|:----------------|
| _`<mode>`_ | The mode is **_proc\_name_** |
| _`<proc_name>`_ | Path and name of the process |
| _`<process>`_ | The process name. This is the process command |
| _`<action>`_ | Action to be taken: **_ignore_, _logmsg_, _blank_** for ignore |

Examples:
```
proc_name = myproc:myproc:ignore
proc_name = myproc2:myproc2 -a:logmsg
proc_name = dconf-service:dconf-service:ignore
proc_name = sshd:/usr/sbin/sshd -D:logmsg
```

Modification to this configuration file requires the restart of the 'ae' daemon.

### The _procmon_ is running in VOLATILE mode ###

```
```

# The _filemon_ Configuration File #

The monitor **_filemon_** requires its own configuration file. This configuration file is named **_fileMonConfigFile_** and is saved under the directory **_/etc/ae/exports_** of the base kernel. This file describes the critical files that the **_filemon_** will monitor the checksum for.  This file is originally created during "build process" of the product, and is copied over during product installation.

Below is the description of how to use this configuration file.

For each line it must conform to the format:

> _**`<filename>`**_

Examples:
```
 /etc/passwd
 /etc/hosts
```

A second(related) file used by **_filemon_** is **_fileMonConfigFileChkSum_**.  This file is created by reading the fileMonConfigFile, calculating the sha256sum of each file, and then writing the output to **_fileMonConfigFileChkSum_**.

each line must conform to the format:

> _**`<checksum{sp}filename>`**_



Examples:
```
 dfa08c7076d3ad5e42db35383106df8058237469a2219f4e40a9bdc93020fc8a  /etc/passwd
 6d2fa288f5811787acca62857501b9e35c062a6911039bb2dfd3bfbe5d276c17  /etc/hosts

```

The creation of **_fileMonConfigFileChkSum_** is performed by executable file **/bin/filemondConfig**.  The user is asked to run this command during product installation.

### The _filemon_ is running in VOLATILE mode ###