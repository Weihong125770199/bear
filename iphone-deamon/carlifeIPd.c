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

#include <stddef.h>
#include <pthread.h>
#include <sys/socket.h>
#include <linux/in.h>
#include <linux/tcp.h>
#include <linux/netlink.h>
//#include <net/tcp_states.h>
#include <sys/un.h>
#include <errno.h>
#include <pthread.h>
//#include "cutils/properties.h"

#include "libusb.h"
#include "libusbi.h"
#include "iap2.h"
#include "Log.h"
//============Function Define Start================
//#define CARLIFE_CARPLAY_SWITCH

//===========Function Define End================

#define LOG_TAG "CARLIFEIPD"

#define UEVENT_BUFFER_SIZE 1024  

#define    VENDOR_ID       0x05ac
#define    PRODUCT_ID      0x12a8
int fd_iap; //the handle for open dev/IAP2 == dev/ipodout
pthread_t pthread_deal_with_carlife_data_thread_tid;
pthread_t pthread_recv_socket_thread_tid;


//iphone status
#define IP_DEATTACHED 				 0
#define IP_ATTACHED                                1
#define IP_ATTACHED_NOT_ROLESWED   2
#define IP_ATTACHED_ROLESWED 		 3
#define CONNECTION_PATH  "/dev/connection_state"
typedef enum _IPSTATUS
{
	IP_STAT_ACCEPT =0,
	IP_STAT_ACCEPT_NOTRUN,
	IP_STAT_WAIT_CONNET,
	IP_STAT_CARLIFE_NOTRUN,
	IP_STAT_SWITCHROLE,
	IP_STAT_SWITCHROLEING,
	IP_STAT_ALLOCIP,
	IP_STAT_AUTHEN,	
	IP_STAT_WAIT_LAUNCH,
	IP_STAT_DISCONNET,	
};

#define IP_UNKNOW -1

#define iAP2_FILE "/dev/ipodout0"
/*
return:
0: not attatched
1: not ready
2: ready
-1: unknow
*/
int iphone_ready(void) {
	libusb_device_handle *handle;
       int ret = access(iAP2_FILE,F_OK);	  
	SLOGI("access ret =%d \n",ret);
       	if(ret== 0)
	{
		SLOGI("Iphone is ready!\n");
		return IP_ATTACHED_ROLESWED;
	}
	else
	{
	/*	handle = libusb_open_device_with_vid_pid(NULL, VENDOR_ID, PRODUCT_ID);	
		if (handle == NULL) {
			printf("iphone disconnected!\n");
			return IP_DEATTACHED;
		}
		
		printf("iphone attached, need do roleswith!\n");
		libusb_close(handle);*/
		return IP_ATTACHED_NOT_ROLESWED;
	}
}

int is_phone_already_exist()
{
    int fd =0;
    char connection_state;
    fd = open(CONNECTION_PATH, O_RDWR);
	if (fd < 0)
	{
		printf("can't open!\n");
	}

	read(fd, &connection_state, 1);
	if((connection_state =='1')||(connection_state == '2'))
		{
		    return 1;
		}
	       else
	       {
	           return 0;
	       }
}
int IsIphoneConnected()
{
     libusb_device_handle *handle;
     int bus =0;
     handle = libusb_open_device_with_vid_pid(NULL, VENDOR_ID, PRODUCT_ID);	
      if (handle == NULL) 
     {		
	 //	SLOGI("iphone disconnected!\n");			
		return IP_DEATTACHED;	
     }
     else	
     {		
	 // 	SLOGI("iphone connected!\n");	
	     bus=(int)libusb_get_bus_number(handle->dev);
	     libusb_close(handle);
	     if(bus ==1)
	     	{
	     	    if(is_phone_already_exist())
	             {
                       return IP_DEATTACHED;
		      }
		         return IP_ATTACHED;
	     	}
	       else
	      {
	                return IP_DEATTACHED;
	       }
     }
}
#ifdef CARLIFE_CARPLAY_SWITCH
//@
//@Attention:persist.sys.iphone_connect_app Will ResetTP2546 
//@
int Readiphonecfg(void)
{
	char property;
	char propbuf[PROPERTY_VALUE_MAX];
	property_get("persist.sys.iphone_connect_app", propbuf, "");
	//SLOGI("Readiphonecfg %s",propbuf);
	property = propbuf[0];
	if(property == '2')
	{
		return 2;
	}
	return -1;
}
#endif

