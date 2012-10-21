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
# File Name: sha256check.pl
# Description:
#     Calcaulates the SHA256 checksum for a given list of files. Then, it generates
#     the proper C code that does the SHA256 checksum verification before launching
#     the script process.
######################################################################################################

use strict;

my $marker = "int checksum_check";
my $marker2 = "checksum_check";

if ($ARGV[0] eq "help") {
   print("Usage: $0 <src_name> <des_name> <final_dir> <mode> <file>...<file>\n");
   print("Where\n");
   print("    <src_name> :The source C file name. There must be a single\n");
   print("                line function that starts with \"$marker(void)\"\n");
   print("                in the C file\n");
   print("    <des_name> :The destination C file name\n");
   print("   <final_dir> :The production directory where the <file> will\n");
   print("                be placed (i.e. /usr/bin/). for current directory,\n");
   print("                use \"\./\"\n");
   print("        <mode> :File mode for <file>. Note that the mode applies to\n");
   print("                all files specified by <file>. The defined modes:\n");
   print("                    -t   :Text file\n");
   print("                    -b   :Binary file\n");
   print("        <file> :The input file name for calculate the checksum\n");
   print("\n");
   exit(0);
}

my $f_index = 4;
if ($#ARGV < $f_index) {
   print("=== Not enough paramters.\n");
   print("=== No SHA256 CHECKSUM IS CALCULATED!\n");
   exit(1);
}

my $c_name = $ARGV[0];
my $c_new = $ARGV[1];
my $dir = $ARGV[2];
my $mode = $ARGV[3];
my @checksum_name = qw();
my @checksum_buf = qw();
my $cname = "";
my $cntname = 0;
my $maxsize = 0;

if (($mode ne "-t") && ($mode ne "-b")) {
   print("=== Unknown file mode '$mode'\n");
   print("=== No SHA256 CHECKSUM IS CALCULATED!\n");
   exit(1);
}

if ($c_name eq $c_new) {
   print("=== The SRC and DES file names can't be the same.\n");
   print("=== No SHA256 CHECKSUM IS CALCULATED!\n");
   exit(1);
}

my $cs_cmd = "sha256sum $mode";

if (open(FH, "$c_name")) {
   ($cname) = split(/\./, $c_name);
   $cname = "_" . $cname;
   if (($dir eq "\.") || ($dir eq "\./")) {
      $dir = "";
   }
   if (open(OH, ">$c_new")) {
      for (my $i = $f_index; $i <= $#ARGV;  $i++) {
         $checksum_name[$i-$f_index] = $ARGV[$i];

         if (! -e $checksum_name[$i-$f_index]) {
            print("=== File '$checksum_name[$i-$f_index]' doesn't exist\n");
            print("=== No SHA256 CHECKSUM IS CALCULATED!\n");
            exit(1);
         }
         my $c = `$cs_cmd $checksum_name[$i-$f_index]`;
         if (length($c) <= 0) {
            print("=== No able to generate a proper checksum string\n");
            print("=== No SHA256 CHECKSUM IS CALCULATED!\n");
            exit(1);
         }
         $checksum_buf[$i-$f_index] = $c;

         my $l = length($checksum_buf[$i-$f_index]);
         if ($l > $maxsize) {
            $maxsize = $l;
         }

         $cntname += 1;
      }
      while (<FH>) {
         if ($_ =~ /$marker/) {
            gen_c_code();
         }
         elsif ($_ =~ /$marker2/) {
            $_ =~ s/$marker2/$marker2$cname/;
            print OH $_;
         }
         else {
            print OH $_;
         }
      }
      close(OH);
   }
   close(FH);
}
exit(0);

#############################################################################
sub gen_c_code {
   if ($maxsize <= 512) {
      $maxsize = 512;
   }

   c_out("int cal_checksum$cname(char *file_name, char s[][$maxsize], int size)");
   c_out("{");
   c_out("   FILE *fp;");
   c_out("   int i = 0;");
   c_out("   char buf[$maxsize];");
   c_out("   char cmd [$maxsize];");
   c_out("");
   c_out("   snprintf(cmd, $maxsize, \"$cs_cmd \%s\", file_name);");
   c_out("");
   c_out("   fp = popen(cmd, \"r\");");
   c_out("   if (fp == NULL) { return(1); }");
   c_out("   fgets(buf, $maxsize, fp);");
   c_out("   fclose(fp);");
   c_out("");
   c_out("   for (i = 0; i < size; i++) {");
   c_out("      if (strcmp(strtok(s[i], \" \"), strtok(buf, \" \")) == 0) { return(i); } ");
   c_out("   }");
   c_out("   return(-1);");
   c_out("}");
   c_out("");
   c_out("int checksum_check$cname(void)");
   c_out("{");
   c_out("   char strs[$cntname][$maxsize];");
   c_out("");
   for (my $i = 0; $i <= $#checksum_buf; $i++) {
      chomp($checksum_buf[$i]);
      c_out("   snprintf(strs[$i], $maxsize, \"$checksum_buf[$i]\\n\");");
   }
   c_out("");
   for (my $i = 0; $i <= $#checksum_name; $i++) {
      c_out("   if (cal_checksum$cname(\"$dir$checksum_name[$i]\", strs, $cntname) == -1) { return(1); }");
   }
   c_out("");
   c_out("   return(0);");
   c_out("}");
}

#############################################################################
sub c_out {
   my($msg) = @_;

   print(OH $msg."\n");
}
