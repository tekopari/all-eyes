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
# Protocol op-code between monitor and ae
my $REQ_INIT    = "REQ_INIT";
my $REQ_HELLO   = "REQ_HELLO";
my $REQ_PROBLEM = "REQ_PROBLEM";
my $REQ_CLEAR   = "REQ_CLEAR";

my $RES_INIT    = "RES_INIT";
my $RES_HELLO   = "RES_HELLO";
my $RES_PROBLEM = "RES_PROBLEM";
my $RES_CLEAR   = "RES_CLEAR";

my $protocol_end = ";";

my $lastE = 1;

#############################################################################
# Functions return the protocol op-code code
sub req_hello   {return($REQ_HELLO);}
sub req_init    {return($REQ_INIT);}
sub req_problem {return($REQ_PROBLEM);}
sub req_clear   {return($REQ_CLEAR);}
sub res_hello   {return($RES_HELLO);}
sub res_init    {return($RES_INIT);}
sub res_problem {return($RES_PROBLEM);}
sub rresclear   {return($RES_CLEAR);}

#############################################################################
sub debug_print {
   my($msg) = @_;

   if ($debug == 1) {
      print($msg);
   }
}

#############################################################################
sub my_print {
   my($msg) = @_;

   print("$0: $msg\n");
}

#############################################################################
sub my_exit {
   my($mode) = @_;

   my_print("Exit!");
   exit($mode);
}


#############################################################################
# SSL certificate/key files and the CA file
#############################################################################
my $ssl_flag = 0;
my $cert_file = "";
my $key_file = "";
my $ca_file = "";
my $password = "";

#############################################################################
sub socket_use_ssl {
   my($cert, $key, $ca, $pw) = @_;

   $cert_file = $cert;
   $key_file = $key;
   $ca_file = $ca;
   $password = $pw;

   $ssl_flag = 1;  #enable SSL connection
}

#############################################################################
sub socket_connect {
   my($ip, $tcp_port, $pSock) = @_;

   my $sock = 0;

   if ($ssl_flag == 0) {
      use IO::Socket;
      $sock = new IO::Socket::INET (
                   PeerAddr => "$ip",
                   PeerPort => "$tcp_port",
                   Proto => 'tcp',
                   );
   }
   else {
      use IO::Socket::SSL;
      $sock = IO::Socket::SSL->new (
                   PeerAddr => "$ip",
                   PeerPort => "$tcp_port",
                   Proto => 'tcp',
                   SSL_use_cert => 1,
                   SSL_verify_mode => 0x01,
                   SSL_cert_file => $cert_file,
                   SSL_key_file => $key_file,
                   SSL_ca_file => $ca_file,
                   SSL_passwd_cb => sub {return $password},
                   );
      warn "$0: ", &IO::Socket::SSL::errstr, "\n";
   }

   if ($sock) {
      $$pSock = $sock;
      my_print("Connected to IP=$ip, port=$tcp_port");
      return(0);
   }

   my_print("Failed to connect to IP=$ip, port=$tcp_port");
   return(1);
}

#############################################################################
sub socket_listen {
   my($ip, $tcp_port, $pSock) = @_;

   my $sock = 0;

   if ($ssl_flag == 0) {
      use IO::Socket;
      $sock = new IO::Socket::INET (
                   LocalHost => $ip,       #listening address
                   LocalPort => $tcp_port,
                   Proto => 'tcp',
                   Listen => 1,
                   Reuse => 1,
                   );
   }
   else {
      use IO::Socket::SSL;
      $sock = IO::Socket::SSL->new (
                   LocalHost => "0.0.0.0",  #any address
                   LocalPort => $tcp_port,
                   Proto => 'tcp',
                   Listen => 1,
                   Reuse => 1,
                   SSL_verify_mode => 0x01,
                   SSL_cert_file => $cert_file,
                   SSL_key_file => $key_file,
                   SSL_ca_file => $ca_file,
                   SSL_passwd_cb => sub {return $password},
                   );
      warn "$0: ", &IO::Socket::SSL::errstr, "\n";
   }

   if ($sock) {
      $$pSock = $sock;
      my_print("Listen on port $tcp_port");
      return(0);
   }

   my_print("Failed to listen on port $tcp_port");
   return(1);
}

#############################################################################
sub socket_receive {
   my($sock) = @_;

   my $buf = <$sock>;   #receive message from socket
   debug_print("RCV:$buf\n");

   my($loc_buf) = split(/$protocol_end/, $buf);
   return($loc_buf);
}

#############################################################################
sub socket_send {
   my($sock, $buf) = @_;

   my $loc_buf = $buf . $protocol_end;
   debug_print("SND:$loc_buf\n");

   print $sock "$loc_buf";    #send message on socket
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
               my_print("Connection Accepted");
               $$pSock = $sock_new;
               return(0);
            }
            else {
               my_print("Unable to accept connection");
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
               my_print("Connection closed by remote");
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

   my_print("Shouldn't be here!");
   return(1);
}

#############################################################################
my $verify_flag = 0;

sub socket_verify {
   my($work_sock) = @_;

   my $buf = req_init();
   socket_send($work_sock, $buf);

   socket_select($work_sock, "_verify_receive", "");

   if ($verify_flag == 0) {
      return(0);     #received the correct response
   }

   my_print("Failed peer verificatoin");
   return(1);     #received wrong response
}

sub _verify_receive {
   my($work_sock, $buff) = @_;

   if ($buff ne res_init()) {
      $verify_flag = 1;
   }
   else {
      $verify_flag = 0;
   }

   $socket_select_return = 1;
}
my $lastE = 1;
