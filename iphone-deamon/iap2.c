// ***********************************************************************
// Assembly        : iap2 authentication
// Author           : xiaokai
// Created          : 03-21-2016
//
// Last Creat By : xiaokai
// Last Create On : 07-26-2016
// ***********************************************************************
// <copyright file="iap2.c" company="Jiangxi Coagent">
//     Copyright (c) Jiangxi Coagent Electronics CO.,Ltd  All rights reserved.
// </copyright>
// <summary>
// "V2.0 revert eap authortication"
//Modify By:xiaokai
//Modify on:20160808
//</summary>
// <summary>
// "V2.1 fix sometime can pull up Iphone App"
//Modify By:xiaokai
//Modify on:20160808
//</summary>
// ***********************************************************************

#include "Log.h"
#include <stdio.h>
#include <linux/types.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <assert.h>
#include <string.h>


#include "iap2.h"
#include "accessory_authentication.h"

#define IAPVER    "V2.1 fix sometime can pull up Iphone App"

#define LOG_TAG "IAP2"

#define MAX_PLAYLOAD_LEN   65525
#define MAX_READBUFF_SIZE    1024
#define PACKET_START   0xff55
#define MAX_CERTIFICATE_DATA 1280

#define  DEVICE_TRANSPORT_COMPONET_ID     15
#define TRANSPORT_COMPONET_IDENTIFIER   16
#define IAP2_HID_COMPENT_ID			   18
#define EXTERNAL_ACCESSORY_PROTOCOL_ID  0
#define VEHICLE_INFO_COMPENT_ID    20
#define VEHICLE_STATUS_COMPENT_ID    21
#define LOCAL_INFO_COMPENT_ID    22
#define iAP2_FILE_PATH "/dev/ipodout0"
typedef enum _IAP2STATUS
{
	IAP2_INITIALIZATION = 0,
	IAP2_SYNCHRONIZATION,
	IAP2_REQUEST_AUTHENTICATION_CERTIFICATE,
	IAP2_AUTHENTICATION_CERTIFICATE,
	IAP2_REQUEST_AUTHENTICATION_CHALLENGE_RESPONSE,
	IAP2_AUTHENTICATION_CHALLENGE_RESPONSE,	
	IAP2_STARTIDENTIFICATION,
	IAP2_IDENTIFICATION_INFO,
	IAP2_LUNCH_APP,
	IAP2_SUCCESS,
};

typedef struct _st_apacket apacket;
typedef struct _st_amessage amessage;
typedef struct _st_playload playload;
typedef struct _st_control_session_parameters  parameter;
typedef struct _st_control_session_amessage ctrl_session_msg;


struct _st_amessage 
{
	unsigned char startMSL;
	unsigned char startLSB;
	unsigned char lengthMSL;
	unsigned char lengthLSB;
	unsigned char ctrl;
	unsigned char sequenceNum;
	unsigned char ackNum;
    unsigned char sessionID;
	unsigned char headerCheckSum;
};
struct _st_playload
{
	unsigned char playload[MAX_PLAYLOAD_LEN];
};

struct _st_apacket
{
	amessage msg;
	playload    data;
};  

struct _st_control_session_parameters 
{	
	unsigned char lengthMSL;
	unsigned char lengthLSB;
	unsigned char idMSB;
	unsigned char idLSB;
	unsigned char data[MAX_CERTIFICATE_DATA];	
	//parameter *next;
};
struct _st_control_session_amessage 
{
	unsigned char startMSL;
	unsigned char startLSB;
	unsigned char lengthMSL;
	unsigned char lengthLSB;
	unsigned char idMSB;
	unsigned char idLSB;
	parameter  ctrl_session_param;
};

typedef struct _Global_IAP2
{
	unsigned char  status;
	unsigned char* pCertificateData;
	int CertificateDataLen;
	//bakup ackNum sequenceNum sessionID
	unsigned char sequenceNum;
	unsigned char ackNum;
    unsigned char sessionID;
	
}Global_IAP2;

//global define 

Global_IAP2 g_iap2;


void Dump(const unsigned char * pData,int Len)
{	
	int i,j;
	SLOGI("Dump = %d:\n",Len);
	for(i=0;i<Len;)
	{
		for(j=0;j<16;j++,i++)
		{
			if(i>=Len)
			{
				break;
			}
			SLOGI("%x,",pData[i]);			

		}
		SLOGI("\n");
	}
}
int iap2_open()
{
	int fd;
	fd = open(iAP2_FILE_PATH,O_RDWR);
	//fd = open("dev/MFi_iAP20",O_RDWR);
	if(fd < 0)
	{
		//printf("\n open dev/ipodout0  FAIL \n");
	}
    return fd;
}

void iap2_close(int fd)
{
	close(fd);
}

int iap2_read(int fd, void *buf, int len)
{
	int  read_bytes =0;
	
	read_bytes = read(fd,buf,len);
	#if 0
	if(read_bytes>0)
	{	
		printf("\nDump start:");
		Dump(buf,read_bytes);		
	}
	#endif
	return read_bytes;
}

int iap2_write(int fd, const void *buf, int len)
{
	int  wrote_bytes;
	//printf("\n iap2_write = %d",len);
	wrote_bytes = write(fd,buf,len);

	return  wrote_bytes;
}

/// <summary>
/// Get_apackets this instance.
/// </summary>
/// <returns>apacket *.</returns>
apacket *get_apacket(void)
{
	apacket *p = (apacket *)malloc(sizeof(apacket));

	if (p == 0) 
		SLOGI("failed to allocate an apacket\r\n");
	memset(p, 0, sizeof(apacket)/* - MAX_PAYLOAD*/);

	return p;
}
/// <summary>
/// Put_apackets the specified application.
/// </summary>
/// <param name="p">The application.</param>
void put_apacket(apacket *p)
{
	free(p);
}


void send_packet(apacket *p, int fd)
{
	unsigned char *x;
	unsigned char sum;
	unsigned count;
	unsigned short dataLen;
	unsigned char* pbuff;

	//printf("\n enter send_packet");
	
	sum = 0;
	sum += p->msg.startMSL;
	sum += p->msg.startLSB;
	sum += p->msg.lengthMSL;
	sum += p->msg.lengthLSB;
	sum += p->msg.ctrl;
	sum += p->msg.sequenceNum;
	sum += p->msg.ackNum;
	sum += p->msg.sessionID;
	p->msg.headerCheckSum = (sum^0xFF)+1;
//	printf("\n p->msg.headerCheckSum = %x",p->msg.headerCheckSum);
	dataLen = (p->msg.lengthMSL<<8)|p->msg.lengthLSB;
	pbuff = malloc(dataLen);
	if(dataLen <10)
	{
		memcpy(pbuff,p,dataLen);
		//Dump(pbuff,dataLen);
		if(iap2_write(fd, pbuff, dataLen)<0)
		{
			SLOGI("send_packet::adb_write p->msg err\n");
		}	
		goto FAIL_OUT ;
	}
	count = dataLen-10;
	sum = 0;
	x = (unsigned char *) p->data.playload;
	while (count-- > 0)
	{
		sum += *x++;
	}
	p->data.playload[dataLen-9-1] = (sum^0xff)+1;
	if (fd <0) 
	{
		SLOGI("Transport is null \r\n");
	}	
	memcpy(pbuff,p,dataLen);
	//Dump(pbuff,dataLen);
	if(iap2_write(fd, pbuff, dataLen)<0)
	{
		SLOGI("send_packet::adb_write p->msg err\n");
	}
FAIL_OUT:
	free(pbuff);
}


