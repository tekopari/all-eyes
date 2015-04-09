'ae' daemon runs with root privileges.  It is a multi-threaded daemon.  The primary thread spawns and manages the monitors.  The second thread manaages SSL clients (i.e. AE Proxy).

'ae' daemon accepts two options.  With '-a' 'ae' daemon takes pre-configured actions (i.e. IGNORE or HALT the host).  If the daemon is invoked without -a option, it will operate in 'monitor only' mode.  Each individual monitor can be configured to be in VOLATILE or PERSISTENT mode (span) in the monitor structure during compile time.

# PROCEDURE FOR AE-DAEMON TO START UP A  MONITOR #

'ae' daemon spawns monitors.  If a monitor does not send regular heartbeat message or a status message within a specified period of time, then 'ae' daemon will kill the monitor.  The monitors are configured at the compile time with the daemon.  Based on the configuration, daemon calls corresponding monitor function after doing the following list of things:

  1. 'fork' a new process
  1. The ae-daemon creates a pair of SOCKET STREAM and re-directs the STDIN/STDOUT of the monitors to the daemon.
  1. Clears out other monitor's details and closes the other monitor's socketpair's file descriptors.
  1. Sets up the SIGCHLD signal to ignore.
  1. Drops the privileges.  This step contains two parts: one is to change the root (/) to chroot-jail, and then do setuid to 'ae' user.
  1. Marks the monitor has running and calls the monitor's C function.  Note that some monitors may exec a PERL/PYTHON/Java/RUBY (or any other language program) to implement a monitor, from the C function called by the monitor.
  1. Polls on the Socket File descriptor(s) of active monitor(s) for input messages.  Upon reeving a message from monitor, it checks for the AeMonitor protocol compliance.  If the message format is correct, then it sends a response back.
  1. 'ae' daemon also sends a ACK response back to SSL client's heart beat message.
  1. If the SSL client asks for the monitor's status, then the daemon responds with the status of all the running monitors.
  1. If a SSL client sends a message that is not consistent with the protocol, then it terminates the SSL connection.

Apart from supporting monitors and SSL clients, 'ae' daemon establishes a framework that is useful for rapid monitor development.  It also provides utility functions such as asDEBUG and aeLOG for debugging and logging the messages.

The protocol between the ae-daemon and the monitor is defined in page http://code.google.com/p/all-eyes/wiki/AeMonitorProtocol?ts=1351811756&updated=AeMonitorProtocol