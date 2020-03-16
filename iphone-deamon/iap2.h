
#ifndef __IAP2_H
#define __IAP2_H

int iap2_open();

void iap2_close(int fd);

int InitializationProcess(int fd);

void AppLunchCarlifeEx(int fd);

#endif //__IAP2_H
