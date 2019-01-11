#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/time.h>
#include <time.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/fcntl.h>

int main(int argc, char **argv)
{

   int fd =0;
   char out_buf[100];
   char *str ="hello world";
   int num = 0;
   fd = open("/data/test.txt",O_RDWR | O_CREAT | O_APPEND,0677);
   while(1)
   {
     sprintf(out_buf,"%s.%d \n",str,num);
     write(fd,out_buf,100);
     num=num+1;
     fsync(fd);
     sleep(1);

   }
   close(fd);

   return 0;
}
