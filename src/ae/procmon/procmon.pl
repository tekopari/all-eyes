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
require("$Bin/util.pl");

#my $debug = 1;
my $debug = 0;

#############################################################################
# DATA AND RUN
#############################################################################
my $PROC_LIST = "proc_name";
my $MON_TM_LIMIT = 10;    #send out same  message every N sec

my @proc_list = qw();
my %save_send_buf = qw();
my $save_bad_proc_list = "";
my $deli = "_";
my $monitor_name = "PM";
my $syscmd = "ps";

main();
my_exit(0);

#############################################################################
# FUNCTIONS
#############################################################################
sub debug_print {
   my($msg) = @_;

   if ($debug == 1) {
      print STDERR "$msg";
   }
}

sub my_print {
   my($msg) = @_;
   print STDERR "$0: $msg\n";
}

#############################################################################
sub my_exit {
   my($code) = @_;

   register_clear();
   for my $key (sort keys (%save_send_buf)) {undef($save_send_buf{"$key"});}
   for (my $i = 0; $i <= $#proc_list; $i++) {$proc_list[$i] = "";}
   $save_bad_proc_list = "";

   exit($code);
}

#############################################################################
sub main {
   check_syscmd($syscmd);

   my $conf_name = $Bin . "/procmon_conf";
   if (read_conf($conf_name) != 0) {
      my_exit(1);
   }
   register_monitor($monitor_name, $debug);

   send_init($monitor_name);
   if (receive_ack_check() != 0) {
      my_exit(1);
   }

   while (1) {
      check_syscmd($syscmd);
      monitor($syscmd);
      sleep(1);
      dec_send_buf();
      debug_print(".");
   }

}

#############################################################################
sub dec_send_buf {
   foreach my $key (sort keys (%save_send_buf)) {
      if ($save_send_buf{"$key"} > 0) {
         $save_send_buf{"$key"} -= 1;
      }
      else {
         undef($save_send_buf{"$key"});
      }
   }
}

#############################################################################
sub check_send_buf {
   my($event, $status, $text) = @_;

   my $buf = $event . $deli . $status . $deli . $text;
   if ($save_send_buf{"$buf"} > 0) {
      return(1);
   }
   else {
      $save_send_buf{"$buf"} = $MON_TM_LIMIT;
   }
   return(0);
}

#############################################################################
sub tell_remote {
   my($event, $status, $text) = @_;

   if (check_send_buf($text) == 0) {
      if (length($event) == 0) {
         if (send_hello() != 0) {
            my_exit(1);
         }
      }
      else {
         if (send_event($event, $status, $text) != 0) {
            my_exit(1);
         }
      }
      if (receive_ack_check() != 0) {
         my_exit(1);
      }
   }
}

#############################################################################
sub monitor {
   my($syscmd) = @_;

   my @sensor_data = `$syscmd -ef`;

   my @full_name = qw();
   my $bad_proc_list = "";
   my $loc_save_bad = "";

   for (my $i = 0; $i <= $#sensor_data; $i++) {
      chomp($sensor_data[$i]);
      $sensor_data[$i] =~ s/\t//g;      #remove all tabs
      $sensor_data[$i] =~ s/( +)/ /g;   #replace mulitple spaces with one
      $sensor_data[$i] =~ s/^ //;       #remove leading space

      my @tok = split(/ /, $sensor_data[$i]);
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
         my $text = $a; 
         $bad_proc_list .= $a . ","; 
         tell_remote("0003", "RED", $text);
      }
   }

   #Compare list for problem cleared
   my @tok_bad = split(/,/, $bad_proc_list);
   my @tok_sav = split(/,/, $save_bad_proc_list);
   foreach my $m1 (@tok_sav) {
      my $flag = 0;
      foreach my $m2 (@tok_bad) {
         if ($m1 eq $m2) {
            $flag = 1;    #still bad
         }
      }
      if ($flag == 0) {
         tell_remote("0003", "GREEN", $m1);
      }
      else {
         $loc_save_bad .= $m1 . ",";
      }
   }
   $save_bad_proc_list = $loc_save_bad;

   #Send HELLO message
   tell_remote("", "", "");
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
