# Motivation behind this project #
Perfect security is a myth.  Most unbreakable security of today, ages to vulnerability over time. Hence taking quick action after a security breach is critical.  Having a security layer along defensive lines, i.e. doing passive/active monitoring of critical components and taking actions based on configurable thresholds provides such a mitigation, against a possible security breach.
All-Eyes is such a framework with a potpourri of tools (monitors) useful for defenders, attackers and security professionals.

The goal of All-Eyes project is to ensure the scalability from small embedded systems to large scale servers.

For the NYU-POLY Application Security class project, we prototyped the concept on Ubuntu.

# Introduction #
All-Eyes (AKA ae) is a miniature applications-security intrusion detection system.  It has four components.  They are:

  1. 'ae' daemon that spawns and maintains monitors.  This component is implemented in C.  This component executes from the host, outside of chroot-jail.  Then it drops the privileges and spawns monitors to run within the chroot-jail.
  1. Monitors are independent processes spawned by the ae daemon. Each monitor watches a class of a system resources such as Sockets, running-processes, critical files etc. Monitors may be implemented in C or in other languages such as PERL, PYTHON, Java etc.
  1. 'ae proxy' acts as a DMZ between the Android ae-Manager and the ae daemon.  Proxy requires an authenticated connection.
  1. 'ae-mgr' is an Android application which helps users to choose various actions for ae daemon to take, based on the status of the monitors. It communicates over the Internet to ae daemon through a secure connection.  This component is implemented in Java.

# All-Eyes Project Presentation #
All-Eyes Project Presentation is at https://vimeo.com/55330398

# All-Eyes Demo #
All-Eyes demo is at https://vimeo.com/55219014

# High Level Architecture #

