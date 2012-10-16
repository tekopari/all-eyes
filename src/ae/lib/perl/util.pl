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
#     Perl utility file contains: (1) protocol op code list, (2) socket functions,
#     (3) encryption/decryption functions, (4) common IO functions
######################################################################################################

use strict;

use FindBin qw($Bin $Script);
use Cwd qw(getcwd abs_path);
my $Bin = abs_path($Bin);
use lib "$FindBin::Bin";

my $debug = 1;

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
my $mcode = "";  #monitor code
my $encrytion_flag = 0;

#############################################################################
sub register_clear {
   $mname = "";
   $mcode = "";
   $encrytion_flag = 0;
}

#############################################################################
sub register_monitor {
   my($mon_name, $code, $crypto_flag) = @_;

   if (length($mon_name) <= 0) {
      my_util_print("Monitor name can't be zero length!");
      return(1);
   }

   $mname = $mon_name;
   $mcode = $code;
   if ($crypto_flag > 0) {
      $encrytion_flag = 1;
   }

   return(0);
}

#############################################################################
sub check_monitor_name {
   if (length($mname) <= 0) {
      my_util_print("Invalid monitor name '$mname'");
      return(1);
   }
   return(0);
}

#############################################################################
sub send_init {
   my($sock, $text) = @_;

   if (check_monitor_name() != 0) {
      return(1);
   }

   my $msg = $s.$d.$P_VER.$d.$P_TYPE_EVENT.$d.$mname.$d.$P_EVENT_INIT.$d.$P_STATUS_GREEN.$d.$P_ACTION_IGNORE.$d.$text.$d.$e;
   socket_send($sock, $msg);
   return(0);
}

#############################################################################
sub send_hello {
   my($sock) = @_;

   if (check_monitor_name() != 0) {
      return(1);
   }

   my $msg = $s.$d.$P_VER.$d.$P_TYPE_HELLO.$d.$mname.$d.$e;
   socket_send($sock, $msg);
   return(0);
}

#############################################################################
sub send_event {
   my($sock, $event, $status, $text) = @_;

   my $st = "";

   if (check_monitor_name() != 0) {
      return(1);
   }
   if (length($event) <= 0) {
      my_util_print("Invalid event code '$event'");
      return(1);
   }

   if ($status eq "GREEN") {
      $st = "00";
   } elsif ($status eq "ORANGE") {
      $st = "01";}
   elsif ($status eq "RED") {
      $st = "11";}
   else {
      my_util_print("Invalid status code '$status'");
      return(1);
   }

   my $msg = $s.$d.$P_VER.$d.$P_TYPE_EVENT.$d.$mname.$d.$event.$d.$st.$d.$P_ACTION_IGNORE.$d.$text.$d.$e;
   socket_send($sock, $msg);
   return(0);
}

#############################################################################
sub send_ack {
   my($sock) = @_;

   if (check_monitor_name() != 0) {
      return(1);
   }

   my $msg = $s.$d.$P_VER.$d.$P_TYPE_ACK.$d.$mname.$d.$e;
   socket_send($sock, $msg);
}

#############################################################################
sub receive_ack_check {
   my($sock) = @_;

   my $msg = socket_receive($sock);
   return (ack_check($msg));
}

#############################################################################
sub ack_check {
   my($msg) = @_;

   my $loc_msg = $P_VER.$d.$P_TYPE_ACK.$d.$P_NAME_AE;

   if ($loc_msg ne $msg) {
      my_util_print("Expect $s$d$loc_msg$d$e but received $msg");
      return(1);
   }

   return(0);
}


#############################################################################
sub debug_print {
   my($msg) = @_;

   if ($debug == 1) {
      print($msg);
   }
}

#############################################################################
sub my_util_print {
   my($msg) = @_;

   print("$0: $msg\n");
}

#############################################################################
sub socket_connect {
   my($ip, $tcp_port, $pSock) = @_;

   my $sock = 0;

   use IO::Socket;
   $sock = new IO::Socket::INET (
                PeerAddr => "$ip",
                PeerPort => "$tcp_port",
                Proto => 'tcp',
                );

   if ($sock) {
      $$pSock = $sock;
      my_util_print("Connected to IP=$ip, port=$tcp_port");
      return(0);
   }

   my_util_print("Failed to connect to IP=$ip, port=$tcp_port");
   return(1);
}

#############################################################################
sub socket_listen {
   my($ip, $tcp_port, $pSock) = @_;

   my $sock = 0;

   use IO::Socket;
   $sock = new IO::Socket::INET (
                LocalHost => $ip,       #listening address
                LocalPort => $tcp_port,
                Proto => 'tcp',
                Listen => 1,
                Reuse => 1,
                );

   if ($sock) {
      $$pSock = $sock;
      my_util_print("Listen on port $tcp_port");
      return(0);
   }

   my_util_print("Failed to listen on port $tcp_port");
   return(1);
}

