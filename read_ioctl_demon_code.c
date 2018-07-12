/*
 * libusb example program to list devices on the bus
 * Copyright (C) 2007 Daniel Drake <dsd@gentoo.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */


#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <linux/ioctl.h>
#define USB_PORT_STATE "/dev/auxdetectdrv"
#define IOCTL_AUX_DETECTGPIO _IOR('M', 0x7, unsigned)
int main(void)
{
 int fd;
// char mode[]="0";
int read_bytes =0;
unsigned long current_value =3;
unsigned long mode;

        fd = open(USB_PORT_STATE, O_RDWR);
        if(fd < 0)
        {
                printf("open %s failed",USB_PORT_STATE);
                return -1;
        }

        ioctl(fd,IOCTL_AUX_DETECTGPIO,&current_value);
        printf("usb current status =%d \n",(int)current_value);
        
     while(1)
      {
          read_bytes =read(fd,&mode, sizeof(unsigned long));

          printf(" read /dev/auxdetectdrv:%d   bytes=%d\n ",(int)mode,read_bytes);
          if( mode == 1)
           {
                printf("usb host status ok \n");
               
           }

          if( mode == 0)
           {
                printf("usb host status over-current \n");
                
           }
      }
        close(fd);

        return 0;	



}

