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
# File Name: util.pl
# Description:
#     Perl utility file contains: (1) protocol op code list, (2) common IO fuctions
######################################################################################################

use strict;

use FindBin qw($Bin $Script);
use Cwd qw(getcwd abs_path);
my $Bin = abs_path($Bin);
use lib "$FindBin::Bin";

#############################################################################
# Protocol defination
my $P_VER = "10";
my $P_TYPE_HELLO = "00";
my $P_TYPE_ACK = "11";
my $P_TYPE_EVENT = "22";
my $P_NAME_AE = "AE";
my $P_EVENT_INIT = "9999";
my $P_STATUS_GREEN = "00";
my $P_STATUS_ORANGE = "01";
my $P_STATUS_RED = "11";
my $P_ACTION_IGNORE = "A0";
my $P_ACTION_LOGMSG = "A1";

my $s = "\[";  #protocol start marker
my $e = "\]";  #protocol end marker
my $d = ":";   #protocol delimeter

my $mname = "";  #monitor name
my $mode = "V";
my $logname = "";
my $debug = 0;

#############################################################################
sub check_syscmd {
   my($cmd) = @_;

   my $ava = `which $cmd`;
   chomp($ava);
   if ((length($ava) <= 0) || ($ava ne $cmd)) { # guarantee the FULL path to cmd
      my_print("Expected '$cmd' not availalbe");
      my_exit(1);
   }
}

#############################################################################
sub register_clear {
   $mname = "";
   close(FH);
}

#############################################################################
sub register_monitor {
   my($mon_name, $mflag, $flag, $lname) = @_;

   if ($flag > 0) {
      $debug = 1;
   }

   if (length($mon_name) <= 0) {
      util_print_err("Monitor name can't be zero length!");
      return(1);
   }
   $mname = $mon_name;

   if ($mflag eq "-p") {
      $mode = "P";
   }
   elsif ($mflag eq "-v") {
      $mode = "V";
   }
   else {
      util_print_err("Undefined mode: $mflag");
      return(1);
   }

   if (($mode eq "P") && (length($lname) > 0)) {
      $logname = $lname;
      if (! open(FH, ">>$logname")) {
         util_print_err("(1)Unable to open the file $logname for write");
         return(1);
      }
   }

   return(0);
}

#############################################################################
sub check_monitor_name {
   if (length($mname) <= 0) {
      util_print_err("Invalid monitor name '$mname'");
      return(1);
   }
   return(0);
}

#############################################################################
sub send_init {
   my($text) = @_;

   if (check_monitor_name() != 0) {
      return(0, 1);
   }

   my $msg_id = create_msgid();

   my $msg = $s.$d.$P_VER.$d.$msg_id.$d.$P_TYPE_EVENT.$d.$mname.$d.$P_EVENT_INIT.$d.$P_STATUS_GREEN.$d.$P_ACTION_IGNORE.$d.$text.$d.$e;
   _send($msg);
   return($msg_id, 0);
}

#############################################################################
sub send_hello {
   if (check_monitor_name() != 0) {
      return(0, 1);
   }

   my $msg_id = create_msgid();

   my $msg = $s.$d.$P_VER.$d.$msg_id.$d.$P_TYPE_HELLO.$d.$mname.$d.$e;
   _send($msg);
   return($msg_id, 0);
}

#############################################################################
sub send_event {
   my($event, $status, $text, $action) = @_;

   my $st = "";

   if (check_monitor_name() != 0) {
      return(0, 1);
   }
   if (length($event) <= 0) {
      util_print_err("Invalid event code '$event'");
      return(0, 1);
   }

   if ($status eq "GREEN") {
      $st = "00";
   } elsif ($status eq "ORANGE") {
      $st = "01";}
   elsif ($status eq "RED") {
      $st = "11";}
   else {
      util_print_err("Invalid status code '$status'");
      return(0, 1);
   }

   my $act = $P_ACTION_IGNORE;
   if ($action eq "logmsg") {
      $act = $P_ACTION_LOGMSG;
   }

   my $msg_id = create_msgid();

   my $msg = $s.$d.$P_VER.$d.$msg_id.$d.$P_TYPE_EVENT.$d.$mname.$d.$event.$d.$st.$d.$act.$d.$text.$d.$e;
   _send($msg);
   return($msg_id, 0);
}

#############################################################################
sub send_ack {
   my($msg_id) = @_;

   if (check_monitor_name() != 0) {
      return(1);
   }

   my $msg = $s.$d.$P_VER.$d.$msg_id.$d.$P_TYPE_ACK.$d.$mname.$d.$e;
   _send($msg);

   return(0);
}

#############################################################################
sub receive_ack_check {
   my($msg_id) = @_;

   my $msg = _receive();
   return (ack_check($msg, $msg_id));
}

#############################################################################
sub ack_check {
   my($msg, $msg_id) = @_;

   my $loc_msg = $P_VER.$d.$msg_id.$d.$P_TYPE_ACK.$d.$P_NAME_AE;

   if ($loc_msg ne $msg) {
      util_print_err("Expect $loc_msg but received $msg");
      return(1);
   }

   return(0);
}

#############################################################################
sub util_print_err {
   my($msg) = @_;
   print STDERR "$0: $msg\n";
}

#############################################################################
sub _send {
   my($msg) = @_;

   chomp($msg);
   $msg .= "\n";
   if ($debug) {
      util_print_err("SND:$msg");
   }
   print STDOUT "$msg";
   $|=1;  #Flush

   if ($mode eq "P") {
      logmsg("SND", $msg);
   }
}

#############################################################################
sub _receive {
   my $buf = <STDIN>;

   if ($mode eq "P") {
      logmsg("RCV", $buf);
   }

   if ($debug) {
      util_print_err("RCV:$buf");
   }
   my @tok = split(/$d/, $buf);
   if (($tok[0] eq $s) || ($tok[$#tok] =~ /$e/)) {
      my $loc_buf = "";
      for (my $i = 1;  $i < $#tok; $i++) {
         $loc_buf .= $tok[$i] . $d;
      }
      chop($loc_buf);
      return($loc_buf);
   }
   util_print_err("Received invalid message: '$buf'");
   return($buf);
}

#############################################################################
sub logmsg {
   my($snd_rcv, $msg) = @_;

   #####  For this project, if the log file size is more than 0xFFFFFFF,
   #####  we just delete the log file and create a new log file
   my $size = -s $logname;
   if ($size > 0xFFFFFFF) {
      close(FH);
      if (! open(FH, ">$logname")) {
         util_print_err("(2)Unable to open the file $logname for write");
         return(1);
      }
   }

   my $tm = localtime;
   print FH "$tm ($$) $snd_rcv: $msg";
   $|=1;  #Flush

   return(0);
}

#############################################################################
my $count = 0;
sub create_msgid {
   use Time::HiRes;

   my $tm = Time::HiRes::time;
   my $tt = sprintf("%.6f", $tm);
   $tt =~ s/\.//;  #remove '.'

   $count += 1;
   if ($count > 999999) {
      $count = 1;
   } 

   return($tt."-".$count);
}

my $lastE = 1;
