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
 * Updated: Blair Wolfinger, 10/29/12.  Setting up filemonConfig for creating checksum/file config file.
 *    11/4-5/12.  Updates for creating config file, including adding to correct directory.
 */

#include <stdio.h>

#define CONFIGFILE "/etc/ae/exports/fileMonConfigFile"
#define CONFIGFILECHKSUM "/etc/ae/exports/fileMonConfigFileChkSum"


/*
 * Calculate checksum and write to file "/etc/ae/exports/fileMonConfigFileChkSum.  Copied over Todds code and made some changes
 *  for my needs.
 * This function will calculate the checksum, then update the fileMonConfigChkSum file in /etc/ae/exports directory.
 * This file will be added to the installation package.  After installation and user is put into chroot, the
 *  user will be asked to run this executable, which will read fileMonConfigFile and create fileMonConfigFileChkSum
 */
int create_checksum_filemon(char *file_name, FILE *chksumFH)
{
   FILE *fp;
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

/*
 * Function:  main
 *   filemonConfig is a stand-alone utility which the user will run when they would like to update
 *    the critical files to monitor.  It relies on the file fileMonConfigFile being present in /etc/ae/exports,
 *    which is copied during installation.
 */

int main(void)
{
	FILE *configFile;
	FILE *configFileChkSum;
	char line[256];
	char cmd[500];


	configFile = fopen(CONFIGFILE, "r");  //open config file for reading list of critical files

	if (configFile == NULL)
	{
		printf("Error opening File Monitor Config File!\n");
		return 1;
	}

	configFileChkSum = fopen(CONFIGFILECHKSUM, "w+"); //open file for writing file/chksum information.

	if (configFileChkSum == NULL)
	{
		printf("Error opening File Monitor Checksum File!\n");
		return 1;
	}

    /*
     * Read each line of the file, and call create_checksum_filemon function when data is present.
     *   For protoype, not checking for valid data.
     */
	while ( fgets ( line, sizeof line, configFile) != NULL )
	{
//		sprintf(cmd, "%s", line);
		snprintf(cmd, sizeof(cmd), "%s", line);
		create_checksum_filemon(cmd, configFileChkSum);
	}


	fclose(configFile);
	fclose(configFileChkSum);

	return(0);
}