Following is an overview diagram of the AE architecture: ![http://all-eyes.googlecode.com/svn/trunk/docs/AEArchOverView.png](http://all-eyes.googlecode.com/svn/trunk/docs/AEArchOverView.png)


## 1) Linux System (ae daemon): ##

> 'ae' daemon runs on Ubuntu 12.04 (Precise Pangolin) release. It is a multi-threaded service.

> On one side, the daemon communicates with monitor(s) over socketpair(s); on the other side, daemon reports the status to a single SSL-client.

> For each monitor 'ae' daemon sets up the STDIN/STDOUT out of a socketpair.

> 'ae' daemon entertains only one SSL-client at any given time.

> monitors are sent SIGKILL signal (KILL -9) if ae-daemon encounters issue with that monitor.

> This design assumes monitors are not hostile to each other and ae-daemon.  In otherwords, 'ae' daemon and the monitors are considered to be in the same security domain.

> The current prototyped monitors are:
    * self monitor - Currently ensures the 'ae' daemon alive through the heart beat message.
    * socket monitor - Check a given list of listening ports (white list) and a list of non-active ports (black list).
    * process monitor - Check the given list of expected running processes.
    * critical file monitor - Ensure the file checksum value is unchanged.

> The procedure for creating a monitor is defined here: MonitorProcedure

> Protocol between daemon & monitors is described here: AeMonitorProtocol

> The monitors spawned by ae-daemon will run in chroot-jail. The installation package sets up chroot-jail for the user.

## 2) Gateway(AE Proxy): ##

The aeProxy is a multi-threaded application that acts as the proxy between the aeManager Android application and the 'ae' daemon.  Its purpose is to cache the ae message events until the user is able to receive and act upon the events. Please refer to page AeProxy for details.

## 3) Android Device(AE Manager): ##

> The Android Device receives monitors status from AE Proxy.  It also gives the users the option of taking action(s) or not.



# Implementation/Execution #

## 1) Development Environments: ##

We use Google-code setup for our development, integration and testing.  The link for the project is at https://code.google.com/p/all-eyes/source/browse/

  * 'ae' daemon - One needs to setup Ubuntu 12.04 (Precise Pangolin) with Java and C development environment.  For Eclipse installation details look at https://code.google.com/p/all-eyes/wiki/EclipseC_SVNSetup. Developers may like to use CLI based environment. In that case, they can install svn using apt-get on Ubuntu. Also, requires installation of SSL as described at page https://code.google.com/p/all-eyes/wiki/UbuntuSSLsetup
  * AE Proxy - Please refer to page AndroidDevelopmentEnvironmentSetup
  * AE Manager - Please refer to page AndroidDevelopmentEnvironmentSetup


## 2) Workspace Installation ##

In order to access the All Eyes source code the Subversion must be installed.

> Installation:
    1. **_sudo apt-get install subversion_**
    1. Answer Y at the prompt “Do you want to continue [Y/n]?”

> To check out the source code:
    1. **_mkdir ae_**
    1. **_cd ae_**
    1. **_svn checkout https://all-eyes.googlecode.com/svn/trunk/ all-eyes --username YOUR-PROJECT-EMAIL-ADDRESS_**
**NOTE: For aeMgr, checkout should be performed at the "aeManager" level:  src/aemgr/aeManager.**
(If you checkout the source code via Eclipse, multiple errors will be reported indicating compiler version is less than 1.5.  To resolve this, go to Window/Preference/Java/Compiler, and change "Compiler compliance level" from 1.6 to 1.4, and "Apply".  Then switch this back from 1.4 and 1.6 and "Apply".  This toggling seems to work-around a bug.)

To read up more on subversion:
You can refer to the link for free subversion book http://svnbook.red-bean.com

You can refer to the link for how to use subversion http://code.google.com/p/support/wiki/UserGroups

## 3) Build and Install the 'ae' daemon and `AeProxy` ##

  * Method one - source code install
    1. Go to directory **_../all-eyes/src_**
    1. Issue **_make_**. This will make and install the product
    1. At this point, the `AeProxy` is running as a service. You should start the 'ae' daemon by command **_/usr/local/bin/startae_** or **_sudo /usr/local/bin/ae -a_**

  * Method two - package install
    1. Go to **_../all-eyes/src_**
    1. Issue **_make package_** This will generate the tar file **_ae-and-proxy.tgz_**
    1. Copy the file **_ae-and-proxy.tgz_** and the file **_install\_ae-and-proxy.sh_** to a known directory and _cd_ to that directory
    1. Issue **_./install\_ae-and-proxy.sh_**. This will install the product.
    1. At this point, the `AeProxy` is running as a service. You should start the 'ae' daemon by command **_/usr/local/bin/startae_** or **_sudo /usr/local/bin/ae -a_**
    1. Setup and authorize the ae users (http://code.google.com/p/all-eyes/wiki/AuthenticationAndAuthorization)

> The _aeproxy_ is installed as a service, so it will automatically start running after the VM reboot. The commands below might be helpful
    * Check if it is running: _sudo service aeproxyd status_
    * Stop the service: _sudo service aeproxyd stop_
    * Start the service: _sudo service aeproxyd start_
    * Restart the service: _sudo service aeproxyd restart_

## 4) Run All-Eyes ##

To run All-Eyes and be able to see the communication traffic among 'ae' daemon and aeproxy, follow the steps below:

  * Method One
    * From a terminal, type **_/usr/local/bin/startae_**

  * Method Two
    1. From one terminal, type **_sudo service aeproxyd restart_** (the aeproxyd should be running at boot). Check the file  **_/var/log/aeproxy/aeproxy.log_** to see if the proxy is running. To setup and run the aeManager see http://code.google.com/p/all-eyes/wiki/RunningAeManager
    1. From another terminal, type **_sudo /usr/local/bin/ae -a_** (where -a for take action)

# Development Testing #

We have been doing incremental testing through out the development cycle (running ae daemon regression test with the monitors, after every couple of builds helps us to iron out bugs before going on to far.)

All the prototype testing were conducted on a single VM running all the components ('ae' daemon, stand-alone monitors, AE Proxy, and AE Manager).

The details of issues/bugs found are documented in http://code.google.com/p/all-eyes/issues/list.  We are continuously resolving the bugs and adding code towards enhancements.

# Coding Styles #

  1. Function type & name on same line:
> > int functionA(int A, int B)
  1. All defined variables will be assigned a default value:
> > int i = 1, j = -1;
  1. Indentation, users are encouraged to indent their code for easy reading:
```
int functionA(void)
{
   int ret=0;
       
   printf("Hello\n");
   return(0);
}
```
  1. Comments:
    * Before every function.
    * Located at right-hand side of statements.
    * In general, when in doubt, comment.
  1. Security:  Add SECURITY comment when have a security concern.


# Security #

We have applied the following layers of security measures during the project development cycle:
  * Strict GCC compilation flags
  * Create a chroot jail for running all the monitor processes.
  * Run command ulimit inside the chroot jail to limit the resource usage
  * Enabled and applied `AppArmor` policies, especially applied the `AppArmor` policy for 'ae' daemon
  * For stand-alone monitors, calculated and compiled the SHA256 checksum values into the 'ae' daemon so that when 'ae' daemon spawns the monitors, it dynamically re-calculates and compares the checksum values to ensure the legitimacy of the monitors
  * Start the 'ae' daemon as root in the host, then jump into chroot jail and drop the privilege to ae user
  * Use the SSL connection between 'ae' daemon and AE Proxy
  * Use the SSL connection between the AE Proxy and AE Manager
  * Use of username/password authentication
  * Use of OAUTH authentication
  * Use of access control list authorization

# Future Enhancements #

  * Add support for all signals.
  * Do security analysis of whether different protocol is necessary between the `aedaemon-and-AeManager` (SSL client) and `aedaemon-and-Monitors`.
  * Make 'ae' as a service.
  * Restrict very verbose log message.
  * Debug messages needs to be commented out - for the prototype.
  * Reduce the priority level of monitors.
  * Make nice macro for Lock/Unlock.
  * A monitor’s action should result in targetted process(es) killed, upon aeMgr’s approval.


# Security Vulnerabilities #

We have been tracking all the issues (to be fixed/unfixed/enhancement) at https://code.google.com/p/all-eyes/issues/list.

The security related exploitable vulnerabilities are specifically marked with 'security' label.

To see all the issues we have been tracking, choose **All issues** option in the Search box and press **_Search_** button.