#if 0
int SetTP2546(int value)
{
	int fd;
	char mode;
	
	fd = open("sys/class/gpio/gpio120/value", O_RDWR);
	if(fd < 0)
	{
		SLOGI("open sys/class/gpio/gpio120/value failed");
		return -1;
	}
	if(value == 0)
	{//0
		mode = '0';
	}
	else
	{// 1
		mode = '1';
	}
	write(fd,&mode,1);	
	close(fd);
	return 0;
}
int ResetTP2546(void)
{
	int fd;
	char mode;

#ifdef CARLIFE_CARPLAY_SWITCH
	if(Readiphonecfg() !=2)
	{
		return -1;
	}
#endif
	
	SLOGI("enter DisableTP2456\n ");
	fd = open("sys/class/gpio/gpio120/value", O_RDWR);
	if(fd < 0)
	{
		SLOGI("open sys/class/gpio/gpio120/value failed");
		return -1;
	}
	mode = '0';
	write(fd,&mode,1);	
	sleep(1); //sleep 1s
	mode = '1';
	write(fd,&mode,1);	
	
	close(fd);

	return 0;

}
#endif

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


int UsbHostDeviceSwitch()
{
	int fd;
 	char mode[]="aaa";
	char new_mode[]="otg";
	int read_bytes =0;
	SLOGI("enter UsbHostDeviceSwitch\n ");
       enable_gadget_function(0);
	set_gadget_function(1);
	enable_gadget_function(1);
	
	fd = open("/sys/atc2ctl/mode", O_RDWR);
	if(fd < 0)
	{
		SLOGI("open /sys/atc2ctl/mode failed");
		return -1;
	}

	read_bytes =read(fd,mode,4);

	//SLOGI(" read /sys/atc2ctl/mode:%s   bytes=%d\n ",mode,read_bytes);
       if( strncmp(mode,"otg",3))
	{
		SLOGI("Usb is HOST mode ,Switch USB Device mode\n");
		write(fd,new_mode,4);
	}
	close(fd);

	return 0;
}

int UsbDeviceHostSwitch()
{
	int fd;
	char mode[]="bbbb";
	char new_mode[]="host";
  int read_bytes =0;
	SLOGI("enter UsbDeviceHostSwitch\n ");
	fd = open("/sys/atc2ctl/mode", O_RDWR);
	if(fd < 0)
	{
		SLOGI("/sys/atc2ctl/mode failed");
		return -1;
	}
	read_bytes = read(fd,mode,5);
	SLOGI(" read /sys/atc2ctl/mode :%s  read_bytes =%d \n ",mode,read_bytes);
  //SLOGI(" CMP RET = :%d \n ",strncmp(mode,"host",4));

       if(strncmp(mode,"host",4))
	{
		SLOGI("Usb is device  mode ,Switch USB Host mode\n");
		write(fd,new_mode,5);
		sleep(1);
	}
	close(fd);
	return 0;
}
//@send cmd to  iphone switch role
int do_roleswitch(void){
	libusb_device *dev;
	libusb_device_handle *handle;
	struct libusb_config_descriptor *conf_desc;
	int  descriptor_size;
	int iface, nb_ifaces, ret;

//	ret = libusb_init(NULL);
//	if (ret < 0)
//	{
//		printf("Fail to libusb_init\n");
//		return ret;
//	}
	
	SLOGI("Opening device %04X:%04X...\n", VENDOR_ID, PRODUCT_ID);
	handle = libusb_open_device_with_vid_pid(NULL, VENDOR_ID, PRODUCT_ID);	
	if (handle == NULL) {
		SLOGI("  Failed libusb_open_device_with_vid_pid .\n");
		return -1;
	}

	dev = libusb_get_device(handle);
	
	SLOGI("\nReading first configuration descriptor:\n");
	libusb_get_config_descriptor(dev, 0, &conf_desc);
	nb_ifaces = conf_desc->bNumInterfaces;
	SLOGI("             nb interfaces: %d\n", nb_ifaces);
	
	libusb_free_config_descriptor(conf_desc);
	
	if(libusb_detach_kernel_driver(handle, 1)<0)
	{
		SLOGI("Fail to libusb_detach_kernel_driver\n");
	}
	
	for (iface = 0; iface < nb_ifaces; iface++)
	{
		SLOGI("\nClaiming interface %d...\n", iface);
		ret = libusb_claim_interface(handle, iface);
		if (ret != LIBUSB_SUCCESS) {
			SLOGI("   Failed.\n");
		}
	}
	
	//send hid data roleswitch
	SLOGI("Reading HID Report Descriptors:\n");
	descriptor_size = libusb_control_transfer(handle, 0x40,
		0x51, 0x00, 0, NULL, 0, 1000);
	if (descriptor_size < 0) {
		SLOGI("   Failed\n");
		return -1;
	}
	
	libusb_close(handle);
//	libusb_exit(NULL);
	
	return 1;
}
//@Iap2 Authentication
int EnterIap2Auythentication()
{
	SLOGI("enter EnterIap2Auythentication\n ");
	fd_iap =iap2_open();
	if(fd_iap<0)
	{     
	       SLOGI("open /dev/ipodout0  fail \n ");
		return -1;
	}
	if(InitializationProcess(fd_iap)<0)
	{
             SLOGI("InitializationProcess  fail \n ");
		return -1;
	}    	
	return 0;
}