#############################################################################
sub socket_receive {
   my($sock) = @_;

   my $inbuf = <$sock>;   #receive message from socket
   debug_print("RCV:$inbuf\n");
   my $buf = aescrypto("d", $inbuf);

   my @tok = split(/$d/, $buf);
   if (($tok[0] eq $s) || ($tok[$#tok] =~ /$e/)) { 
      my $loc_buf = "";
      for (my $i = 1;  $i < $#tok; $i++) {
         $loc_buf .= $tok[$i] . $d;
      }
      chop($loc_buf);
      return($loc_buf);
   }
   else {
      my_util_print("Received invalid protocol message: '$buf'");
      return($buf);
   }
}

#############################################################################
sub socket_send {
   my($sock, $buf) = @_;

   my $outbuf = aescrypto("e", $buf);

   debug_print("SND:$outbuf\n");
   print $sock "$outbuf";    #send message on socket
}

#############################################################################
sub socket_close {
   my($sock) = @_;

   close($sock);
}

#############################################################################
sub socket_accept {
   my($sock_listen, $timeout, $pSock) = @_;

   use IO::Select;

   my $sock_select = new IO::Select();
   $sock_select->add($sock_listen);

   my $cnt = $timeout;
   my $mm = 1;
   if ($cnt == -1) {
      $mm = 0;
      $cnt = 1;
   }

   while ($cnt > 0) {
      $cnt -= $mm;
      my($sock_list) = IO::Select->select($sock_select, undef, undef, 1); #every second
      foreach my $sock (@$sock_list) {
         if ($sock == $sock_listen) {       #Accept new connection
            my $sock_new = $sock->accept();
            if ($sock_new) {
               my_util_print("Accept new connection");
               $$pSock = $sock_new;
               return(0);
            }
            else {
               my_util_print("Failed to accept connection");
               return(1);
            }
         }
      }
   }

   return(1);
}

#############################################################################
my $socket_select_return = 0;

sub socket_select {
   my($work_sock, $receive_sub, $monitor_sub) = @_;

   use IO::Select;
   my $sock_select = new IO::Select();
   $sock_select->add($work_sock);

   $socket_select_return = 0;
   while (1) {
      my($sock_list) = IO::Select->select($sock_select, undef, undef, 1);   #every second
      foreach my $sock (@$sock_list) {
         if ($sock == $work_sock) {
            my $buff = socket_receive($sock);
            if (! $buff) {
               my_util_print("Connection closed by remote");
               return(1);
            }

            if (length($receive_sub) > 0) {
               my $cmd = "$receive_sub(\$work_sock, \"$buff\")";
               eval "$cmd";     #execute receiver subroutine
            } 
         } 
      }
      if (length($monitor_sub) > 0) {
         my $cmd = "$monitor_sub(\$work_sock)";
         eval "$cmd";           #execute monitor subroutine
      }
      if ($socket_select_return == 1) {
         return(0);
      }
   }

   my_util_print("Shouldn't be here!");
   return(1);
}

#############################################################################
# Sending a verificatoin message to remote
#############################################################################
my $verify_flag = 0;
sub socket_verify {
   my($work_sock, $text) = @_;

   send_init($work_sock, $text);

   socket_select($work_sock, "_verify_receive", "");

   if ($verify_flag == 0) {
      return(0);     #received the correct response
   }
   return(1);     #received wrong response
}
sub _verify_receive {
   my($work_sock, $buff) = @_;

   if (ack_check($buff) == 0) {
      $verify_flag = 0;
   }
   else {
      $verify_flag = 1;
   }

   $socket_select_return = 1;
}

#############################################################################
sub aescrypto {
   my($mode, $buf) = @_;

   if ($encrytion_flag == 1) {
      my $ifile = get_random_name();
      my $ofile = get_random_name();

      system("echo \"$buf\" > $ifile");
      if ($mode eq "e") {   #encrypt
         if (system("aescrypt -e -p$mcode -o $ofile $ifile") != 0) {
            my_util_print("Encrypt message '$buf' failed");
            return("");
         }
      }
      elsif ($mode eq "d") {   #decrypt
         if (system("aescrypt -d -p$mcode -o $ofile $ifile") != 0) {
            my_util_print("Decrypt message '$buf' failed");
            return("");
         }
      }
      else {
         my_util_print("Invalid crypto mode '$mode' for message '$buf'");
         return("");
      }
      my $loc_buf = `cat $ofile`;
      debug_print("AES:$loc_buf\n");
      
      unlink($ifile);
      unlink($ofile);
      return($loc_buf);
   } 
   else {
      return($buf);
   }
}

#############################################################################
sub get_random_name {
   my $name = $mname . rand($$);
   return($name);
}

my $lastE = 1;
