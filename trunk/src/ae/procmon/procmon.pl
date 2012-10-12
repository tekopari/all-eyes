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
# File Name: procmon.pl
# Description:
#    This is a daemon that should be started up by the "ae" manager daemon. This
#    daemon monitors all a listening running processes by calling the "ps" command.
#    It first reads the config file named "procmon_conf". The config file stores a
#    list of process names that we expect to be on the proc list. Based on the
#    information in the config file and the result from "ps", this monitor generates
#    error message and transmits it back to "ae" manager.
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
# DATA AND RUN
#############################################################################
my $PROC_LIST = "proc_name";
my $MON_TM_LIMIT = 30;    #send out same error message every 30 sec
my $HELLO_TM_LIMIT = 30;  #send out hello message every 30 sec

my @proc_list = qw();

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
   my $conf_name = $Bin . "/procmon_conf";
   if (read_conf($conf_name) != 0) {
      my_exit(1);
   }

   my $listen_sock = 0;
   if (socket_listen($tcp_port, \$listen_sock) != 0) {
      my_print("Failed to listen on port '$tcp_port'");
      my_exit(1);
   }

   my $timeout = 30;    #seconds
   my $work_sock = 0;
   if (socket_accept($listen_sock, $timeout, \$work_sock) != 0) {
      my_print("Unable to accept connection");
      my_exit(1);
   }

   if(socket_verify($work_sock) != 0) {
      my_print("Failed to verify protocol");
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

   my @msg = `ps -ef`;

   my @full_name = qw();
   my $bad_proc_list = "";

   for (my $i = 0; $i <= $#msg; $i++) {
      chomp($msg[$i]);
      $msg[$i] =~ s/\t//g;      #remove all tabs
      $msg[$i] =~ s/( +)/ /g;   #replace mulitple spaces with one
      $msg[$i] =~ s/^ //;       #remove leading space

      my @tok = split(/ /, $msg[$i]);
      $full_name[1+$#full_name] = $tok[7];
   }

   foreach my $a (@proc_list) {
      my $flag = 0;
      foreach my $f (@full_name) {
         my $pos = rindex($f, "/");
         my $n = substr($f, $pos+1, length($f));

         my @c = split(//, $a);
         if ($c[0] eq '/') {  #compare full path-name string 
            if ($a eq $f) {
               $flag = 1;
            }
         }
         else {              #compare name string only
            if ($a eq $n) {
               $flag = 1;
            }
         }
      }
      if ($flag == 0) {
         $bad_proc_list .= $a . ","; 
      }
   }

   #Generate message
   if (length($bad_proc_list) > 0) {
      chop($bad_proc_list);
      my $result = req_problem() . ":proc(" . $bad_proc_list .")";

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
sub set_proc_list {
   my($fname, $cnt, $line) = @_;

   $line =~ s/\t//g;     #remove all tabs
   $line =~ s/( +)//g;    #remove all spaces

   my ($a, $b) = split(/#/, $line);
   if (length($a) <= 0) {
      return(0);      #skip comment
   }

   my ($mode, $value) = split(/=/, $a);

   if ($mode eq $PROC_LIST) {
      $proc_list[1+$#proc_list] = $value;
   }
   else {
      my_print("Syntax error at line $cnt of file $fname");
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

         if (set_proc_list($name, $line_cnt, $line) != 0) {
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
