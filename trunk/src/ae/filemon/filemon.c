/*
 * Copyright (C) <2012> <Blair Wolfinger, Ravi Jagannathan, Thomas Pari, Todd Chu>
 *is ca
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this
 * software and associated documentation files (the "Software"), to deal in the Software
 * without restriction, including without limitation the rights to use, copy, modify,
 * merge, publish, distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to the following conditions:

 * The above copyright notice and this permission notice shall be included in all copies
 * or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,
 * INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR
 * PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR
 * ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * Original Author: Ravi Jagannathan
 * Updated: Blair Wolfinger, 10/27/12.  Setting up filemon for connection to ae daemon.  Following 
 *       instructions from wiki for creating Monitor (copying template, in this case used selfmon as template.
 * Updated: Blair Wolfinger, 10/28 - 11/5, 2012.  Adding code to support actual monitoring of files.
 *        For prototype, we will be monitorring /etc/hosts and /etc/passwd 'within' the chroot environment.
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <unistd.h>
#include <fcntl.h>
#include <time.h>
#include <ctype.h>
#include <string.h>
#include <syslog.h>
#include <errno.h>
#include <pwd.h>
#include <signal.h>
#include <dirent.h>
#include <pthread.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/resource.h>
#include <sys/file.h>
#include <sys/select.h>
#include <sys/stat.h>
#include "ae.h"


#define BUFSIZE 1024
#define CONFIGFILECHKSUM "/etc/ae/exports/fileMonConfigFileChkSum"

//define prototypes.
int cal_checksum_filemon(char *file_name, char *chksum);
int verifyCheckSum(void);



/* function: fileMon:  Monitor (sha256sum) checksum of critical files.
 *    For prototype will monitor just two files:
 *         1. /etc/hosts
 *         2. /etc/passwd
 *
 * bew.10/10/2012.  Test checking via svn
 *     up to 11/6/2012.  Added code to monitor sha256sum
 * Mode tells the monitor whether it is going to
 * operate in PERSISTENT or VOLATILE mode.  Only VOLATILE supported
 * in prototype.
 */
void fileMon(int mode)
{
static char sbuf[BUFSIZE];
static char *msg="[:10:00:FM:]";
static char *msg1="[:10:22:FM:0003:11:A1:filemon_chksum_error:]";
int ret = -1, err = 0;

    memset(sbuf, 0, BUFSIZE);

    aeLOG("filemon called");

    //change priority of process to slow it down.  If error, exit.
    err = setpriority(PRIO_PROCESS, 0, 19);
    if( err != 0 )
      {
    	aeLOG("setPriority failed\n");
    	exit(1);
       }

    while (1)  {
        write(1, msg, strlen(msg));   //Send hello message
        memset(sbuf, 0, BUFSIZE);
        while (1)  {
           sleep(5);   //sleep to avoid sending too many messages.
           ret = read(0, sbuf, BUFSIZE); 
           if (ret < 0)  {
             	aeLOG("read failed!  ....exiting");
             	exit(1);
           } else if ( ret > 0)  {   //Assume hello ack from daemon. TBD: Check for actual HELLO msg from daemon
               ret = verifyCheckSum();
               if (ret < 0 )
               {
            	   aeLOG("filemon, checksum checking failed");  //for prototype will assume checksum error.
            	                                                //however, could be a file read error.
            	   write(1, msg1, strlen(msg1));
            	   sleep(15);
               }
               break;
           }
        }
    }
}

/*
 * Function: verifyCheckSum
 *       Check sha256sum of critical files (defined in fileMonConfigFile).  In prototype
 *       will not specifically handle file read errors, althrough they will be reported in syslog.
 */
int verifyCheckSum()
{
	FILE *configFileChkSum;
	char line[256];
	char cmd[500];
    char *token;
    char *chksum;
    char *filename;
    int cnt = 0;

  	configFileChkSum = fopen(CONFIGFILECHKSUM, "r");

	if (configFileChkSum == NULL)
	{
		aeLOG("Error opening File Monitor Checksum File!\n");
		return (-1);
	}


	while ( fgets ( line, sizeof line, configFileChkSum) != NULL )
	{
		sprintf(cmd, "%s", line);

		token = strtok(cmd, " ");

		cnt = 0;
		while(token != NULL)
		{
			switch(cnt)
			{
			case 0:
				chksum = token;
				break;
			case 1:
				filename = token;
				if(cal_checksum_filemon(filename, chksum))
				{
				   	aeLOG("cal_checksum_filemon failed!");
					return (-1);
				}
			}
			token = strtok(NULL, " ");
			cnt++;
		}
	}
	return(0);
}

/*
 * Calculate checksum and compare to one from file.  Copied over Todds code and made some changes
 *  for my needs.
 * This function will calculate the checksum, then compare to checksum saved in file in /etc/ae/exports directory.
 */
int cal_checksum_filemon(char *file_name, char *chksum)
{
   FILE *fp;
   char buf[512];
   char cmd [512];
   char *token;

   snprintf(cmd, 512, "sha256sum -t %s", file_name); //setup execution of sha256sum cmd

   fp = popen(cmd, "r");
   if (fp == NULL) { return(1); }

   fgets(buf, 512, fp);
   token = strtok(buf, " ");
   if(strncmp(chksum, token, 64))
   {
	  aeLOG("filemon: stncmp failed");
      fclose(fp);
      return(-1);
   }

   fclose(fp);

   return(0);
}




