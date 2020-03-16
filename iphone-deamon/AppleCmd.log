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

#include "libusb.h"
#define msleep(msecs) usleep(1000*msecs)

#define    VENDOR_ID       0x05ac
#define    PRODUCT_ID      0x12a8

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


int do_roleswitch(void){
	libusb_device *dev;
	libusb_device_handle *handle;
	struct libusb_config_descriptor *conf_desc;
	int  descriptor_size;
	int iface, nb_ifaces, ret;

	ret = libusb_init(NULL);
	if (ret < 0)
	{
		printf("Fail to libusb_init\n");
		return ret;
	}
	
	printf("Opening device %04X:%04X...\n", VENDOR_ID, PRODUCT_ID);
	handle = libusb_open_device_with_vid_pid(NULL, VENDOR_ID, PRODUCT_ID);	
	if (handle == NULL) {
		printf("  Failed libusb_open_device_with_vid_pid .\n");
		return -1;
	}

	dev = libusb_get_device(handle);
	
	printf("\nReading first configuration descriptor:\n");
	libusb_get_config_descriptor(dev, 0, &conf_desc);
	nb_ifaces = conf_desc->bNumInterfaces;
	printf("             nb interfaces: %d\n", nb_ifaces);
	
	libusb_free_config_descriptor(conf_desc);
	
	if(libusb_detach_kernel_driver(handle, 0)<0)
	{
		printf("Fail to libusb_detach_kernel_driver\n");
	}
	
	for (iface = 0; iface < nb_ifaces; iface++)
	{
		printf("\nClaiming interface %d...\n", iface);
		ret = libusb_claim_interface(handle, iface);
		if (ret != LIBUSB_SUCCESS) {
			printf(" claim_interface  Failed ret=%d.\n",ret);
		}
	}
	
	//send hid data roleswitch
	printf("\nReading HID Report Descriptors:\n");
	descriptor_size = libusb_control_transfer(handle, 0x40,
		0x51, 0x01, 0, NULL, 0, 1000);
	if (descriptor_size < 0) {
		printf("   Failed\n");
		return -1;
	}
	
	libusb_close(handle);
	
	libusb_exit(NULL);
	
	return 1;
}

int main(int argc, char *argv[])
{    
       do_roleswitch();
	return 0;
}
