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
require("$Bin/util.pl");

#############################################################################
# DATA  and RUN
#############################################################################
my $WHITE_LIST = "white_port";
my $BLACK_LIST = "black_port";
my $PROTO_TCP = "tcp";
my $PROTO_UDP = "udp";
my $MON_TM_LIMIT = 30;    #send out same error message every 30 sec

my @white_list = qw();
my @black_list = qw();

my $mon_tm = $MON_TM_LIMIT;
my $save_bad_white_list = "";
my $save_bad_black_list = "";
my $deli = "_";

main();
exit(0);

#############################################################################
# FUNCTIONS
#############################################################################
sub main {
   my $conf_name = $Bin . "/socketmon_conf";
   if (read_conf($conf_name) != 0) {
      my_exit(1);
   }
   register_monitor_name("SM");

   print("Enter(Port/Ticket/Code)> ");
   my $msg = <STDIN>;
   chomp($msg);
   my ($tcp_port, $ticket, $code) = split(/\//, $msg);

   my $listen_sock = 0;
   if (socket_listen("127.0.0.1", $tcp_port, \$listen_sock) != 0) {
      my_exit(1);
   }

   my $work_sock = 0;
   if (socket_accept($listen_sock, 30, \$work_sock) != 0) {
      my_exit(1);
   }
   socket_close($listen_sock);

   send_init($work_sock, $code);
   if (receive_ack_check($work_sock) != 0) {
      my_exit(1);
   }

   while (1) {
      monitor($work_sock);
      sleep(1);
   }
}

#############################################################################
sub tell_remote {
   my($sock, $event, $status, $text) = @_;

   if (length($event) == 0) {
      if (send_hello($sock, ) != 0) {
         my_exit(1);
      }
   }
   else { 
      if (send_event($sock, $event, $status, $text) != 0) {
         my_exit(1);
      }
   }

   if (receive_ack_check($sock) != 0) {
      my_exit(1);
   }
}

#############################################################################
sub monitor {
   my($sock) = @_;

   my @msg = `netstat  -tulpn`;

   my @loc_msg = qw();
   my $bad_white_list = "";
   my $bad_black_list = "";
   my $loc_save_bad = "";

   for (my $i = 0; $i <= $#msg; $i++) {
      chomp($msg[$i]);
      $msg[$i] =~ s/\t//g;      #remove all tabs
      $msg[$i] =~ s/( +)/ /g;   #replace mulitple spaces with one
      $msg[$i] =~ s/^ //;       #remove leading space

      my @tok = split(/ /, $msg[$i]);
      my $proto = $tok[0];
      my($ip, $port) = split(/:/, $tok[3]);
      my($pid, $proc) = split(/\//, $tok[6]);

      if ((length($proto) > 0) && ($port > 0)) {
         if (($proto eq $PROTO_TCP) || ($proto eq $PROTO_UDP)) {
            $loc_msg[1+$#loc_msg] = $proto . $deli . $port . $deli . $proc;
         }
      }
   }

   #Check the black list
   foreach my $m (@loc_msg) {
      my($proto, $port, $proc) = split(/$deli/, $m);
      foreach my $a (@black_list) {
         my($x, $y) = split(/$deli/, $a);
         if (($x eq $proto) && ($y == $port)) {
            my $text = $proto . $deli . $port . $deli . $proc;
            $bad_black_list .= $text . ",";
            tell_remote($sock, "0001", "RED", $text);
         }
      }
   }

   #Check the white list
   foreach my $a (@white_list) {
      my($x, $y) = split(/$deli/, $a);
      my $proto = "";
      my $port = "";
      my $flag = 0;
      foreach my $m (@loc_msg) {
         ($proto, $port) = split(/$deli/, $m);
         if (($x eq $proto) && ($y == $port)) {
            $flag = 1;
         }
      }
      if ($flag == 0) {
         my $text .= $x . $deli . $y;
         $bad_white_list .= $text . ",";
         tell_remote($sock, "0002", "RED", $text);
      }
   }

   #Compare black list for problem cleared
   my @tok_bad = split(/,/, $bad_black_list);
   my @tok_sav = split(/,/, $save_bad_black_list);
   foreach my $m1 (@tok_sav) {
      my $flag = 0;
      foreach my $m2 (@tok_bad) {
         if ($m1 eq $m2) {
            $flag = 1;    #still bad
         }
      }
      if ($flag == 0) {
         tell_remote($sock, "0001", "GREEN", $m1);
      }
      else {
         $loc_save_bad .= $m1 . ",";
      }
   }
   $save_bad_black_list = $loc_save_bad;

   #Compare white list for problem cleared
   my @tok_bad = split(/,/, $bad_white_list);
   my @tok_sav = split(/,/, $save_bad_white_list);
   foreach my $m1 (@tok_sav) {
      my $flag = 0;
      foreach my $m2 (@tok_bad) {
         if ($m1 eq $m2) {
            $flag = 1;    #still bad
         }
      }
      if ($flag == 0) {
         tell_remote($sock, "0002", "GREEN", $m1);
      }
      else {
         $loc_save_bad .= $m1 . ",";
      }
   }
   $save_bad_black_list = $loc_save_bad;

   #Send HELLO message
   $mon_tm = -1;
   if ($mon_tm <= 0) {
      tell_remote($sock, "", "", "");
      $mon_tm = $MON_TM_LIMIT;
   }
}

#############################################################################
sub set_monitor_list {
   my($fname, $cnt, $line) = @_;

   $line =~ s/\t//g;     #remove all tabs
   $line =~ s/( +)//g;    #remove all spaces

   my ($a, $b) = split(/#/, $line);
   if (length($a) <= 0) {
      return(0);      #skip comment
   }

   my ($mode, $value) = split(/=/, $a);
   my ($proto, $port) = split(/:/, $value);

   if ((length($port) <= 0) || ($port > 0xffff)) {
      my_print("(1)Syntax error at line $cnt of file '$fname'");
      return(1);
   }
   if (($proto ne $PROTO_TCP) && ($proto ne $PROTO_UDP)) {
      my_print("(2)Syntax error at line $cnt of file '$fname'");
      return(1);
   }

   if ($mode eq $WHITE_LIST) {
      $white_list[1+$#white_list] = $proto . $deli . $port;
   }
   elsif ($mode eq $BLACK_LIST) {
      $black_list[1+$#black_list] = $proto . $deli . $port;
   }
   else {
      my_print("(3)Syntax error at line $cnt of file $fname");
      return(1);
   }

   return(0);
}

#############################################################################
sub read_conf {
   my($name) = @_;

   if (open(FH, "$name")) {
      my_print("Read configuration file '$name'");

      my $line_cnt = 0;
      my $flag = 0;

      while (<FH>) {
         my $line = $_;     #assign the read line
         chomp($line);      #remove the \n
         $line_cnt += 1;

         if (set_monitor_list($name, $line_cnt, $line) != 0) {
            $flag = 1;
         }
      }
      close(FH);

      if ($flag != 0) {
         return(1);
      }
   } 
   else {
      my_print("Failed to open file '$name'");
      return(1); 
   }

   return(0);
}
