# Last Modified: Sun Oct 21 12:02:34 2012
#include <tunables/global>

/ae/jail/bin/procmon.pl {
  #include <abstractions/base>
  #include <abstractions/perl>
  #include <abstractions/ubuntu-konsole>

  deny capability sys_ptrace,


  /ae/jail/bin/dash r,
  /ae/jail/bin/procmon.pl r,
  /ae/jail/bin/ps rix,
  /ae/jail/bin/util.pl r,
  /ae/jail/bin/which rix,
  /ae/jail/dev/urandom r,
  /ae/jail/etc/ld.so.cache r,
  /ae/jail/etc/locale.alias r,
  /ae/jail/etc/localtime r,
  /ae/jail/etc/nsswitch.conf r,
  /ae/jail/etc/passwd r,
  /ae/jail/etc//ae/exports/procmon_conf r,
  /ae/jail/lib/i386-linux-gnu/ld-*.so r,
  /ae/jail/lib/i386-linux-gnu/libc-*.so mr,
  /ae/jail/lib/i386-linux-gnu/libcrypt-*.so mr,
  /ae/jail/lib/i386-linux-gnu/libdl-*.so mr,
  /ae/jail/lib/i386-linux-gnu/libm-*.so mr,
  /ae/jail/lib/i386-linux-gnu/libnsl-*.so mr,
  /ae/jail/lib/i386-linux-gnu/libnss_compat-*.so mr,
  /ae/jail/lib/i386-linux-gnu/libnss_files-*.so mr,
  /ae/jail/lib/i386-linux-gnu/libnss_nis-*.so mr,
  /ae/jail/lib/i386-linux-gnu/libpthread-*.so mr,
  /ae/jail/lib/lib*so* mr,
  /ae/jail/proc/ r,
  /ae/jail/proc/** r,
  /ae/jail/usr/lib/perl/** mr,
  /ae/jail/usr/share/perl/** r,
  /usr/bin/perl ix,

}
