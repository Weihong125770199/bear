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
#include <libusb-1.0/libusb.h>

#define msleep(msecs) usleep(1000*msecs)

#define    VENDOR_ID       0x05ac
#define    PRODUCT_ID      0x12a8

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

        ret= libusb_set_configuration(handle, 1);
	if(ret ==0 )
	{
	  printf("set config#1 successful \n");
	  while(1);
	
	}else
	{
	  printf("set config#2 fail ret=%d \n",ret);
	}

	dev = libusb_get_device(handle);
	
	printf("\nReading first configuration descriptor:\n");
	libusb_get_config_descriptor(dev, 2, &conf_desc);
	nb_ifaces = conf_desc->bNumInterfaces;
	printf("             nb interfaces: %d\n", nb_ifaces);
	
	libusb_free_config_descriptor(conf_desc);
	
	if(libusb_detach_kernel_driver(handle, 2)<0)
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
		0x51, 0x1, 0, NULL, 0, 3000);
	if (descriptor_size < 0) {
		printf("   Failed\n");
		return -1;
	}else
	{
	  printf("do role switch successful -- \n");
	
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
