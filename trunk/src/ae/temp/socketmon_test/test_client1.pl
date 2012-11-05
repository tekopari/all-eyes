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
# File Name: socketmon.pl
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
require("$Bin/../lib_test/util1.pl");

main();
exit(0);

#############################################################################
sub main {
   print("Enter(Port/Ticket)> ");
   my $msg = <STDIN>;
   chomp($msg);
   my($tcp_port, $ticket) = split(/\//, $msg);

   print("Enter Own Msg (y/n)? ");
   my $code = <STDIN>;
   chomp($code);

   register_monitor("AE", $ticket, 0);

   my $sock = 0;
   if (socket_connect("127.0.0.1", $tcp_port, \$sock) != 0) {
      exit(1);
   }

   while (1) {
      my $buf = socket_receive_block($sock);
      print("RCV:$buf\n");

      my $in_buf = "";
      if (($code eq "y") || ($code eq "Y")) {
         $in_buf = <STDIN>;
      }
      else {
         $in_buf = "[:10:11:AE:]";
      }

      socket_send($sock, $in_buf);
   }
   socket_close($sock);
}

