# Last Modified: Sun Oct 21 11:28:37 2012
#include <tunables/global>

/ae/jail/usr/bin/perl {
  #include <abstractions/base>



  /ae/jail/dev/urandom r,
  /ae/jail/etc/ld.so.cache r,
  /ae/jail/etc/locale.alias r,
  /ae/jail/lib/i386-linux-gnu/libc-*.so mr,
  /ae/jail/lib/i386-linux-gnu/libcrypt-*.so mr,
  /ae/jail/lib/i386-linux-gnu/libdl-*.so mr,
  /ae/jail/lib/i386-linux-gnu/libm*.so.6 mr,
  /ae/jail/usr/bin/perl mr,

}