int check_header(apacket *p)
{
	unsigned short startCmd;
	unsigned short dataLen;
	
	//printf("read_packet-msg.startMSL==0x%x\r\n",p->msg.startMSL);
	//printf("read_packet-msg.startLSB=0x%x\r\n",p->msg.startLSB);
	startCmd = (p->msg.startMSL<<8)|p->msg.startLSB;
	dataLen = (p->msg.lengthMSL<<8)|p->msg.lengthLSB;
	
	if (startCmd != 0xFF5A) 
	{
		SLOGI("check_header(): invalid startCmd\r\n");
		return -1;
	}

	if (dataLen> MAX_PLAYLOAD_LEN) 
	{
		SLOGI("check_header(): %d > MAX_PAYLOAD\r\n", dataLen);
		return -1;
	}

	return 0;
}

int check_data(apacket *p)
{
	unsigned char count, sum;
	unsigned char *x;
	short dataLen;

	x = p ;
	sum = 0;
	count = 8;
	while (count-- > 0) 
	{
		sum += *x++;
	}	
	sum = (sum^0xFF)+1;
	//printf("headerCheckSum sum = %x ",sum);
	if(sum != p->msg.headerCheckSum)
	{
		SLOGI("headerCheckSum err\n");
		return -1;
	}
	dataLen = p->msg.lengthMSL<<8|p->msg.lengthLSB;
	if(dataLen < 10)
	{
		return 1;
	}
	count = dataLen-10;	
	x = (unsigned char *) p->data.playload;
	sum = 0;
	while (count-- > 0) 
	{
		sum += *x++;
	}
	sum = (sum^0xFF)+1;
	if (sum != p->data.playload[dataLen - 10])
	{
		SLOGI("playCheckSum err\n");
		return -1;
	}
	return 1;
}


int read_packet(apacket *p, int fd)
{
	int ret = 0;
	int dataLen;
	unsigned char * pbuff;
	short relLen;

	pbuff = malloc(MAX_READBUFF_SIZE);
	relLen = iap2_read(fd, pbuff, MAX_READBUFF_SIZE);	
	
	if(relLen>0)
	{		
		memcpy(p,pbuff,relLen);
		if (check_header(p))
		{
			SLOGI("read_packet-check_header(): invalid header\r\n");
			goto FAIL_OUT;
		}
		
		if (check_data(p)<0) 
		{
			SLOGI("read_packet-check_data: check_data failed\r\n");
			goto FAIL_OUT;
		}
		ret = 1;
		//printf("read_packet-msg.ctrl=%d\r\n",p->msg.ctrl);
		//printf("read_packet-msg.sessionID=%d\r\n",p->msg.sessionID);
		//dataLen = (p->msg.lengthMSL<<8)+p->msg.lengthMSL - 9;
		//if (dataLen) 
		{	
		//	iap2_read(fd, &p->data,dataLen);
			
		}
	}
	else
	{
		SLOGI("iap2_read no value \n");
	}
FAIL_OUT:
	 free(pbuff);
	return ret;
}
void SendAckLink(apacket *m_apacket ,int fd)
{
	apacket *p =get_apacket();
//	printf("\n enter SendAckLink \n");
	p->msg.startMSL = 0xFF;
	p->msg.startLSB = 0x5A;
	p->msg.lengthMSL = 0x00;
	p->msg.lengthLSB = 0x9;
	p->msg.ctrl = 0x40;
	p->msg.sequenceNum = m_apacket->msg.ackNum;
	p->msg.ackNum = m_apacket->msg.sequenceNum;
	p->msg.sessionID = 0;
	send_packet(p,fd);	
	put_apacket(p);
}
void SendSycnLink(int fd)
{
	unsigned char linkData[]={0x01,0x05,0x10,0x00,0x04,0x0b,0x00,0x17,0x03,0x03,0x0A,0x00,0x01,0x0B,0x02,0x01};
	apacket *p =get_apacket();
	//printf("\n enter SendSycnLink \n");
	p->msg.startMSL = 0xFF;
	p->msg.startLSB = 0x5A;
	p->msg.lengthMSL = 0x00;
	p->msg.lengthLSB = 0x1A;
	p->msg.ctrl = 0x80;
	p->msg.sequenceNum = 0x2B;
	p->msg.ackNum = 0;
	p->msg.sessionID = 0;
	memcpy(p->data.playload,linkData,sizeof(linkData));
	send_packet(p,fd);	
	put_apacket(p);
}

