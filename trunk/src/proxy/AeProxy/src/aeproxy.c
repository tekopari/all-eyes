/*
 * Thomas Pari
 * This program launches the java ae proxy after dropping privileges
 */

#include <stdio.h>  
#include <ulimit.h>
#include <sys/types.h>
#include <unistd.h>
#include <errno.h>

int main(int argc, char **argv)
{
   /*
    * BECOME THE MEMBER OF THE GROUP WHO GID BIT IS SET ON THE PROGRAM
    */
   uid_t someundefined = 17771;
   if(0 != setgid(someundefined)) {
       fprintf(stderr, "[ERROR] Failed to setgid\n");
       return(1);
   }

   /*
    * BECOME THE USER WHO USER ID BIT IS SET ON THE PROGRAM
    */
   if(0 != setuid(someundefined)) {
       fprintf(stderr, "[ERROR] Failed to setuid\n");
       return(1);
   }

   /*
    * LAUNCH THE JAVA TO RUN THE SANDBOX 
    */
   int rc = execlp("/usr/bin/java",
                   "/usr/bin/java",
                   "-jar",
                   "/usr/local/bin/AeProxy.jar",
                   "127.0.0.1",
                   "6000",
                   NULL);  
   printf("[ERROR] Failed to exec the ae proxy rc=%d, errno=%d\n", rc, errno);  
   return errno;  
}
