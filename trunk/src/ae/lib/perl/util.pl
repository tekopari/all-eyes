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
my $P_ACTION_RESTART = "A1";
my $P_ACTION_KILL = "A2";

my $s = "\[";  #protocol start marker
my $e = "\]";  #protocol end marker
my $d = ":";   #protocol delimeter

my $mname = "";  #monitor name

#############################################################################
sub register_clear {
   $mname = "";
}

#############################################################################
sub register_monitor {
   my($mon_name) = @_;

   if (length($mon_name) <= 0) {
      util_print_err("Monitor name can't be zero length!");
      return(1);
   }

   $mname = $mon_name;
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
      return(1);
   }

   my $msg = $s.$d.$P_VER.$d.$P_TYPE_EVENT.$d.$mname.$d.$P_EVENT_INIT.$d.$P_STATUS_GREEN.$d.$P_ACTION_IGNORE.$d.$text.$d.$e;
   _send($msg);
   return(0);
}

#############################################################################
sub send_hello {
   if (check_monitor_name() != 0) {
      return(1);
   }

   my $msg = $s.$d.$P_VER.$d.$P_TYPE_HELLO.$d.$mname.$d.$e;
   _send($msg);
   return(0);
}

#############################################################################
sub send_event {
   my($event, $status, $text) = @_;

   my $st = "";

   if (check_monitor_name() != 0) {
      return(1);
   }
   if (length($event) <= 0) {
      util_print_err("Invalid event code '$event'");
      return(1);
   }

   if ($status eq "GREEN") {
      $st = "00";
   } elsif ($status eq "ORANGE") {
      $st = "01";}
   elsif ($status eq "RED") {
      $st = "11";}
   else {
      util_print_err("Invalid status code '$status'");
      return(1);
   }

   my $msg = $s.$d.$P_VER.$d.$P_TYPE_EVENT.$d.$mname.$d.$event.$d.$st.$d.$P_ACTION_IGNORE.$d.$text.$d.$e;
   _send($msg);
   return(0);
}

#############################################################################
sub send_ack {
   if (check_monitor_name() != 0) {
      return(1);
   }

   my $msg = $s.$d.$P_VER.$d.$P_TYPE_ACK.$d.$mname.$d.$e;
   _send($msg);
}

#############################################################################
sub receive_ack_check {
   my $msg = _receive();
   return (ack_check($msg));
}

#############################################################################
sub ack_check {
   my($msg) = @_;

   my $loc_msg = $s.$d.$P_VER.$d.$P_TYPE_ACK.$d.$P_NAME_AE.$d.$e;

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
   print STDOUT "$msg\n";
}

#############################################################################
sub _receive {
   my $msg = <STDIN>;
   chomp($msg);
   return($msg);
}

my $lastE = 1;
