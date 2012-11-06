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
 */

#include <stdio.h>

#define CONFIGFILE "/etc/ae/exports/fileMonConfigFile"
#define CONFIGFILECHKSUM "/etc/ae/exports/fileMonConfigFileChkSum"


/*
 * Calculate checksum and write to file.  Copied over Todds code and made some changes
 *  for my needs.
 * This function will calculate the checksum, then update the file in /etc/ae directory.
 */
int cal_checksum_filemon(char *file_name, FILE *chksumFH)
{
   FILE *fp;
   int i = 0;
   char buf[512];
   char cmd [512];

   snprintf(cmd, 512, "sha256sum -t %s", file_name);

   fp = popen(cmd, "r");
   if (fp == NULL) { return(1); }

   fgets(buf, 512, fp);
   fprintf(chksumFH, "%s", buf);
   fclose(fp);

   return(-1);
}


int main()
{
	FILE *configFile;
	FILE *configFileChkSum;
	char line[256];
	char cmd[500];
    char strs[2][512];


	configFile = fopen(CONFIGFILE, "r");

	if (configFile == NULL)
	{
		printf("Error opening File Monitor Config File!\n");
		return 1;
	}

	configFileChkSum = fopen(CONFIGFILECHKSUM, "w+");

	if (configFileChkSum == NULL)
	{
		printf("Error opening File Monitor Checksum File!\n");
		return 1;
	}


	while ( fgets ( line, sizeof line, configFile) != NULL )
	{
		//fputs (line, stdout );
		sprintf(cmd, "%s", line);
		cal_checksum_filemon(cmd, configFileChkSum);
	}


	fclose(configFile);
	fclose(configFileChkSum);

}
