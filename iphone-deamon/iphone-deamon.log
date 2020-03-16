/*
 * libusb-0.1 example program
 * Copyright (C) 2008 Daniel Drake <dsd@gentoo.org>
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
#include "usb.h"
#include <unistd.h>
#define msleep(msecs) usleep(1000*msecs)


#define IPHONE_VENDER_ID 0x05ac
#define IPHONE_PRODUCT_ID  0x12a8

/*
mode =1 for host mode
mode =0 for device mode
*/
static void set_usb_prot_mode(int mode)
{
    int writeLen = 0;
	
     if (mode == 1) {
            FILE *pFile = fopen("/sys/atc2ctl/mode", "wb");
            if (NULL != pFile) {
                  writeLen = fwrite("host", sizeof("host"), 1, pFile);
                  fclose(pFile);
                  printf("open \"/sys/atc2ct1/mode\" to write host\n");
               } else {
                     printf("open \"/sys/atc2ct1/mode\" host fail \n");
               }

     } else if (mode == 0) {
        FILE *pFile = fopen("/sys/atc2ctl/mode", "wb");
        if (NULL != pFile) {
                 writeLen = fwrite("otg", sizeof("otg"), 1, pFile);
                 fclose(pFile);
                 printf("open \"/sys/atc2ct1/mode\" to write otg\n");
        } else {
                     printf("open \"/sys/atc2ct1/mode\" otg fail \n");
                }
       }
}


static void  enable_gadget_function(int enable)
{
    int writeLen = 0;
	
     if (enable == 1) {
            FILE *pFile = fopen("/sys/class/android_usb/android0/enable", "wb");
            if (NULL != pFile) {
                  writeLen = fwrite("1", sizeof("1"), 1, pFile);
                  fclose(pFile);
                  printf("open \"/sys/class/android_usb/android0/enable\" to write 1\n");
               } else {
                     printf("open \"/sys/class/android_usb/android0/enable\" 0 fail \n");
               }

     } else if (enable == 0) {
        FILE *pFile = fopen("/sys/class/android_usb/android0/enable", "wb");
        if (NULL != pFile) {
                 writeLen = fwrite("0", sizeof("0"), 1, pFile);
                 fclose(pFile);
                 printf("open \"/sys/class/android_usb/android0/enable\" to write 0\n");
        } else {
                     printf("open \"/sys/class/android_usb/android0/enable\" 0 fail \n");
                }
       }
}
/*
function->0 rndis
function->1 iap_ncm_eap
*/
static void  set_gadget_function(int function)
{
    int writeLen = 0;
	
     if (function == 1) {
            FILE *pFile = fopen("/sys/class/android_usb/android0/functions", "wb");
            if (NULL != pFile) {
                  writeLen = fwrite("iapncm", sizeof("iapncm"), 1, pFile);
                  fclose(pFile);
                  printf("open \"/sys/class/android_usb/android0/functions\" to write iapncm\n");
               } else {
                     printf("open \"/sys/class/android_usb/android0/functions\" to write  iapncm fail \n");
               }

     } else if (function == 0) {
        FILE *pFile = fopen("/sys/class/android_usb/android0/functions", "wb");
        if (NULL != pFile) {
                 writeLen = fwrite("rndis", sizeof("rndis"), 1, pFile);
                 fclose(pFile);
                 printf("open \"/sys/class/android_usb/android0/functions\" to write rndis\n");
        } else {
                     printf("open \"/sys/class/android_usb/android0/functions\" to write rndis fail \n");
                }
       }
}

static void request_role_switch(struct usb_device *dev)
{
    usb_dev_handle *udev;
    char buf[256];
    int ret =-1;
    udev=usb_open(dev);
    ret = usb_detach_kernel_driver_np(udev,0);

    if(ret !=0)
    	{
            printf("usb_detach_kernel_driver_np interface 0 fail \n");
	}
	
      ret = usb_claim_interface(udev,0);
	  if(ret !=0)
	{
              printf("usb_claim_interface 0 fail \n");
	 }

    /*usb_dev_handle *dev, int requesttype, int request, int value, int index, char *bytes, int size, int timeout*/
    printf("request_role_switch begin \n");
    ret =usb_control_msg(udev,0x40,0x51,0x01,0x0,NULL,0,1000);
    printf("request_role_switch ret=0x%04x\n",ret);

    if (udev)
        usb_close(udev);
    return 0;

}

int main(void)
{
	struct usb_bus *busses;
	struct usb_bus *bus;
	struct usb_device *iphone_dev =NULL;
       int  device_found = 0;
       set_usb_prot_mode(1);
	enable_gadget_function(0);
	msleep(1000);
	   
	usb_init();
	usb_find_busses();
	usb_find_devices();

	busses = usb_get_busses();
	for (bus = busses; bus; bus = bus->next) {
		struct usb_device *dev;
			
		for (dev = bus->devices; dev; dev = dev->next) {
			printf("%04x:%04x\n",
				dev->descriptor.idVendor, dev->descriptor.idProduct);
			if((dev->descriptor.idVendor == IPHONE_VENDER_ID)&&(dev->descriptor.idProduct ==IPHONE_PRODUCT_ID))
				{
                              iphone_dev = dev;
				  device_found =1;
				  printf("iphone device has been detected %04x:%04x\n",dev->descriptor.idVendor, dev->descriptor.idProduct);
				  break ;
			       }
			
		}
		if(device_found)
			break ;
	}

      if(device_found &&(iphone_dev !=NULL))
      	{
              request_role_switch(iphone_dev);
		//printf("set functions as iap_ncm \n");
	     //  set_gadget_function(1);
	     //  msleep(1);
	}
       	//printf("enable functions \n");
	//enable_gadget_function(1);
	//msleep(500);
	//printf("HU switch to device mode \n");
     //  set_usb_prot_mode(0);
	msleep(1000);
	return 0;
}