//Driver to Local Carlife
#define SENDUSBROLESWITCH	0x01
#define IAP2AUTHBEFORE		0x02
#define IAP2AUTHAFTER		0x03
#define ALLOCATIONNCMIP		0x04
#define EAPFILEREADY  		0x05
#define USBDISCONNECT		0x10
//Local Carlife to Driver
#define PULLUPCARLIFE		0x11
#define CALIRFE_TCP_CLIENT_DISCONNECT  0x12


//Carlife IPd to APP
#define CARLIFE_IPHONE_PLUGIN                0x20
#define CARLIFE_IPHONE_PLUGOUT               0x21
// APP to Carlife IPd 
#define APP_REQUEST_IPHONE_STATUS        0x30


#define	BD_SOCKET_NAME	" /data/local/tmp/ncm.sock"
int g_socketfd = -1;

//@comunicate to carlife client
static void bd_socket_init()
{
	struct sockaddr_un server_addr;
	int addr_len;
	
	server_addr.sun_family = AF_LOCAL;
	strcpy(server_addr.sun_path, BD_SOCKET_NAME);
	addr_len = sizeof(server_addr);
	
	server_addr.sun_path[0] = 0;	// set first char 0
	addr_len = strlen(BD_SOCKET_NAME) + offsetof(struct sockaddr_un, sun_path);	// offsetof defined in stddef.h

	g_socketfd = socket(AF_LOCAL,SOCK_STREAM, 0);
	if(g_socketfd < 0) 
	{
		perror("socket failed");  
		return ;
	}
	
	if(bind(g_socketfd, (struct sockaddr *)&server_addr, addr_len) < 0)
	{
		perror("bind failed");
		return;  
	}
	
	if(listen(g_socketfd, 5) < 0)
	{
		perror("listen failed");
		return;  
	}
}
/*
return:
1: iphone attached
0: iphone deattached
-1: no iphone event
*/
int iphone_event(unsigned char* buf, int size)
{
       char* p=NULL;
	char* actionADD="add@";
	char* actionRM="remove@";
	char* iphoneVID="05AC:";
	char* iphonePID="12A8";
	char* idevice="hidraw0";
	
	char attached[] = "add@/devices/platform/fsl-ehci.0/usb1/1-1/1-1:2.2/0003:05AC:12A8.0002/hidraw/hidraw0";
	char deattached[] = "remove@/devices/platform/fsl-ehci.0/usb1/1-1/1-1:2.2/0003:05AC:12A8.0002/hidraw/hidraw0";

	if(size>=sizeof(attached)){
	 SLOGI("%s\n", buf); 
	  if(((p = strstr(buf, actionADD))!=NULL)){
		  if((p = strstr(p, iphoneVID))!=NULL){
			  if((p = strstr(p, iphonePID))!=NULL){
				  if((p = strstr(p, idevice))!=NULL){
					 //  iattached = 1;
					   SLOGI("iPhone plugin! %s\n", buf);
					   return 1;
				  }
			  }
		  }
	  } else if(((p = strstr(buf, actionRM))!=NULL)){
			  if((p = strstr(p, iphoneVID))!=NULL){
				  if((p = strstr(p, iphonePID))!=NULL){
					  if((p = strstr(p, idevice))!=NULL){
						 //  iattached = 0;
						   SLOGI("iPhone removed! %s\n", buf);
						   return 0;
					  }
				  }
			  }
		  }
	} 

	return -1;
}

