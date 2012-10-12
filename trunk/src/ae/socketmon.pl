#!/usr/bin/perl
#############################################################################
# File Name: socketmon.pl
# Description:
#     Monitor the active listening TCP/UDP ports
#############################################################################

use strict;
use FindBin qw($Bin $Script);
use Cwd qw(getcwd abs_path);
my $Bin = abs_path($Bin);

my $tcp_port = $ARGV[0];
my $seed = $ARGV[1];

my $timeout = 30;   #seconds
my $debug = 0;
my $title = $0;     #process name
my $conf_name = $Bin . "/socketmon_conf";

if ($#ARGV != 2) {
   print("******************* TEST RUN ONLY ********************\n");
   $tcp_port = 3456;
   $seed = 1;
   $timeout = -1;
}

my $WHITE_LIST = "white_port";
my $BLACK_LIST = "black_port";
my $PROTO_TCP = "tcp";
my $PROTO_UDP = "udp";
my $MON_TM_LIMIT = 30;  #send out same error message every 30 sec

my @white_list = qw();
my @black_list = qw();

if (read_conf($conf_name) != 0) {
   my_exit(1);
}

my $listen_sock = 0;
if (socket_listen($tcp_port, \$listen_sock) != 0) {
   my_exit(1);
}

my $work_sock = 0;
if (socket_accept($listen_sock, $timeout, \$work_sock) != 0) {
   close(listen_sock);
   my_exit(1);
}
close(listen_sock);

if (socket_select($work_sock, "receive_sub", "monitor_sub") != 0) {
   close(work_sock);
}

exit(0);


#############################################################################
sub receive_sub {
   my($sock, $buff) = @_;

   my_print("Receive data:[$buff]");

   my $myb = "<THANK YOU>";
   my_print("Send data:$myb");
   socket_send($sock, $myb);
}

#############################################################################
my $mon_result = "";
my $mon_tm = 0;

sub monitor_sub {
   my($sock) = @_;

   my @msg = `netstat  -l -n|grep -v LISTENING`;

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

      if ((length($proto) > 0) && ($port > 0)) {
         if (($proto eq $PROTO_TCP) || ($proto eq $PROTO_UDP)) {
            $loc_msg[1+$#loc_msg] = $proto . ":" . $port;
         }
      }
   }

   #Check the black list
   foreach my $m (@loc_msg) {
      my($proto, $port) = split(/:/, $m);
      foreach my $a (@black_list) {
         my($x, $y) = split(/:/, $a);
         debug_print("BLK:$x-$proto,$y-$port");

         if (($x eq $proto) && ($y == $port)) {
            $bad_black_list .= $proto . ":" . $port . ",";
            debug_print(":BAD");
         }
         debug_print("\n");
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
         debug_print("WHT:$x-$proto,$y-$port");

         if (($x eq $proto) && ($y == $port)) {
            $flag = 1;
            debug_print(":BAD");
         }
         debug_print("\n");
      }
      if ($flag == 0) {
         $bad_white_list .= $x . ":" . $y . ",";
      }
   }

   #Generate message
   if ((length($bad_black_list) > 0) || (length($bad_white_list) > 0)) {
      chop($bad_black_list);
      chop($bad_white_list);
      my $result = "SOCKETMON_PROBLEM:black(" . $bad_black_list ."),white(" . $bad_white_list . ")";


      if (($result ne $mon_result) || ($mon_tm == 0)) {
         $mon_tm = $MON_TM_LIMIT;
         $mon_result = $result;
         debug_print("SND:$mon_result\n");
         socket_send($sock, $mon_result);
      }
      $mon_tm -= 1;
   }
   else {
      if (length($mon_result) > 0) {
         my $result = "SOCKETMON_CLEAR";
         debug_print("SND:$result\n");
         socket_send($sock, $result);
      }
      $mon_result = "";
      $mon_tm  = 0;
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
      my_print("Syntax error at line $cnt of file '$fname'");
      return(1);
   }
   if (($proto ne $PROTO_TCP) && ($proto ne $PROTO_UDP)) {
      my_print("Syntax error at line $cnt of file '$fname'");
      return(1);
   }

   if ($mode eq $WHITE_LIST) {
      $white_list[1+$#white_list] = $proto.":".$port;
   }
   elsif ($mode eq $BLACK_LIST) {
      $black_list[1+$#black_list] = $proto.":".$port;
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

   print("$title: $msg\n");
}

#############################################################################
sub my_exit {
   my($mode) = @_;

   my_print("Exit!");
   exit($mode);
}

#############################################################################
sub socket_connect {
   my($ip, $tcp_port, $pSock) = @_;

   use IO::Socket;

   my $sock = new IO::Socket::INET (
                   PeerAddr => "$ip",
                   PeerPort => "$tcp_port",
                   Proto => 'tcp',
                   );
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
   my($tcp_port, $pSock) = @_;

   use IO::Socket;

   my $sock = new IO::Socket::INET (
                   LocalHost => "0.0.0.0",  #any address
                   LocalPort => $tcp_port,
                   Proto => 'tcp',
                   Listen => 1,
                   Reuse => 1,
                   );
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
   return($buf);
}

#############################################################################
sub socket_send {
   my($sock, $buff) = @_;

   print $sock "$buff";    #send message on socket
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
            my_print("Accept new connection");
            my $sock_new = $sock->accept();
            $$pSock = $sock_new;
            return(0);
         }
      }
   }

   return(1);
}

#############################################################################
sub socket_select {
   my($work_sock, $receive_sub, $monitor_sub) = @_;

   use IO::Select;
   my $sock_select = new IO::Select();
   $sock_select->add($work_sock);

   while (1) {
      my($sock_list) = IO::Select->select($sock_select, undef, undef, 1);   #every second
      foreach my $sock (@$sock_list) {
         if ($sock == $work_sock) {
            my $buff = socket_receive($sock);
            if (! $buff) {
               my_print("Connection closed by remote");
               return(1);
            }

            my $cmd = "$receive_sub(\$work_sock, \"$buff\")";
            eval "$cmd";     #execute receiver subroutine
         } 
      }
      my $cmd = "$monitor_sub(\$work_sock)";
      eval "$cmd";           #execute monitor subroutine
   }

   my_print("Shouldn't be here!");
   return(1);
}