//Send Authentication Certtificate
// X.509 Certificate<Accessory Certificate Data>
//Parameters:
//m_apacket:receiv apacket info;
//fd: handle 
int SendAuthenticationCertificate(apacket *m_apacket ,int fd)
{
	apacket *p =get_apacket();
	ctrl_session_msg *pmsg = (ctrl_session_msg*)malloc(sizeof(ctrl_session_msg));
	int len;
	p->msg.startMSL = 0xFF;
	p->msg.startLSB = 0x5A;
	p->msg.ctrl = 0x40; //iAP2 Session Payload be present
	p->msg.sequenceNum = m_apacket->msg.ackNum+1;
	p->msg.ackNum = m_apacket->msg.sequenceNum;
	p->msg.sessionID = m_apacket->msg.sessionID;
	// ctrl session msg
	pmsg->startMSL = 0x40;
	pmsg->startLSB = 0x40;
	pmsg->idMSB = 0xAA;
	pmsg->idLSB = 0x01;
	//ctrl parameters

	pmsg->ctrl_session_param.idMSB = 0;
	pmsg->ctrl_session_param.idLSB = 0;


	req_authentication_certificate(g_iap2.pCertificateData,&(g_iap2.CertificateDataLen));
	if(g_iap2.CertificateDataLen <= 0)
	{
	//	printf("\nSendAuthenticationCertificate: req_authentication_certificate FAILED\n");
		return -1;
	}
	else
	{
		SLOGI("req_authentication_certificate read len=%x\n",g_iap2.CertificateDataLen);
	}
	//printf("\ng_iap2.CertificateDataLen = %x\n",g_iap2.CertificateDataLen);
	memcpy(pmsg->ctrl_session_param.data,g_iap2.pCertificateData,g_iap2.CertificateDataLen);
	//Dump(g_iap2.pCertificateData,g_iap2.CertificateDataLen);
	len = g_iap2.CertificateDataLen+4;
	pmsg->ctrl_session_param.lengthMSL =(unsigned char) (len>>8);
	pmsg->ctrl_session_param.lengthLSB = (unsigned char)len;
	//printf("\npmsg->ctrl_session_param.lengthMSL = %x\n",pmsg->ctrl_session_param.lengthMSL);	

	//printf("\npmsg->ctrl_session_param.lengthLSB = %x\n",pmsg->ctrl_session_param.lengthLSB);	
	len +=6; 
	
	pmsg->lengthMSL = (unsigned char)(len>>8);
	pmsg->lengthLSB = (unsigned char)len;
	//printf("\npmsg->lengthMSL = %x\n",pmsg->lengthMSL);	
	//printf("\npmsg->lengthLSBB = %x\n",pmsg->lengthLSB);	
	memcpy(p->data.playload,pmsg,len);
	len += 10;
	p->msg.lengthMSL = (unsigned char)(len>>8);
	p->msg.lengthLSB = (unsigned char)len;

	send_packet(p,fd);	
	put_apacket(p);
	free(pmsg);
	return 0;
}
//Send Challenge Response
// X.509 Certificate<Accessory Certificate Data>
//Parameters:
//m_apacket:receiv apacket info;
//fd: handle 
//return:-1 failed 0 success
int SendChallengeResponse(apacket *m_apacket ,int fd)
{
	apacket *p =get_apacket();
	ctrl_session_msg *pmsg = (ctrl_session_msg*)malloc(sizeof(ctrl_session_msg));
	int len;
	len = (m_apacket->msg.lengthMSL<<8)|(m_apacket->msg.lengthLSB);
	if(len <10)
	{
		SLOGI("SendChallengeResponse:len err\n");
		return -1;
	}
	len -= 10;
	memcpy(pmsg,m_apacket->data.playload,len);	
	len = (pmsg->ctrl_session_param.lengthMSL<<8)|(pmsg->ctrl_session_param.lengthLSB);
	len = len -4;
	//Dump(pmsg->ctrl_session_param.data,len);
	g_iap2.CertificateDataLen = 0;
	req_authentication_challenge_response(pmsg->ctrl_session_param.data,len,g_iap2.pCertificateData,&(g_iap2.CertificateDataLen));
	if(g_iap2.CertificateDataLen <= 0)
	{
		//printf("\SendChallengeResponse: req_authentication_certificate FAILED\n");
		return -1;
	}
	else
	{
		SLOGI("req_authentication_certificate read len=%x\n",g_iap2.CertificateDataLen);
	}
	p->msg.startMSL = 0xFF;
	p->msg.startLSB = 0x5A;
	p->msg.ctrl = 0x40; //iAP2 Session Payload be present
	p->msg.sequenceNum = m_apacket->msg.ackNum+1;
	p->msg.ackNum = m_apacket->msg.sequenceNum;
	p->msg.sessionID = m_apacket->msg.sessionID;
	// ctrl session msg
	pmsg->startMSL = 0x40;
	pmsg->startLSB = 0x40;
	pmsg->idMSB = 0xAA;
	pmsg->idLSB = 0x03;
	//ctrl parameters
	pmsg->ctrl_session_param.idMSB = 0;
	pmsg->ctrl_session_param.idLSB = 0;

	//printf("\ng_iap2.CertificateDataLen = %x\n",g_iap2.CertificateDataLen);
	memcpy(pmsg->ctrl_session_param.data,g_iap2.pCertificateData,g_iap2.CertificateDataLen);
	//Dump(g_iap2.pCertificateData,g_iap2.CertificateDataLen);
	len = g_iap2.CertificateDataLen+4;
	pmsg->ctrl_session_param.lengthMSL =(unsigned char) (len>>8);
	pmsg->ctrl_session_param.lengthLSB = (unsigned char)len;
	//printf("\npmsg->ctrl_session_param.lengthMSL = %x\n",pmsg->ctrl_session_param.lengthMSL);	
	//printf("\npmsg->ctrl_session_param.lengthLSB = %x\n",pmsg->ctrl_session_param.lengthLSB);	
	len +=6; 
	
	pmsg->lengthMSL = (unsigned char)(len>>8);
	pmsg->lengthLSB = (unsigned char)len;
	//printf("\npmsg->lengthMSL = %x\n",pmsg->lengthMSL);	
	//printf("\npmsg->lengthLSBB = %x\n",pmsg->lengthLSB);	
	memcpy(p->data.playload,pmsg,len);
	len += 10;
	p->msg.lengthMSL = (unsigned char)(len>>8);
	p->msg.lengthLSB = (unsigned char)len;

	send_packet(p,fd);
	
	put_apacket(p);
	free(pmsg);
	return 0;
}
//Send Identification Info
//Parameters:
//m_apacket:receiv apacket info;
//fd: handle 
void SendIdentificationInfo(apacket *m_apacket ,int fd)
{
	apacket *p =get_apacket();
	ctrl_session_msg *pmsg = (ctrl_session_msg*)malloc(sizeof(ctrl_session_msg));
	parameter *parameter1 = (parameter*)malloc(sizeof(parameter));
	parameter *subparameter1 = (parameter*)malloc(sizeof(parameter));
	 char * pstr;
	 char * pdata = p->data.playload;
	 char * sumbuff = parameter1->data;
	int len,totalLen,subTotalLen;
	unsigned char ReceiveMsg[] = {0xea,0x00,0xea,0x01};
	unsigned char SendMsg[] = {0xea,0x02};	
	char *pbuff = (char *)malloc(MAX_CERTIFICATE_DATA);
	char * mpbuff;

	mpbuff = pbuff;
	SLOGI("SendIdentificationInfo\n");
	
	p->msg.startMSL = 0xFF;
	p->msg.startLSB = 0x5A;
	p->msg.ctrl = 0x40; //iAP2 Session Payload be present
	p->msg.sequenceNum = m_apacket->msg.ackNum+1;
	p->msg.ackNum = m_apacket->msg.sequenceNum;
	p->msg.sessionID = m_apacket->msg.sessionID;
	// ctrl session msg
	pmsg->startMSL = 0x40;
	pmsg->startLSB = 0x40;
	pmsg->idMSB = 0x1D;
	pmsg->idLSB = 0x01;

	//ctrl parameters
	//identification information
	//name
	totalLen = 0;
	//pstr = "s401_6dq";
	pstr = "Carlife";
	parameter1->idMSB = 0;
	parameter1->idLSB = 0;
	len = strlen(pstr)+1;
	memcpy(parameter1->data,pstr,len);	
	len = len+4;
	parameter1->lengthMSL = (unsigned char)(len>>8);
	parameter1->lengthLSB = (unsigned char)len;
	memcpy(mpbuff,parameter1,len);

	totalLen +=len;
	mpbuff +=len;
	pstr = "S401AUTO-MX6Q";
	parameter1->idMSB = 0;
	parameter1->idLSB = 1;
	len = strlen(pstr)+1;
	memcpy(parameter1->data,pstr,len);
	len = len+4;
	parameter1->lengthMSL = (unsigned char)(len>>8);
	parameter1->lengthLSB = (unsigned char)len;
	memcpy(mpbuff,parameter1,len);

	totalLen +=len;
	mpbuff +=len;
	pstr = "CoagentSoft";
	parameter1->idMSB = 0;
	parameter1->idLSB = 2;
	len = strlen(pstr)+1;
	memcpy(parameter1->data,pstr,len);
	len = len+4;
	parameter1->lengthMSL = (unsigned char)(len>>8);
	parameter1->lengthLSB = (unsigned char)len;
	memcpy(mpbuff,parameter1,len);

	totalLen +=len;
	mpbuff +=len;
	pstr = "android-2e58d25f52430a88";
	parameter1->idMSB = 0;
	parameter1->idLSB = 3;
	len = strlen(pstr)+1;
	memcpy(parameter1->data,pstr,len);
	len = len+4;
	parameter1->lengthMSL = (unsigned char)(len>>8);
	parameter1->lengthLSB = (unsigned char)len;
	memcpy(mpbuff,parameter1,len);

	totalLen +=len;
	mpbuff +=len;
	pstr = "1.0.0-rc3";
	parameter1->idMSB = 0;
	parameter1->idLSB = 4;
	len = strlen(pstr)+1;
	memcpy(parameter1->data,pstr,len);
	len = len+4;
	parameter1->lengthMSL = (unsigned char)(len>>8);
	parameter1->lengthLSB = (unsigned char)len;
	memcpy(mpbuff,parameter1,len);

	//hardware version
	totalLen +=len;
	mpbuff +=len;
	pstr = "1.0.0";
	parameter1->idMSB = 0;
	parameter1->idLSB = 5;
	len = strlen(pstr)+1;
	memcpy(parameter1->data,pstr,len);
	len = len+4;
	parameter1->lengthMSL = (unsigned char)(len>>8);
	parameter1->lengthLSB = (unsigned char)len;
	memcpy(mpbuff,parameter1,len);
	
	totalLen +=len;
	mpbuff +=len;
	parameter1->idMSB = 0;
	parameter1->idLSB = 6;
	len = sizeof(SendMsg);
	memcpy(parameter1->data,SendMsg,len);
	len = len+4;
	parameter1->lengthMSL = (unsigned char)(len>>8);
	parameter1->lengthLSB = (unsigned char)len;
	memcpy(mpbuff,parameter1,len);
	
	totalLen +=len;
	mpbuff +=len;
	parameter1->idMSB = 0;
	parameter1->idLSB = 7;
	len = sizeof(ReceiveMsg);
	memcpy(parameter1->data,ReceiveMsg,len);
	len = len+4;
	parameter1->lengthMSL = (unsigned char)(len>>8);
	parameter1->lengthLSB = (unsigned char)len;
	memcpy(mpbuff,parameter1,len);
	
	//Power Providing
	totalLen +=len;
	mpbuff +=len;
	parameter1->idMSB = 0;
	parameter1->idLSB = 8;
	parameter1->data[0] = 2;
	len = 5;
	parameter1->lengthMSL = (unsigned char)(len>>8);
	parameter1->lengthLSB = (unsigned char)len;
	memcpy(mpbuff,parameter1,len);
	
	//current
	totalLen +=len;
	mpbuff +=len;
	parameter1->idMSB = 0;
	parameter1->idLSB = 9;
	parameter1->data[0] = (unsigned char)(0);	
	parameter1->data[1] = (unsigned char)(0);
	
	len = 6;
	parameter1->lengthMSL = (unsigned char)(len>>8);
	parameter1->lengthLSB = (unsigned char)len;
	memcpy(mpbuff,parameter1,len);
	
	//Current Language
	totalLen +=len;
	mpbuff +=len;
	pstr = "zh";
	parameter1->idMSB = 0;
	parameter1->idLSB = 12;
	len = strlen(pstr)+1;
	memcpy(parameter1->data,pstr,len);
	len = len+4;
	parameter1->lengthMSL = (unsigned char)(len>>8);
	parameter1->lengthLSB = (unsigned char)len;
	memcpy(mpbuff,parameter1,len);
	
	//Support Language
	totalLen +=len;
	mpbuff +=len;
	pstr = "zh";
	parameter1->idMSB = 0;
	parameter1->idLSB = 13;
	len = strlen(pstr)+1;
	memcpy(parameter1->data,pstr,len);
	len = len+4;
	parameter1->lengthMSL = (unsigned char)(len>>8);
	parameter1->lengthLSB = (unsigned char)len;
	memcpy(mpbuff,parameter1,len);
	
	totalLen +=len;
	mpbuff +=len;
	pstr = "en";
	parameter1->idMSB = 0;
	parameter1->idLSB = 13;
	len = strlen(pstr)+1;
	memcpy(parameter1->data,pstr,len);
	len = len+4;
	parameter1->lengthMSL = (unsigned char)(len>>8);
	parameter1->lengthLSB = (unsigned char)len;
	memcpy(mpbuff,parameter1,len);
	
	//Support EA Protocol
	totalLen +=len;
	mpbuff +=len;
	
	parameter1->idMSB = 0;
	parameter1->idLSB = 10;
	//group
		subTotalLen = 0;
		sumbuff = parameter1->data;
		subparameter1->idMSB = 0;
		subparameter1->idLSB = 0;
		subparameter1->data[0] = 0;
		len = 5;
		subparameter1->lengthMSL = (unsigned char)(len>>8);
		subparameter1->lengthLSB = (unsigned char)len;
		memcpy(sumbuff,subparameter1,len);
		subTotalLen += len;
		sumbuff +=len;

		subparameter1->idMSB = 0;
		subparameter1->idLSB = 1;
		#ifdef IAP2_EAP
		pstr = "com.baidu.CarLifeVehicleProtocol";
		#else		
		pstr = "com.baidu.lbs.CarLife";
		#endif
		len = strlen(pstr)+1;
		memcpy(subparameter1->data,pstr,len);
		len = len+4;
		subparameter1->lengthMSL = (unsigned char)(len>>8);
		subparameter1->lengthLSB = (unsigned char)len;
		memcpy(sumbuff,subparameter1,len);
		subTotalLen += len;
		sumbuff +=len;
	
		subparameter1->idMSB = 0;
		subparameter1->idLSB = 2;	
		#ifdef IAP2_EAP
		subparameter1->data[0] = 1;
		#else
		subparameter1->data[0] = 0;
		#endif
		len = 5;
		subparameter1->lengthMSL = (unsigned char)(len>>8);
		subparameter1->lengthLSB = (unsigned char)len;
		memcpy(sumbuff,subparameter1,len);
		subTotalLen += len;
		#ifdef IAP2_EAP
		sumbuff +=len;

		subparameter1->idMSB = 0;
		subparameter1->idLSB = 3;	
		subparameter1->data[0] = 0;
		subparameter1->data[1] = 0;
		len = 6;
		subparameter1->lengthMSL = (unsigned char)(len>>8);
		subparameter1->lengthLSB = (unsigned char)len;
		memcpy(sumbuff,subparameter1,len);
		subTotalLen += len; 	
		#endif
/* 		sumbuff +=len;

		subparameter1->idMSB = 0;
		subparameter1->idLSB = 3;	
		subparameter1->data[0] = DEVICE_TRANSPORT_COMPONET_ID;
		subparameter1->data[1] = 0x00;
		len = 6;
		subparameter1->lengthMSL = (unsigned char)(len>>8);
		subparameter1->lengthLSB = (unsigned char)len;
		memcpy(sumbuff,subparameter1,len);
		subTotalLen += len; */
	subTotalLen +=4;
	parameter1->lengthMSL = (unsigned char)(subTotalLen>>8);
	parameter1->lengthLSB = (unsigned char)subTotalLen;
	memcpy(mpbuff,parameter1,subTotalLen);

	
	//Usb Device TransportCompent
	/*totalLen +=subTotalLen;
	mpbuff +=subTotalLen;	
	
	parameter1->idMSB = 0;
	parameter1->idLSB = 15;
	//group
		subTotalLen = 0;
		sumbuff = parameter1->data;
		subparameter1->idMSB = 0;
		subparameter1->idLSB = 0;
		subparameter1->data[0] = 0;
		subparameter1->data[1] = DEVICE_TRANSPORT_COMPONET_ID;
		len = 6;
		subparameter1->lengthMSL = (unsigned char)(len>>8);
		subparameter1->lengthLSB = (unsigned char)len;
		memcpy(sumbuff,subparameter1,len);
		subTotalLen += len;
		sumbuff +=len;

		subparameter1->idMSB = 0;
		subparameter1->idLSB = 1;
		pstr = "usb_device";
		len = strlen(pstr)+1;
		memcpy(subparameter1->data,pstr,len);
		len = len+4;
		subparameter1->lengthMSL = (unsigned char)(len>>8);
		subparameter1->lengthLSB = (unsigned char)len;
		memcpy(sumbuff,subparameter1,len);
		subTotalLen += len;
		sumbuff +=len;
	
		subparameter1->idMSB = 0;
		subparameter1->idLSB = 2;		
		len = 4;
		subparameter1->lengthMSL = (unsigned char)(len>>8);
		subparameter1->lengthLSB = (unsigned char)len;
		memcpy(sumbuff,subparameter1,len);
		subTotalLen += len;
		sumbuff +=len;

		subparameter1->idMSB = 0;
		subparameter1->idLSB = 3;	
		subparameter1->data[0] = 0x03;		
		len = 5;
		subparameter1->lengthMSL = (unsigned char)(len>>8);
		subparameter1->lengthLSB = (unsigned char)len;
		memcpy(sumbuff,subparameter1,len);
		subTotalLen += len;
	subTotalLen +=4;
	parameter1->lengthMSL = (unsigned char)(subTotalLen>>8);
	parameter1->lengthLSB = (unsigned char)subTotalLen;
	memcpy(mpbuff,parameter1,subTotalLen);*/

	//Usb Host TransportCompent
	totalLen +=subTotalLen;
	mpbuff +=subTotalLen;	
	
	parameter1->idMSB = 0;
	parameter1->idLSB = 16;
	//group
		subTotalLen = 0;
		sumbuff = parameter1->data;
		subparameter1->idMSB = 0;
		subparameter1->idLSB = 0;
		#ifdef IAP2_EAP
		subparameter1->data[0] = 0;
		#else
		subparameter1->data[0] = TRANSPORT_COMPONET_IDENTIFIER;
		#endif
		subparameter1->data[1] = 0;
		len = 6;
		subparameter1->lengthMSL = (unsigned char)(len>>8);
		subparameter1->lengthLSB = (unsigned char)len;
		memcpy(sumbuff,subparameter1,len);
		subTotalLen += len;
		sumbuff +=len;

		subparameter1->idMSB = 0;
		subparameter1->idLSB = 1;
		#ifdef IAP2_EAP
		pstr = "Baidu CarLife";
		#else
		pstr = "NCM";
		#endif
		len = strlen(pstr)+1;
		memcpy(subparameter1->data,pstr,len);
		len = len+4;
		subparameter1->lengthMSL = (unsigned char)(len>>8);
		subparameter1->lengthLSB = (unsigned char)len;
		memcpy(sumbuff,subparameter1,len);
		subTotalLen += len;
		sumbuff +=len;
	
		subparameter1->idMSB = 0;
		subparameter1->idLSB = 2;		
		len = 4;
		subparameter1->lengthMSL = (unsigned char)(len>>8);
		subparameter1->lengthLSB = (unsigned char)len;
		memcpy(sumbuff,subparameter1,len);
		subTotalLen += len;
		sumbuff +=len;

		subparameter1->idMSB = 0;
		subparameter1->idLSB = 3;	
		subparameter1->data[0] = 0x01;
		len = 5;
		subparameter1->lengthMSL = (unsigned char)(len>>8);
		subparameter1->lengthLSB = (unsigned char)len;
		memcpy(sumbuff,subparameter1,len);
		subTotalLen += len;
	subTotalLen +=4;
	parameter1->lengthMSL = (unsigned char)(subTotalLen>>8);
	parameter1->lengthLSB = (unsigned char)subTotalLen;
	memcpy(mpbuff,parameter1,subTotalLen);

	//iap2 HID Compent
	/*totalLen +=subTotalLen;
	mpbuff +=subTotalLen;	
	
	parameter1->idMSB = 0;
	parameter1->idLSB = 18;
	//group
		subTotalLen = 0;
		sumbuff = parameter1->data;
		subparameter1->idMSB = 0;
		subparameter1->idLSB = 0;
		subparameter1->data[0] = 0;
		subparameter1->data[1] = IAP2_HID_COMPENT_ID;
		len = 6;
		subparameter1->lengthMSL = (unsigned char)(len>>8);
		subparameter1->lengthLSB = (unsigned char)len;
		memcpy(sumbuff,subparameter1,len);
		subTotalLen += len;
		sumbuff +=len;

		subparameter1->idMSB = 0;
		subparameter1->idLSB = 1;
		pstr = "hidiap2";
		len = strlen(pstr)+1;
		memcpy(subparameter1->data,pstr,len);
		len = len+4;
		subparameter1->lengthMSL = (unsigned char)(len>>8);
		subparameter1->lengthLSB = (unsigned char)len;
		memcpy(sumbuff,subparameter1,len);
		subTotalLen += len;
		sumbuff +=len;
	
		subparameter1->idMSB = 0;
		subparameter1->idLSB = 2;	
		subparameter1->data[0] = 0x07;	
		len = 5;
		subparameter1->lengthMSL = (unsigned char)(len>>8);
		subparameter1->lengthLSB = (unsigned char)len;
		memcpy(sumbuff,subparameter1,len);
		subTotalLen += len;
		sumbuff +=len;

	subTotalLen +=4;
	parameter1->lengthMSL = (unsigned char)(subTotalLen>>8);
	parameter1->lengthLSB = (unsigned char)subTotalLen;
	memcpy(mpbuff,parameter1,subTotalLen);*/

	//Vehicle Info Compent
	totalLen +=subTotalLen;
	mpbuff +=subTotalLen;	
	
	parameter1->idMSB = 0;
	parameter1->idLSB = 20;
	//group
		subTotalLen = 0;
		sumbuff = parameter1->data;
		subparameter1->idMSB = 0;
		subparameter1->idLSB = 0;
		subparameter1->data[0] = 0;
		subparameter1->data[1] = VEHICLE_INFO_COMPENT_ID;
		len = 6;
		subparameter1->lengthMSL = (unsigned char)(len>>8);
		subparameter1->lengthLSB = (unsigned char)len;
		memcpy(sumbuff,subparameter1,len);
		subTotalLen += len;
		sumbuff +=len;

		subparameter1->idMSB = 0;
		subparameter1->idLSB = 1;
		pstr = "headunit";
		len = strlen(pstr)+1;
		memcpy(subparameter1->data,pstr,len);
		len = len+4;
		subparameter1->lengthMSL = (unsigned char)(len>>8);
		subparameter1->lengthLSB = (unsigned char)len;
		memcpy(sumbuff,subparameter1,len);
		subTotalLen += len;
		sumbuff +=len;

		subparameter1->idMSB = 0;
		subparameter1->idLSB = 2;	
		subparameter1->data[0] = 0x0;	
		len = 5;
		subparameter1->lengthMSL = (unsigned char)(len>>8);
		subparameter1->lengthLSB = (unsigned char)len;
		memcpy(sumbuff,subparameter1,len);
		subTotalLen += len;
		sumbuff +=len;
	
	subTotalLen +=4;
	parameter1->lengthMSL = (unsigned char)(subTotalLen>>8);
	parameter1->lengthLSB = (unsigned char)subTotalLen;
	memcpy(mpbuff,parameter1,subTotalLen);

	//Vehicle status Compent
	/*totalLen +=subTotalLen;
	mpbuff +=subTotalLen;	
	
	parameter1->idMSB = 0;
	parameter1->idLSB = 21;
	//group
		subTotalLen = 0;
		sumbuff = parameter1->data;
		subparameter1->idMSB = 0;
		subparameter1->idLSB = 0;
		subparameter1->data[0] = 0;
		subparameter1->data[1] = VEHICLE_STATUS_COMPENT_ID;
		len = 6;
		subparameter1->lengthMSL = (unsigned char)(len>>8);
		subparameter1->lengthLSB = (unsigned char)len;
		memcpy(sumbuff,subparameter1,len);
		subTotalLen += len;
		sumbuff +=len;

		subparameter1->idMSB = 0;
		subparameter1->idLSB = 1;
		pstr = "VehicleStatus";
		len = strlen(pstr)+1;
		memcpy(subparameter1->data,pstr,len);
		len = len+4;
		subparameter1->lengthMSL = (unsigned char)(len>>8);
		subparameter1->lengthLSB = (unsigned char)len;
		memcpy(sumbuff,subparameter1,len);
		subTotalLen += len;
		sumbuff +=len;

		subparameter1->idMSB = 0;
		subparameter1->idLSB = 3;		
		len = 4;
		subparameter1->lengthMSL = (unsigned char)(len>>8);
		subparameter1->lengthLSB = (unsigned char)len;
		memcpy(sumbuff,subparameter1,len);
		subTotalLen += len;
		sumbuff +=len;

		subparameter1->idMSB = 0;
		subparameter1->idLSB = 4;		
		len = 4;
		subparameter1->lengthMSL = (unsigned char)(len>>8);
		subparameter1->lengthLSB = (unsigned char)len;
		memcpy(sumbuff,subparameter1,len);
		subTotalLen += len;
		sumbuff +=len;

		subparameter1->idMSB = 0;
		subparameter1->idLSB = 6;		
		len = 4;
		subparameter1->lengthMSL = (unsigned char)(len>>8);
		subparameter1->lengthLSB = (unsigned char)len;
		memcpy(sumbuff,subparameter1,len);
		subTotalLen += len;
		sumbuff +=len;
	
	subTotalLen +=4;
	parameter1->lengthMSL = (unsigned char)(subTotalLen>>8);
	parameter1->lengthLSB = (unsigned char)subTotalLen;
	memcpy(mpbuff,parameter1,subTotalLen);*/

	//Local info Compent
	/*totalLen +=subTotalLen;
	mpbuff +=subTotalLen;	
	
	parameter1->idMSB = 0;
	parameter1->idLSB = 22;
	//group
		subTotalLen = 0;
		sumbuff = parameter1->data;
		subparameter1->idMSB = 0;
		subparameter1->idLSB = 0;
		subparameter1->data[0] = 0;
		subparameter1->data[1] = LOCAL_INFO_COMPENT_ID;
		len = 6;
		subparameter1->lengthMSL = (unsigned char)(len>>8);
		subparameter1->lengthLSB = (unsigned char)len;
		memcpy(sumbuff,subparameter1,len);
		subTotalLen += len;
		sumbuff +=len;

		subparameter1->idMSB = 0;
		subparameter1->idLSB = 1;
		pstr = "LocalInfo";
		len = strlen(pstr)+1;
		memcpy(subparameter1->data,pstr,len);
		len = len+4;
		subparameter1->lengthMSL = (unsigned char)(len>>8);
		subparameter1->lengthLSB = (unsigned char)len;
		memcpy(sumbuff,subparameter1,len);
		subTotalLen += len;
		sumbuff +=len;
	
	subTotalLen +=4;
	parameter1->lengthMSL = (unsigned char)(subTotalLen>>8);
	parameter1->lengthLSB = (unsigned char)subTotalLen;
	memcpy(mpbuff,parameter1,subTotalLen);*/
	
	totalLen +=subTotalLen;
	
	totalLen +=6;
	len = totalLen;
	pmsg->lengthMSL = (unsigned char)(totalLen>>8);
	pmsg->lengthLSB = (unsigned char)(totalLen);
	memcpy(pdata,pmsg,6);
	//Dump(pmsg,6);
	//Dump(p->data.playload,6);
	pdata +=6;
	totalLen -=6;
	memcpy(pdata,pbuff,totalLen);
	

	len += 10;
	p->msg.lengthMSL = (unsigned char)(len>>8);
	p->msg.lengthLSB = (unsigned char)len;
	
	send_packet(p,fd);
	
	put_apacket(p);
	free(pmsg);
	free(parameter1);
	free(subparameter1);
	free(pbuff);	

}
void AppLunchCarlife(apacket *m_apacket ,int fd)
{
	apacket *p =get_apacket();
	ctrl_session_msg *pmsg = (ctrl_session_msg*)malloc(sizeof(ctrl_session_msg));
	int len,totalLen;
	char * pdata = p->data.playload;
	parameter *p_parameter = (parameter*)malloc(sizeof(parameter));
	
	char *pstr = "com.baidu.lbs.CarLife";
	char *pbuff = (char *)malloc(MAX_CERTIFICATE_DATA);
	char * mpbuff;

	mpbuff = pbuff;
	//printf("\n enter AppLunchCarlife \n");
	totalLen = 0;
	p->msg.startMSL = 0xFF;
	p->msg.startLSB = 0x5A;
	p->msg.ctrl = 0x40; //iAP2 Session Payload be present
	p->msg.sequenceNum = m_apacket->msg.ackNum+1;
	p->msg.ackNum = m_apacket->msg.sequenceNum;
	p->msg.sessionID = m_apacket->msg.sessionID;
	// ctrl session msg
	pmsg->startMSL = 0x40;
	pmsg->startLSB = 0x40;
	pmsg->idMSB = 0xea;
	pmsg->idLSB = 0x02;
	//ctrl parameters group
		p_parameter->idMSB = 0;
		p_parameter->idLSB = 0;
		len = strlen(pstr)+1;
		memcpy(p_parameter->data,pstr,len);
		//printf("\nlen = %x\n",len);
		len +=4;
		p_parameter->lengthMSL =(unsigned char) (len>>8);
		p_parameter->lengthLSB = (unsigned char)len;

		memcpy(mpbuff,p_parameter,len);

		totalLen +=len;
		mpbuff +=len;
		p_parameter->idMSB = 0;
		p_parameter->idLSB = 1;
		p_parameter->data[0] = 0;
		//printf("\nlen = %x\n",len);
		len = 5;
		p_parameter->lengthMSL =(unsigned char) (len>>8);
		p_parameter->lengthLSB = (unsigned char)len;
		//printf("\np_parameter->lengthMSL = %x\n",p_parameter->lengthMSL);	
		//printf("\np_parameter->lengthLSB = %x\n",p_parameter->lengthLSB);	
		memcpy(mpbuff,p_parameter,len);

		totalLen +=len;

		totalLen +=6;
		len = totalLen;
		pmsg->lengthMSL = (unsigned char)(totalLen>>8);
		pmsg->lengthLSB = (unsigned char)(totalLen);
		memcpy(pdata,pmsg,6);
		//Dump(pmsg,6);
		//Dump(p->data.playload,6);
		pdata +=6;
		totalLen -=6;
		memcpy(pdata,pbuff,totalLen);

	len += 10;
	p->msg.lengthMSL = (unsigned char)(len>>8);
	p->msg.lengthLSB = (unsigned char)len;
	
	send_packet(p,fd);
	
	put_apacket(p);
	free(pmsg);
	free(p_parameter);
	free(pbuff);
}
void AppLunchCarlifeEx(int fd)
{
	apacket *p =get_apacket();
	ctrl_session_msg *pmsg = (ctrl_session_msg*)malloc(sizeof(ctrl_session_msg));
	int len,totalLen;
	char * pdata = p->data.playload;
	parameter *p_parameter = (parameter*)malloc(sizeof(parameter));	
	char *pstr = "com.baidu.lbs.CarLife";
	
	char *pbuff = (char *)malloc(MAX_CERTIFICATE_DATA);
	char * mpbuff;

	mpbuff = pbuff;
	
	//printf("\n enter AppLunchCarlife \n");
	totalLen = 0;
	p->msg.startMSL = 0xFF;
	p->msg.startLSB = 0x5A;
	p->msg.ctrl = 0x40; //iAP2 Session Payload be present
	p->msg.sequenceNum = g_iap2.ackNum+1;
	g_iap2.ackNum = g_iap2.ackNum+1;
	p->msg.ackNum = g_iap2.sequenceNum;
	p->msg.sessionID = g_iap2.sessionID;
	// ctrl session msg
	pmsg->startMSL = 0x40;
	pmsg->startLSB = 0x40;
	pmsg->idMSB = 0xea;
	pmsg->idLSB = 0x02;
	//ctrl parameters group
		p_parameter->idMSB = 0;
		p_parameter->idLSB = 0;
		len = strlen(pstr)+1;
		memcpy(p_parameter->data,pstr,len);
		//printf("\nlen = %x\n",len);
		len +=4;
		p_parameter->lengthMSL =(unsigned char) (len>>8);
		p_parameter->lengthLSB = (unsigned char)len;

		memcpy(mpbuff,p_parameter,len);

		totalLen +=len;
		mpbuff +=len;
		p_parameter->idMSB = 0;
		p_parameter->idLSB = 1;
		p_parameter->data[0] = 0;
		//printf("\nlen = %x\n",len);
		len = 5;
		p_parameter->lengthMSL =(unsigned char) (len>>8);
		p_parameter->lengthLSB = (unsigned char)len;
		//printf("\np_parameter->lengthMSL = %x\n",p_parameter->lengthMSL);	
		//printf("\np_parameter->lengthLSB = %x\n",p_parameter->lengthLSB);	
		memcpy(mpbuff,p_parameter,len);

		totalLen +=len;

		totalLen +=6;
		len = totalLen;
		pmsg->lengthMSL = (unsigned char)(totalLen>>8);
		pmsg->lengthLSB = (unsigned char)(totalLen);
		memcpy(pdata,pmsg,6);
		//Dump(pmsg,6);
		//Dump(p->data.playload,6);
		pdata +=6;
		totalLen -=6;
		memcpy(pdata,pbuff,totalLen);

	len += 10;
	p->msg.lengthMSL = (unsigned char)(len>>8);
	p->msg.lengthLSB = (unsigned char)len;
	
	send_packet(p,fd);

	/* usleep(10000);
	memset(p, 0, sizeof(apacket));
	len = read_packet(p,fd);
	if(len>0)
	{
		g_iap2.sequenceNum = p->msg.sequenceNum;
		g_iap2.ackNum = p->msg.ackNum;
		g_iap2.sessionID = p->msg.sessionID;				
	} */	
	put_apacket(p);
	free(pmsg);
	free(p_parameter);
	free(pbuff);
}


