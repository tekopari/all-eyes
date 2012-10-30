#!/usr/bin/perl
#######################################################################################################
# Copyright (C) <2012> <Blair Wolfinger, Ravi Jagannathan, Thomas Pari, Todd Chu>
#
# Permission is hereby granted, free of charge, to any person obtaining a copy of this
# software and associated documentation files (the "Software"), to deal in the Software
# without restriction, including without limitation the rights to use, copy, modify,
# merge, publish, distribute, sublicense, and/or sell copies of the Software, and to
# permit persons to whom the Software is furnished to do so, subject to the following conditions:
# The above copyright notice and this permission notice shall be included in all copies
# or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,
# INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR
# PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR
# ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
# ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
#
# Original Author: Todd Chu
#
#######################################################################################################
# File Name: client_ssl_test.pl
# Description:
#    This is a daemon that should be started up by the "ae" manager daemon. This daemon
#    monitors all the active listening TCP/UDP ports by calling the "netstat" command.
#    It first reads the config file named "socketmon_conf". The config file stores a
#    list of tcp/udp ports that we expect to be in listening mode, called white_port
#    list. The config file also stores a list of tcp/udp ports that we expect to not
#    be in listening mode, called black_port list. Based on the information in the
#    config file and the result from "netstat", this monitor generates error message
#    and transmits it back to "ae" manager.
######################################################################################################

use strict;
use FindBin qw($Bin $Script);
use Cwd qw(getcwd abs_path);
$Bin = abs_path($Bin);
require("$Bin/util2.pl");

my $ip = $ARGV[0];
my $tcp_port = $ARGV[1];

if ($#ARGV != 1) {
   $ip = "127.0.0.1";
   $tcp_port = 6000;
}

main();
exit(0);

#############################################################################
# FUNCTIONS
#############################################################################
sub main {
   my $ssl_cert_dir = $Bin . "/cert/";
   socket_use_ssl($ssl_cert_dir."clientcert81.pem",
                  $ssl_cert_dir."clientcert81.pem",
                  $ssl_cert_dir."cacert81.pem",
                  "password");

   my $sock = 0;
   if (socket_connect_ssl($ip, $tcp_port, \$sock) != 0) {
      my_exit(1);
   }

   ##### TEST CODE #####
#   socket_send_ssl($sock, "RES_INIT");
#   my $buf = socket_receive_ssl($sock);
#   socket_close_ssl($sock);
#   my_exit(0);
   ##### TEST CODE #####

   while (1) {
      socket_send_ssl($sock, "[:10:00:AM:]");
      my $buf = socket_receive_ssl($sock);
      sleep(2);

#      my $in_buf = <STDIN>;
#      socket_send_ssl($sock, "$in_buf");
   }

   socket_close_ssl($sock);
}
