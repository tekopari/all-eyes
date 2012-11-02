# Last Modified: Sun Oct 21 11:56:20 2012
#include <tunables/global>

/ae/jail/bin/socketmon.pl {
  #include <abstractions/base>
  #include <abstractions/perl>

  deny capability sys_ptrace,




  /ae/jail/bin/dash r,
  /ae/jail/bin/netstat rix,
  /ae/jail/bin/socketmon.pl r,
  /ae/jail/bin/util.pl r,
  /ae/jail/bin/which rix,
  /ae/jail/dev/urandom r,
  /ae/jail/etc/ld.so.cache r,
  /ae/jail/etc/locale.alias r,
  /ae/jail/etc/ae/exports/socketmon_conf r,
  /ae/jail/lib/i386-linux-gnu/ld-*.so r,
  /ae/jail/lib/i386-linux-gnu/libc-*.so mr,
  /ae/jail/lib/i386-linux-gnu/libcrypt-*.so mr,
  /ae/jail/lib/i386-linux-gnu/libdl-*.so mr,
  /ae/jail/lib/i386-linux-gnu/libm-*.so mr,
  /ae/jail/lib/i386-linux-gnu/libpthread-*.so mr,
  /ae/jail/proc/ r,
  /ae/jail/proc/** r,
  /ae/jail/usr/lib/perl/** mr,
  /ae/jail/usr/share/perl/** r,
  /usr/bin/perl ix,

}
