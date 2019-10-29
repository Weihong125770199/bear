#include "stdafx.h"
#include "CommUtils.h"
#include "stdio.h"
const int READ_TIMEOUT = 500;

CommUtils::CommUtils()
{
    bOpenCom = false;
}

CommUtils::~CommUtils()
{
    this->CloseCom();
}




bool CommUtils::OpenCom(int Port,unsigned long BaudRate )
{
    if (bOpenCom)
    {
        this->CloseCom();
        bOpenCom = false;
    }
    char szport[10];
    sprintf(szport,"\\\\.\\COM%d",Port);
    hComm = CreateFile(szport,GENERIC_READ|GENERIC_WRITE, 0,NULL,OPEN_EXISTING,/*FILE_FLAG_OVERLAPPED*/0,NULL);

	int error=GetLastError();

    if (hComm == INVALID_HANDLE_VALUE)
	{
		TRACE("CreateFile fial hComm=%d  ---- \n",hComm);
	    return false;
	}
    if (!SetupComm(hComm, 1024, 1024))   
	{
			TRACE("SetupComm fail \n");
		return false;
	}
    COMMTIMEOUTS commtimeouts;
    commtimeouts.ReadIntervalTimeout = 10;
    commtimeouts.ReadTotalTimeoutConstant =500;
    commtimeouts.ReadTotalTimeoutMultiplier =0;
    commtimeouts.WriteTotalTimeoutConstant =0;
    commtimeouts.WriteTotalTimeoutMultiplier=0;

    if (!SetCommTimeouts(hComm, &commtimeouts))        return false;

    memset(&ReadovReady,0,sizeof(OVERLAPPED));
    memset(&WriteovReady,0,sizeof(OVERLAPPED));
    ReadovReady.hEvent = CreateEvent(NULL,TRUE,FALSE,NULL);
    WriteovReady.hEvent =CreateEvent(NULL,TRUE,FALSE,NULL);

    SECURITY_ATTRIBUTES sa;
    sa.nLength=sizeof(SECURITY_ATTRIBUTES);
    sa.lpSecurityDescriptor=NULL;
    sa.bInheritHandle=TRUE;

    DCB dcb;
    GetCommState(hComm, &dcb);
   // dcb.fBinary = TRUE;
    //dcb.fParity = TRUE;
    // 波特率  数据位  标志位 根据自己的设备在此做修改 
   // dcb.BaudRate = CBR_115200;        // baud rate 9600
	   dcb.BaudRate =BaudRate;
    dcb.ByteSize = 8;               
    dcb.Parity = NOPARITY;            
    dcb.StopBits = ONESTOPBIT;        

    if (!SetCommState(hComm, &dcb ))        return false;

    bOpenCom = true;
    global_BaudRate = BaudRate;
    return bOpenCom;
}

bool CommUtils::WriteCom(unsigned char *sendchar, int sendsize)
{
    if (!bOpenCom)    return false;

    DWORD    BytesSent;
    DWORD    resD;        

   // PurgeComm(hComm, PURGE_RXCLEAR | PURGE_TXCLEAR | PURGE_RXABORT | PURGE_TXABORT);
    PurgeComm(hComm,  PURGE_TXCLEAR| PURGE_TXABORT);
    BytesSent=0;
    BOOL hr = WriteFile(hComm,                            // Handle to COMM Port
                        sendchar,                        // Pointer to message buffer in calling finction
                        sendsize,                        // Length of message to send
                        &BytesSent,                        // Where to store the number of bytes sent
                        &WriteovReady);                    // Overlapped structure
    if(!hr)
    {
        if(GetLastError() != ERROR_IO_PENDING)
        {
            return false;
        }
        else
        {
            resD=WaitForSingleObject(WriteovReady.hEvent,INFINITE);
        }
        switch(resD)
        {
            case WAIT_OBJECT_0:
            {
                if(!GetOverlappedResult(hComm,&WriteovReady,&BytesSent,false))
                    return false;
                else
                    return true;

            }
            default:
                return false;
                break;
        }
    }
    return true;
}

void CommUtils::CloseCom()
{
    if (!bOpenCom)    return;

    CloseHandle(hComm);
    hComm=NULL;

    CloseHandle(ReadovReady.hEvent);
    CloseHandle(WriteovReady.hEvent );
    ReadovReady.hEvent =NULL;
    WriteovReady.hEvent =NULL;
}

bool CommUtils::ReadCom(unsigned char * ReceiveData, DWORD& ReceiveLength ,int & result)
{
    if (!bOpenCom)
	{
		result = 1;
		return false;
	}
    if (ReadovReady.hEvent == NULL)  
	{
        result = 2;
		return false;
	}
   // PurgeComm(hComm, PURGE_RXCLEAR  | PURGE_RXABORT );
   //ReceiveLength = 0;
    if (ReadFile(hComm, ReceiveData,ReceiveLength , &ReceiveLength,&ReadovReady) == FALSE) 
    {
        if (GetLastError() != ERROR_IO_PENDING)
		{
            result = 3;
			return false;
		}
    }
    /*
    bResult = ReadFile(port->m_hComm,  // Handle to COMM port 
                &RXBuff,    // RX Buffer Pointer
                 1,     // Read one byte              
                 &BytesRead,   // Stores number of bytes read               
                 &port->m_ov);  // pointer to the m_ov structure                              
                 // deal with the error code */
    if(ReceiveLength == 0)
	{
         result = 4;
		return false;
	}
    ReceiveData[ReceiveLength] = 0;

    DWORD dwRead;
    DWORD dwRes = WaitForSingleObject(ReadovReady.hEvent, READ_TIMEOUT);
    switch(dwRes)
    {
        case WAIT_OBJECT_0:
            if (!GetOverlappedResult(hComm, &ReadovReady, &dwRead, FALSE))
			{
				 result = 5;
				return false;
			}
            break;

        case WAIT_TIMEOUT:
            break;                

        default:
            break;
    }
    return true;
}


