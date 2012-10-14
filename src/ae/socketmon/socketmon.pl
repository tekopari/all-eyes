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
require("$Bin/../lib/util.pl");

my $tcp_port = $ARGV[0];
my $seed = $ARGV[1];

if ($#ARGV != 2) {
   print("******************* TEST RUN ONLY ********************\n");
   $tcp_port = 3456;
   $seed = 1;
}

#############################################################################
# DATA  and RUN
#############################################################################
my $WHITE_LIST = "white_port";
my $BLACK_LIST = "black_port";
my $PROTO_TCP = "tcp";
my $PROTO_UDP = "udp";
my $MON_TM_LIMIT = 30;    #send out same error message every 30 sec
my $HELLO_TM_LIMIT = 30;  #send out hello message every 30 sec

my @white_list = qw();
my @black_list = qw();

my $expected_response = "";
my $expected_response_err = 0;
my $mon_result = "";
my $mon_tm = 0;
my $hello_tm = 0;

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

   my $ssl_cert_dir = $Bin . "/../utils/todd_cert/";
   socket_use_ssl($ssl_cert_dir."clientCrt_admin.pem", 
                  $ssl_cert_dir."clientCrt_admin.pem",
                  $ssl_cert_dir."cacert.pem",
                  "password");


   my $listen_sock = 0;
   if (socket_listen("127.0.0.1", $tcp_port, \$listen_sock) != 0) {
      my_exit(1);
   }

   my $timeout = 30;    #seconds
   my $work_sock = 0;
   if (socket_accept($listen_sock, $timeout, \$work_sock) != 0) {
      my_exit(1);
   }

   if(socket_verify($work_sock) != 0) {
      my_exit(1);
   }

   close(listen_sock);

   if (socket_select($work_sock, "receive_sub", "monitor_sub") != 0) {
      close(work_sock);
   }
}

#############################################################################
sub receive_sub {
   my($sock, $buff) = @_;

   if ($buff ne $expected_response) {
      $expected_response_err += 1;
      my_print("Expecting response '$expected_response' but received '$buff' (err_cnt=$expected_response_err)");
   }
   else {
      $expected_response = "";
      $expected_response_err = 0;
   }
}

#############################################################################
sub monitor_sub {
   my($sock) = @_;

   my @msg = `netstat  -tulpn`;

   my @loc_msg = qw();
   my $bad_white_list = "";
   my $bad_black_list = "";

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
            $loc_msg[1+$#loc_msg] = $proto . ":" . $port . ":" . $proc;
         }
      }
   }

   #Check the black list
   foreach my $m (@loc_msg) {
      my($proto, $port, $proc) = split(/:/, $m);
      foreach my $a (@black_list) {
         my($x, $y) = split(/:/, $a);
         #TC debug_print("BLK:$x-$proto,$y-$port,$proc");

         if (($x eq $proto) && ($y == $port)) {
            $bad_black_list .= $proto . ":" . $port . ":" . $proc . ",";
            #TC debug_print(":BAD");
         }
         #TC debug_print("\n");
      }
   }

   #Check the white list
   foreach my $a (@white_list) {
      my($x, $y) = split(/:/, $a);
      my $proto = "";
      my $port = "";
      my $flag = 0;
      foreach my $m (@loc_msg) {
         ($proto, $port) = split(/:/, $m);
         #TC debug_print("WHT:$x-$proto,$y-$port");

         if (($x eq $proto) && ($y == $port)) {
            $flag = 1;
            #TC debug_print(":BAD");
         }
         #TC debug_print("\n");
      }
      if ($flag == 0) {
         $bad_white_list .= $x . ":" . $y . ",";
      }
   }

   #Generate message
   if ((length($bad_black_list) > 0) || (length($bad_white_list) > 0)) {
      chop($bad_black_list);
      chop($bad_white_list);
      my $result = req_problem() . ":black(" . $bad_black_list ."),white(" . $bad_white_list . ")";


      if (($result ne $mon_result) || ($mon_tm == 0)) {
         $mon_tm = $MON_TM_LIMIT;
         $mon_result = $result;
         $expected_response = res_problem();
         socket_send($sock, $result);
      }
      $mon_tm -= 1;
   }
   else {
      if  (length($mon_result) > 0) {
         my $result = req_clear();
         $expected_response = res_clear();
         socket_send($sock, $result);

         $mon_result = "";
         $mon_tm  = 0;
         $hello_tm = 0;
      }
      elsif ($hello_tm <= 0) {
         $hello_tm = $HELLO_TM_LIMIT;
         my $result = req_hello();
         $expected_response = res_hello();
         socket_send($sock, $result);
      }
      $hello_tm -= 1;
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
      $white_list[1+$#white_list] = $proto.":".$port;
   }
   elsif ($mode eq $BLACK_LIST) {
      $black_list[1+$#black_list] = $proto.":".$port;
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