int IAP2_exist(void){
	if(access(iAP2_FILE_PATH,F_OK)== 0)
	{		
		return 1;
	}

	return -1;
}
/*
// Device(iphone)--------------------------------Accessory
// Detect iAP2 support                              |
// |						FF 55 02 00 EE 10		|				
// |<-----------------------------------------------|
// |	   FF 55 02 00 EE 10                        |
// |----------------------------------------------->|
// |Negotiate Link Parameters                       |
// |                         SYN[100]               |
// |<-----------------------------------------------|
// |	SYN[200] ACK[100]                           |
// |----------------------------------------------->|
// |							ACK[200]            |
// |<-----------------------------------------------|
// |  Request Authentication Certificate            |
// |----------------------------------------------->|
// |                  Authentication Certificate    |
// |<-----------------------------------------------|
// |Request Challenge Response                      |
// |----------------------------------------------->|
// |                        Challenge Response      |
// |<-----------------------------------------------|
// | Authentication Result[PASS]                    |
// |----------------------------------------------->|


*/

//return -1 Failed 0 success
int InitializationProcess(int fd)
{
	unsigned char InitData[]={0xff,0x55,0x02,0x00,0xEE,0x10};
	unsigned char * pbuff;
	short len;
	apacket *p = get_apacket();
	ctrl_session_msg *ctrl_msg = (ctrl_session_msg *)malloc(sizeof(ctrl_session_msg));
	int timeout = 15;

	pbuff = malloc(1024);
	g_iap2.pCertificateData = malloc(MAX_CERTIFICATE_DATA);
	g_iap2.status = IAP2_INITIALIZATION;
	g_iap2.CertificateDataLen = 0;

	SLOGI("IAP2 %s",(char*)IAPVER);
	for(;;)
	{
		timeout--;
	
		if(IAP2_exist()<0)
		{
			timeout = 0;
			break;
		}
		switch(g_iap2.status)
		{
			case IAP2_INITIALIZATION:
				   iap2_write(fd,InitData,sizeof(InitData));
				   usleep(10000);
				   len = iap2_read(fd,pbuff,1024);
 			  	  if(len>0)
 				  {   
 				  	if(memcmp(pbuff,InitData,sizeof(InitData)) == 0)
 				  	{    
						g_iap2.status =  IAP2_SYNCHRONIZATION;
					}
					else
					{
						 SLOGI("Initialization Response Data failed\n");
						  usleep(10000);
						  g_iap2.status = IAP2_INITIALIZATION;
					}
 					
 				  }
				  else
				  {
					g_iap2.status = IAP2_INITIALIZATION;
					// delay 1s
					usleep(10000);
				  }
				break;
			case IAP2_SYNCHRONIZATION:
				SendSycnLink(fd);
				usleep(10000);
				memset(p, 0, sizeof(apacket)/* - MAX_PAYLOAD*/);
				 len = read_packet(p,fd);
				   if(len>0)
 				  { 				  	
					g_iap2.status =  IAP2_REQUEST_AUTHENTICATION_CERTIFICATE;									
 					
 				  }
				  else
				  {
					g_iap2.status = IAP2_INITIALIZATION;
					// delay 1s
					usleep(10000);
				  }
				break;
		       case IAP2_REQUEST_AUTHENTICATION_CERTIFICATE:
			   	SendAckLink(p,fd);
				usleep(10000);
				memset(p, 0, sizeof(apacket));
				 len = read_packet(p,fd);
				 if(len>0)
 				 { 	
 				 	memcpy(ctrl_msg,p->data.playload,sizeof(ctrl_session_msg));
					if((ctrl_msg->idMSB == 0xAA)&&(ctrl_msg->idLSB == 0x00))
					{
						g_iap2.status =  IAP2_AUTHENTICATION_CERTIFICATE;
					}
					else
					{
						SLOGI(" Request Authentication Certificate  Failed\n");
						 usleep(10000);
						  g_iap2.status = IAP2_INITIALIZATION;
					}
 					
 				 }
				 else
				 {
					g_iap2.status = IAP2_INITIALIZATION;
					// delay 1s
					usleep(10000);
				  }
			   	break;
			case IAP2_AUTHENTICATION_CERTIFICATE:
				if(SendAuthenticationCertificate(p,fd)<0)
				{
					SLOGI("SendAuthenticationCertificate Failed\n");
					 g_iap2.status = IAP2_INITIALIZATION;
					 break;
				}
				usleep(10000);
				memset(p, 0, sizeof(apacket));
				 len = read_packet(p,fd);
				  if(len>0)
 				 {
 				 	memcpy(ctrl_msg,p->data.playload,sizeof(ctrl_session_msg));
					if((ctrl_msg->idMSB == 0xAA)&&(ctrl_msg->idLSB == 0x02))
					{
						g_iap2.status =  IAP2_REQUEST_AUTHENTICATION_CHALLENGE_RESPONSE;
					}
					else
					{	

						SLOGI("Request Authentication Challege Failed\n");
						usleep(10000);
						  g_iap2.status = IAP2_INITIALIZATION;
					}
 					
 				 }
				 else
				 {
					g_iap2.status = IAP2_INITIALIZATION;
					// delay 1s
					usleep(10000);
				  }
				break;
			case IAP2_REQUEST_AUTHENTICATION_CHALLENGE_RESPONSE:
			   	if(SendChallengeResponse(p,fd)<0)
			   	{
					 SLOGI("SendChallengeResponse Failed\n");
					 g_iap2.status = IAP2_INITIALIZATION;
					 break;
				}
				usleep(10000);
				memset(p, 0, sizeof(apacket));
				 len = read_packet(p,fd);
				 if(len>0)
 				 {
 				 	 memcpy(ctrl_msg,p->data.playload,sizeof(ctrl_session_msg));
					if((ctrl_msg->idMSB == 0xAA)&&(ctrl_msg->idLSB == 0x05))
					{
						g_iap2.status =  IAP2_STARTIDENTIFICATION;
					}
					else
					{	
						SLOGI("Authentication Failed\n");
						 usleep(10000);
						  g_iap2.status = IAP2_INITIALIZATION;
					} 					
 				 }
				 else
				 {
					g_iap2.status = IAP2_INITIALIZATION;
					// delay 1s
					usleep(10000);
				 }
				break;
			case IAP2_STARTIDENTIFICATION:
				memset(p, 0, sizeof(apacket));
				 len = read_packet(p,fd);
				 if(len>0)
 				 {
 				 	 memcpy(ctrl_msg,p->data.playload,sizeof(ctrl_session_msg));
					if((ctrl_msg->idMSB == 0x1D)&&(ctrl_msg->idLSB == 0x00))
					{
						g_iap2.status =  IAP2_IDENTIFICATION_INFO;
					}
					else
					{	
						SLOGI("Start Identification Failed\n");
						usleep(10000);
						 g_iap2.status = IAP2_INITIALIZATION;
					} 
 					
 				 }
				 else
				 {
					g_iap2.status = IAP2_INITIALIZATION;
					// delay 1s
					usleep(10000);
				 }
				break;				
			case IAP2_IDENTIFICATION_INFO:
				SendIdentificationInfo(p,fd);
				usleep(10000);
				memset(p, 0, sizeof(apacket));
				 len = read_packet(p,fd);
				  if(len>0)
 				 {
 					 memcpy(ctrl_msg,p->data.playload,sizeof(ctrl_session_msg));
					 
					if((ctrl_msg->idMSB == 0x1D)&&(ctrl_msg->idLSB == 0x02))
					{
                                         
					}
					else
					{     
						g_iap2.status = IAP2_INITIALIZATION;
						break;
					}
 				 }
				 else
				 {
					g_iap2.status = IAP2_INITIALIZATION;
					// delay 1s
					usleep(10000);
					break;
				 }
				 g_iap2.sequenceNum = p->msg.sequenceNum;
				 g_iap2.ackNum = p->msg.ackNum;
				 g_iap2.sessionID = p->msg.sessionID;
				 usleep(10000);
				 SendAckLink(p,fd);				
 				g_iap2.status =  IAP2_SUCCESS; 	
				break;			
			case IAP2_LUNCH_APP:			
				AppLunchCarlife(p,fd);	
				usleep(10000);
				memset(p, 0, sizeof(apacket));
				 len = read_packet(p,fd);
				 if(len>0)
 				 {
 					g_iap2.status =  IAP2_SUCCESS;					
 				 }
				 else
				 {
					g_iap2.status = IAP2_INITIALIZATION;
					// delay 1s
					usleep(10000);
				 }
				break;			
			case IAP2_SUCCESS:				
				break;
		}
		usleep(10000);
		if(g_iap2.status == IAP2_SUCCESS)
		{
			break;
		}
		if(timeout == 0)
		{
			break;
		}
	}	

	put_apacket(p);
	free(pbuff);
	free(g_iap2.pCertificateData);
	free(ctrl_msg);
	if(timeout > 0)
	{
		return 0;
	}
	else
	{
		return -1;
	}
}
