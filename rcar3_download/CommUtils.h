#ifndef _CommUtils_H__
#define _CommUtils_H__

class CommUtils  
{
public:
	unsigned long global_BaudRate;
    bool ReadCom(unsigned char * ReceiveData, DWORD& ReceiveLength,int & result);
    void CloseCom();
    bool WriteCom(unsigned char * sendchar,int sendsize);
    bool OpenCom(int Port,unsigned long BaudRate );
    bool bOpenCom;
    CommUtils();
    virtual ~CommUtils();
      int m_Port;
    char szCurPath[256];
    
private:

    OVERLAPPED ReadovReady, WriteovReady;
    HANDLE hComm;
    
};

#endif