static int init_hotplug_sock()  
{  
	  const int buffersize = UEVENT_BUFFER_SIZE;  
	  int ret;  

	  struct sockaddr_nl snl;  
	  bzero(&snl, sizeof(struct sockaddr_nl));  
	  snl.nl_family = AF_NETLINK;  
	  snl.nl_pid = getpid();  
	  snl.nl_groups = 1;  

	 SLOGI("init_hotplug_sock");  
	  int s = socket(PF_NETLINK, SOCK_DGRAM, NETLINK_KOBJECT_UEVENT);  
	  if (s == -1)   
	  {  
		    SLOGE("socket");  
		    return -1;  
	  }  
	  setsockopt(s, SOL_SOCKET, SO_RCVBUF, &buffersize, sizeof(buffersize));  

	  ret = bind(s, (struct sockaddr *)&snl, sizeof(struct sockaddr_nl));  
	  if (ret < 0)   
	  {  
		   SLOGE("bind");  
		   close(s);  
		   return -1;  
	  }  

	  return s;  
}  

//@communicate to carlife Client
void *talkwithlocalCarlife_thread( void *arg )
{
	int client_sockfd = -1, client_sockfd_bak =-1,i=0;  
  	socklen_t	client_len;    
       struct sockaddr_un client_addr; 
	struct tcp_info info;
	int info_len = sizeof(info);
	char mdstatus = SENDUSBROLESWITCH;
	ssize_t ret = 0;	
	uint8_t value = 0;
	char ipstatus = IP_STAT_ACCEPT;	
	char* pstr = NULL;
	 int socket_fd = 0;	
	 socket_fd = *((int*)arg);
	int count =0;	
	    //fd_set
  	  fd_set server_fd_set;
  	  int max_fd = -1;
    	struct timeval tv;  //超时时间设置
#ifdef CARLIFE_CARPLAY_SWITCH
	char iphone_conn_status= IP_DEATTACHED;
	unsigned int timeout=0;
	unsigned char iphone_hotplug_event = 0;
	unsigned char iphone_send_event = 0;
#endif
    	//@receive param
    	//unsigned char buf[UEVENT_BUFFER_SIZE] = {0};
	//int hotplug_sock = -1;		
   	//	int num;
		

	for(;;)
	{
	      // SLOGI("00000000000 ipstatus = %d\r\n",ipstatus);
		usleep(100000);// 100ms
		{
			if((ipstatus == IP_STAT_WAIT_CONNET)||(ipstatus == IP_STAT_CARLIFE_NOTRUN))
			{		
                         //   SLOGI("111111111 ipstatus = %d\r\n",ipstatus);
			
									if(IsIphoneConnected() == IP_ATTACHED)
										{
											ipstatus = IP_STAT_SWITCHROLE;
											SLOGI("IPHONE HOTPLUG IN\r\n");
											
										}
				                               else
										{	
											
											  //   SLOGI("2222222 ipstatus = %d\r\n",ipstatus);
										}
			}
			if(ipstatus == IP_STAT_WAIT_LAUNCH)
			{
			   
	
				if(access(iAP2_FILE,F_OK) != 0)				
				{
					SLOGI("access(iAP2_FILE,F_OK) failed!\n");	
					ipstatus = IP_STAT_DISCONNET;
				}
				else
				{
				//	printf("iphone disconnected!\n");	
				//	SLOGI("iphone disconnected !\n");
				}
			}						
				
		}	
		tv.tv_sec = 0;
	    	tv.tv_usec = 100000;
	    	FD_ZERO(&server_fd_set);
		//服务器端socket
	        FD_SET(socket_fd, &server_fd_set);
	    //    printf("server_sock_fd=%d\n", server_sock_fd);
	        if(max_fd < socket_fd)
	        {
	        	max_fd = socket_fd;
	        }	
			//SLOGI("IPd wait select ............................. \n"); 
		 switch(select(max_fd + 1, &server_fd_set, NULL, NULL, &tv))   //select使用
	        {
	            case -1: 
                          //     SLOGI("select error ,exit  carlifeIpd \n"); 
				   break; //select错误，退出程序 
	            case 0:
				 //   SLOGI("exit and select again \n"); 
				   break; //再次轮询
	            default:
				//    SLOGI("select default \n"); 
	                if(FD_ISSET(socket_fd, &server_fd_set))
	                  {	        
	                  	SLOGI("CarLife server accept \n"); 
	                     	 client_sockfd_bak = accept(socket_fd,(struct sockaddr*)&client_addr, &client_len); 
				if(client_sockfd_bak>0)
				{
					if(client_sockfd != (-1))
					{								
						close(client_sockfd);
								
					}
					client_sockfd = client_sockfd_bak;									
					if(ipstatus == IP_STAT_WAIT_LAUNCH)
					{
						mdstatus = ALLOCATIONNCMIP;
					       send(client_sockfd, &mdstatus, sizeof(mdstatus), MSG_DONTWAIT);
					}
				}
	                  }
			   break;
	          }	
		switch(ipstatus)
		{
			case IP_STAT_ACCEPT:
				//printf("\n=============IP_STAT_ACCEPT \n");
				if(client_sockfd>0)
				{
					ipstatus = IP_STAT_WAIT_CONNET;
				}				
				break;
			
			case IP_STAT_WAIT_CONNET:	
				//printf("\n=============IP_STAT_WAIT_CONNET \n");
				ret = recv(client_sockfd, &value, sizeof(value), MSG_DONTWAIT);
				if(ret >0)
				{
					if(value == CALIRFE_TCP_CLIENT_DISCONNECT)
					{
						//ipstatus = IP_STAT_ACCEPT;
						ret = recv(client_sockfd, &value, sizeof(value), MSG_DONTWAIT);						
						if(ret  != 0)
						{
							mdstatus = 0xff;
							send(client_sockfd, &mdstatus, sizeof(mdstatus), MSG_DONTWAIT);
						}						
						//close(client_sockfd);
						//client_sockfd = -1;
					}
					
				}
				else if(ret < 0)
				{
					//printf("socket err\n");					
				}
				else
				{
					SLOGI("IP_STAT_WAIT_CONNET socket quit\n");	
					ipstatus = IP_STAT_ACCEPT;	
					close(client_sockfd);
					client_sockfd = -1;
				}
				break;

			case IP_STAT_SWITCHROLE:
			//	printf("\n=============IP_STAT_SWITCHROLE \n");
				ret = recv(client_sockfd, &value, sizeof(value), MSG_DONTWAIT);						
				if(ret  != 0)
				{
					mdstatus = SENDUSBROLESWITCH;
					send(client_sockfd, &mdstatus, sizeof(mdstatus), MSG_DONTWAIT);
				}				
				do_roleswitch();
				//sleep(1);
				usleep(100000); //100ms
				UsbHostDeviceSwitch();	
				ipstatus = IP_STAT_SWITCHROLEING;
				count = 10;
				break;
			case IP_STAT_SWITCHROLEING:
				printf("\n=============IP_STAT_SWITCHROLEING \n");
				count--;
				if(count == 0)
				{
					ipstatus = IP_STAT_WAIT_LAUNCH;
					break;
				}				
				if(IP_ATTACHED_ROLESWED == iphone_ready())
				{
					ipstatus = IP_STAT_AUTHEN;
					
					ret = recv(client_sockfd, &value, sizeof(value), MSG_DONTWAIT);						
					if(ret  != 0)
					{
						mdstatus = IAP2AUTHBEFORE;
						send(client_sockfd, &mdstatus, sizeof(mdstatus), MSG_DONTWAIT);
					}					
				}
				break;
			case IP_STAT_AUTHEN:		
				SLOGI("=============IP_STAT_AUTHEN");
				if(EnterIap2Auythentication()<0)
				{
					ipstatus = IP_STAT_DISCONNET;	
					break;
				}
				ret = recv(client_sockfd, &value, sizeof(value), MSG_DONTWAIT);						
				if(ret  != 0)
				{
					mdstatus = IAP2AUTHAFTER;
					send(client_sockfd, &mdstatus, sizeof(mdstatus), MSG_DONTWAIT);
				}
				//usleep(100000);
				//notice local carlife status: ip configure
				ipstatus = IP_STAT_ALLOCIP;				
				break;
			case IP_STAT_ALLOCIP:	
                            SLOGI("=============IP_STAT_ALLOCIP");
				ipstatus = IP_STAT_WAIT_LAUNCH;
				//config s401 ip && lunch dhcp, config it early for better performence
				//system("ifconfig usb0 192.168.0.2");
				//system("busybox udhcpd /system/etc/udhcpd.conf");
				//system("ip -6 addr add 2001:0db8:0:f101::1/64 dev usb0");
				ret = recv(client_sockfd, &value, sizeof(value), MSG_DONTWAIT);						
				if(ret  != 0)
				{
					mdstatus = ALLOCATIONNCMIP;
					send(client_sockfd, &mdstatus, sizeof(mdstatus), MSG_DONTWAIT);
				}				
				break;
			
			case IP_STAT_WAIT_LAUNCH:
				// SLOGI("=============IP_STAT_WAIT_LAUNCH");
				ret = recv(client_sockfd, &value, sizeof(value), MSG_DONTWAIT);
				if(ret >0)
				{
					SLOGI("local carlife cmd: 0x%x\n", value);
					if(value == PULLUPCARLIFE)
					{
						//printf("cmd: pullup iphone Carlife!\n", mdstatus);

						if(fd_iap)
						{
							AppLunchCarlifeEx(fd_iap);	
						}	
						
					}
					else if(value == CALIRFE_TCP_CLIENT_DISCONNECT)
					{
						ret = recv(client_sockfd, &value, sizeof(value), MSG_DONTWAIT);
						if(ret  != 0)
						{
							mdstatus = 0xff;
							send(client_sockfd, &mdstatus, sizeof(mdstatus), MSG_DONTWAIT);
						}										
						//close(client_sockfd);
						//client_sockfd = -1;
						ipstatus = IP_STAT_DISCONNET;
					}				
						
						
				}
				else if(ret < 0)
				{
					//printf("socket err\n");					
				}
				else
				{
					SLOGI("IP_STAT_WAIT_LAUNCH socket quit\n");	
					//ipstatus = IP_STAT_DISCONNET;	
					if(client_sockfd != (-1))
					{
						close(client_sockfd);
						client_sockfd = -1;
					}
					
				}				
				break;
			case IP_STAT_DISCONNET:
				SLOGI("=============IP_STAT_DISCONNET");	
				//send iphone disconnect msg to local carlife
				if(client_sockfd != (-1))
				{	
					ret = recv(client_sockfd, &value, sizeof(value), MSG_DONTWAIT);
					if(ret  != 0)
					{
						mdstatus = USBDISCONNECT;
						send(client_sockfd, &mdstatus, sizeof(mdstatus), MSG_DONTWAIT);
					}	
				}
				//set to HU OTG to host mode
				//SetTP2546(0);
				usleep(250000);//250ms
				UsbDeviceHostSwitch();
				usleep(250000);//250ms
				//SetTP2546(1);
				iap2_close(fd_iap);				
				
				//usleep(100000); //100ms
				//sleep(1);	
				if(client_sockfd == (-1))
				{
					ipstatus = IP_STAT_ACCEPT;
				}
				else
				{
					ipstatus = IP_STAT_WAIT_CONNET;
				}				
				break;
			defualt:
				break;
				
		}
	}
}
void ExitCarlifeIPd()
{
	SLOGI("ExitCarlifeIPd!\n");
	UsbDeviceHostSwitch();
	close(g_socketfd);
	libusb_exit(NULL);	
}
int main(void)
{
	int ret = libusb_init(NULL);
	 int err =-1 ;
	if (ret < 0)
	{
		SLOGI("Fail to libusb_init\n");
		return ret;
	}
	
	UsbDeviceHostSwitch();
	bd_socket_init();
	
	if(g_socketfd == -1){
		printf("init local socket failed!\n");
		return -1;
	}
	atexit(ExitCarlifeIPd);

	
	 err = pthread_create( &pthread_deal_with_carlife_data_thread_tid, NULL,                   
	  			talkwithlocalCarlife_thread, &g_socketfd);   
	pthread_detach( pthread_deal_with_carlife_data_thread_tid );

	while(1)
	{
		sleep(1);
	}	
	return 0;
}

