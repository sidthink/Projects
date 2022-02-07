#include "contiki.h"
#include "cfs/cfs.h"
#include <stdio.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

 
 PROCESS(coffee_test_process, "Coffee test process");
 AUTOSTART_PROCESSES(&coffee_test_process);
 
 PROCESS_THREAD(coffee_test_process, ev, data)
 {
   PROCESS_BEGIN();
     
   /* step 1 */
   char message[32];
   char buf[100];
   int fd_read;
   
   
 strcpy(buf,"empty string");
 fd_read = cfs_open("saved.ann", CFS_READ);
 if(fd_read!=-1) {
   cfs_read(fd_read, buf, sizeof(message));
   printf("step 3: %s\n", buf);
   cfs_close(fd_read);
 } else {
   printf("ERROR: could not read from memory in step 3.\n");
 }
        
   PROCESS_END();
 }