#include "Screen.h"

#include <log/log.h>

#include<fcntl.h>
#include <signal.h>
#include<unistd.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <asm/types.h>
#include "changan_protocol_parse.h"
#include <sys/times.h>
#define LOG_TAG "ScreenService"

namespace vendor {
namespace mediatek {
namespace hardware {
namespace screen {
namespace V1_0 {
namespace implementation {

static u8 pkg[I2CDATA_MAX_LEN_TOUCH_TYPE_CAP_LEN+256]={0};
static  struct mcu_cmd_packet* mcupk ;
static int fd = -1;

static u8 *data = NULL;
static    unsigned sz =0; 
static    unsigned per_package_size =512;
static    IapStatus g_state= IapStatus::IAP_DONE;
static    UpdateResult g_UpdateResult =  UPDATE_RESULT_INIT;
static    int g_UpdateAck =  0;
static    int g_total_num =1;
static    int g_send_num =0;
static    bool restart = false;
static    bool finish = false;
static    unsigned long  start_time;
static    unsigned long  end_time;
static u8 checksum(u8 * buf, int size)
{
	u8 sum=0;
	int i=0;
	
	for(; i<size; i++)
	{
		sum += buf[i];
	}
	sum++;
	return sum;
}

 u8* Screen::load_file(const char *file, unsigned *sz)
{
        u8 *data;
        long size =0;
		long read_size=0;
		long temp_size=0;
        //int fd;
        FILE *fd;
        fd = fopen(file, "rb");
        if (fd == NULL)
		{    
			   ALOGE("can not open %s  %s\n",file,strerror(errno));
               return NULL;
		}
		fseek(fd,0L,SEEK_END);
        size = ftell(fd);
        if (size <=0)
                goto fail;

        ALOGE("%s lenth=%d  sizeof(char)=%d \n",file,size,sizeof(char));
        data = (u8 *)malloc(size);
        if (!data)
                goto fail;
		fseek(fd,0L,SEEK_SET);
		while(read_size < size)
        {
          temp_size = fread( (data+read_size), sizeof(u8),1024,fd);
		   if(temp_size <=0)
		   {
		      ALOGE("read %s fail,read_size =%d,   %s\n",file,read_size,strerror(errno));
		   }else
		   {
			 read_size = read_size+temp_size;
		     //printf("read_size =%d \n",read_size);
		   }

		}
        if (read_size != size) {
                free(data);
                goto fail;
        }

        fclose(fd);
        *sz = size;
        return (u8*)data;

fail:
        ALOGE("failed loading file\n");
        fclose(fd);
        return 0;
}

u8 * Screen::load_data_file(const char *file, unsigned *size)
{
	    u8 *data;
	    ALOGE("start to load file =%s \n",file);
        
        data = load_file(file, size);

        return data;
}

Screen::Screen() {
	state = IapStatus::IAP_DONE;
	mCallback = NULL;
	mFd = -1;
    mcupk = (struct mcu_cmd_packet*)pkg;
	ALOGE("screenImpl Init status:%d", state);
}


// Methods from ::vendor::mediatek::hardware::screen::V1_0::IScreen follow.
Return<::vendor::mediatek::hardware::screen::V1_0::IapStatus> Screen::get() {
    // TODO implement
    //return ::vendor::mediatek::hardware::screen::V1_0::IapStatus {};
	state = g_state;
	return state;

}

Return<void> Screen::init() {
    // TODO implement
    mExit = false;
	ALOGE("init \n");
	run("Screen");

    return Void();
}

Return<void> Screen::release() {
    // TODO implement
     mExit = true;
	 if(data != NULL)
	 	{
	       free(data);
		   data = NULL;
	 	}

    return Void();
}

Return<void> Screen::setCallback(const sp<::vendor::mediatek::hardware::screen::V1_0::IScreenCallBack>& callback) {
    // TODO implement
   mCallback = callback;
 if(mCallback != nullptr) {
     ALOGD("setCallback: done");
 }


    return Void();
}



 int Screen::start_update(int fd ,UpdateType type) {  
    ssize_t err = 0; 
    char cmd;
	int len =0;
	struct IAP_mode iap_mode_pkg;
	struct up_pk_amount update_amount_pkg;
	struct update_pk update_pkg;
    iap_mode_pkg.cmdid = IAP_COMMAND;
	iap_mode_pkg.cmd= 0x01;
#if 0	
	if(type == UpdateType::FIRMWARE_MCU)
		{
	      iap_mode_pkg.pwd[0]=0xc2;
	      iap_mode_pkg.pwd[1]=0x01;
	      iap_mode_pkg.pwd[2]=0xA1;
		}

	 if(type == UpdateType::FIRMWARE_TOUCH)
	    {

		  iap_mode_pkg.pwd[0]=0xc2;
	      iap_mode_pkg.pwd[1]=0xAA;
	      iap_mode_pkg.pwd[2]=0xFF;

	    }
#endif

          iap_mode_pkg.pwd[0]=data[sz-3];
          iap_mode_pkg.pwd[1]=data[sz-2];
          iap_mode_pkg.pwd[2]=data[sz-1];
	      ALOGE("iap_mode_pkg pwd-3=0x%x ,pwd-2=0x%x,pwd-1=0x%x!\n",iap_mode_pkg.pwd[0],iap_mode_pkg.pwd[1],iap_mode_pkg.pwd[2]);	  
          iap_mode_pkg.EXT_length = 0;
    iap_mode_pkg.chsum = checksum((u8 *)(&iap_mode_pkg), sizeof(struct IAP_mode)-1);
	len = write(fd, (u8 *)(&iap_mode_pkg), sizeof(struct IAP_mode));
    if(len<0){
		     ALOGE("<mfd>send iap mode failed!\n");
		    err = -EFAULT;
		}
	else
		{

           ALOGD("start_update write =%d! fd=%d \n",len,fd);
	    }

    return err;  
}

 #define MAX_TRY 30
 #define TIMEOUT_START 60
 #define TIMEOUT 60


Return<void> Screen::upgrade(const hidl_string& path, ::vendor::mediatek::hardware::screen::V1_0::UpdateType type, upgrade_cb _hidl_cb) {
    // TODO implement
   int count =0;
    g_UpdateAck =0;
   unsigned long HZ = sysconf(_SC_CLK_TCK);
   ALOGE("update file path =  %s \n",path.c_str());
   
   if(data != NULL)
   	{
      free(data); //make sure it is free
	  data = NULL;
     }
   
   data =Screen::load_data_file(path.c_str(),&sz);
   if((data == NULL) || sz <=0)
 	 { // mExit = true;
 	    _hidl_cb(Status::FAIL,0xff);
		return Void();

     }else
     {
       ALOGE("load %s successful lenth=  %d \n",path.c_str(),sz);
     }

  while((mFd <= 0) && (count <MAX_TRY))
  {
    ALOGE("waiting mFd =  %d \n",mFd);
    //sleep(1);
     ::sleep(1);
    count=count+1;
  }
 
  if(mFd > 0)
  {
  count =0;
  start_update(mFd,type);
  ::sleep(4);

    start_time=(unsigned long ) times(NULL);
	end_time = start_time;
  while(( g_UpdateAck == 0) && ((end_time - start_time) < (TIMEOUT_START*HZ)))
  {
    ALOGD("waiting for UpdateAck,g_UpdateAck =  %d ,time=%d \n",g_UpdateAck,(end_time - start_time)/HZ );
	::sleep(4);
	 start_update(mFd,type);
	 
	 end_time = (unsigned long ) times(NULL);
   // count=count+1;
  }
  if((end_time - start_time) >= (TIMEOUT_START*HZ))
  	{
         _hidl_cb(Status::TIME_OUT,0xff);
		   return Void();


    }

  
   // start_update(mFd,type);
	g_send_num = 0;
	g_total_num = 1;
	count =0;

	//waiting for restart
	finish = false;
	restart = false;
	start_time=(unsigned long ) times(NULL);
	end_time = start_time;
	
	
	while(((end_time - start_time) < (TIMEOUT_START*HZ))&& (!restart)  )
		{
		  //ALOGD("waiting for restart time=  %d \n",(end_time - start_time)/HZ);
		  sleep(1);
		  end_time = (unsigned long ) times(NULL);

	    }

	  if(((end_time - start_time) >=(TIMEOUT_START*HZ)))
	  	{
           _hidl_cb(Status::TIME_OUT,0xff);
		   return Void();


	     }
     //waiting for finish 
      //count =0;
      start_time=(unsigned long ) times(NULL);
	  end_time = start_time;
	  while(((end_time - start_time) < (TIMEOUT*HZ))&&(!finish))
		{
		  // ALOGD("waiting for finish time=  %d \n",(end_time - start_time)/HZ);
		   ::sleep(1);
		   end_time = (unsigned long ) times(NULL);

	    }

	  if(((end_time - start_time) >=(TIMEOUT*HZ)))
	  	{
           _hidl_cb(Status::TIME_OUT,0xff);
		   return Void();

         }


		
   }
 else
 {
     ALOGD("  mFd=%d \n",mFd);
	 // mExit = true;
	 _hidl_cb(Status::FAIL,0xff);
		return Void();
 }
  _hidl_cb(Status::SUCCESSFUL,0xff);

    return Void();
}

Return<int32_t> Screen::set(::vendor::mediatek::hardware::screen::V1_0::IapStatus val) {
    // TODO implement
    return int32_t {};
}

#define SCREEN_PATH "/dev/lcdtsp_mcu"

#if 0
 void Screen::handle_IAP_status_info(int fd,struct IAP_status * pinfo)
{
   
	pinfo = (struct IAP_status*)(pkg+1);
	unsigned short temp =0x01;
   
     ALOGE(" handle_IAP_status_info  IAP_status=%d ,data_len=%d \n", pinfo->status,pinfo->data_len);
	 g_state=(IapStatus)pinfo->status;
	 if (pinfo->status == 0x01)
	 	{    if(pinfo->data_len > 0)
	 	    {
             per_package_size =(temp<<pinfo->data_len)*4;
             //ALOGE("set  per_package_size =%d \n",per_package_size);
	 	    }
	    }
	 

}
#endif

 void Screen::handle_update_result(int fd,struct update_result * pinfo)
{
   
	pinfo = (struct update_result*)(pkg+1);
	
   
     ALOGE(" handle_update_result  =%d  \n", pinfo->result);
	 g_UpdateResult=(UpdateResult)pinfo->result;
	 

}


 void Screen::handle_request_package_total_num(int fd)
{
   
   unsigned int package_num =0;
   
   struct up_pk_amount update_amount_pkg;
   package_num = sz /per_package_size;
      if(sz /per_package_size)
      {
        package_num = package_num + 1;
      }
       if(package_num >0)
        {
           g_total_num = package_num;
        }

      ALOGE("tootal size =%d per_pkg_size=%d  pkg_num=%d !\n",(int)sz,(int)per_package_size,package_num);
      update_amount_pkg.cmdid = UPDATE_PKG_NUM;
      update_amount_pkg.pkg_num[0]=0xff & (package_num >>8);
      update_amount_pkg.pkg_num[1]=0xff & package_num;
 //     update_amount_pkg.reserved[0]=0x0;
   //   update_amount_pkg.reserved[1]=0x0;
      update_amount_pkg.EXT_length = 0;
      update_amount_pkg.chsum = checksum((u8 *)&update_amount_pkg, sizeof(struct up_pk_amount)-1);
      if(write(fd, (u8 *)&update_amount_pkg, sizeof(struct up_pk_amount))<0){
            ALOGE("<mfd>send pkg amount failed!\n");

        }

	
   

	
	 

}


void Screen::handle_update_ack()
{
     g_UpdateAck = 1;  
}


void Screen::handle_req_update_pkg(int fd,struct req_update * pinfo)
{
   //struct up_pk_amount update_amount_pkg;
   struct update_pk update_pkg;
   void *ret =NULL;
   //unsigned int pkg_size =0;
   unsigned int package_num =0;
  // pkg_size=pinfo->pkg_num[0];
   //pkg_size = ((pkg_size<<8)|pinfo->pkg_num[1]);
   unsigned int request_num =0;
   request_num = pinfo->pkg_num[0];
   request_num = (request_num <<8) | pinfo->pkg_num[1];
   ALOGE("request_num =0x%x !\n",request_num);
   g_send_num = request_num;
   if(request_num == 0) //first package send update_amount_pkg
   	{ 

      ALOGE("error ---- request_num==0 ,it should not be !\n");
	 #if 0
      package_num = sz /per_package_size;
      if(sz /per_package_size)
   	  {
        package_num = package_num + 1;
      }
       if(package_num >0)
       	{
           g_total_num = package_num;
	    }
	  
      ALOGE("tootal size =%d per_pkg_size=%d  pkg_num=%d !\n",(int)sz,(int)per_package_size,package_num);
      update_amount_pkg.cmdid = UPDATE_PKG_NUM;
      update_amount_pkg.pkg_num[0]=0xff & (package_num >>8);
      update_amount_pkg.pkg_num[1]=0xff & package_num;
      update_amount_pkg.reserved[0]=0x0;
      update_amount_pkg.reserved[1]=0x0;
      update_amount_pkg.EXT_length = 0;
      update_amount_pkg.chsum = checksum((u8 *)&update_amount_pkg, sizeof(struct up_pk_amount)-1);
      if(write(fd, (u8 *)&update_amount_pkg, sizeof(struct up_pk_amount))<0){
		    ALOGE("<mfd>send pkg amount failed!\n");
		 
		}

	#endif	
   	}else
   	{
       update_pkg.cmdid = UPDATE_PKG;
	   update_pkg.packetIndex[0]= pinfo->pkg_num[0];
	   update_pkg.packetIndex[1]= pinfo->pkg_num[1];
	   update_pkg.size = per_package_size;
	   if((data == NULL) || (sz == 0))
	   	{
          ALOGE("update file has not load to data sz=%d!\n",sz);
		  return ;
	    }
	   memset(update_pkg.data,0,per_package_size);  //all package data set "0"
	   if((per_package_size*request_num) <=sz)
	   	
	   	{

		   ret =memcpy(update_pkg.data,data+(per_package_size*(request_num-1)),per_package_size);
		   if(ret == NULL)
			{
			  ALOGE("package memcmp failed error=%s!\n",strerror(errno));
			  return ;
             }
			
	   	}
	    else //last package 
	    {
  	
            ret =memcpy(update_pkg.data,data+(per_package_size*(request_num-1)),sz-(per_package_size*(request_num-1)));
		    if(ret == NULL)
				{
			      ALOGE("last package memcmp failed error=%s!\n",strerror(errno));
			     return ;
                }
      
            
		}
        update_pkg.EXT_length = 0;
        update_pkg.chsum = checksum((u8 *)(&update_pkg), sizeof(struct update_pk)-1);
        if(write(fd, (u8 *)(&update_pkg), sizeof(struct update_pk))<0){
		     ALOGE("<mfd>send pkg failed!\n");
		}


	}

   
}




 void Screen::on_data_change(int signum)
{
int len =0;
//ALOGE("got data --- weihong\n");
if(fd > 0)
{
   len = read(fd,pkg, I2CDATA_MAX_LEN_TOUCH_TYPE_CAP_LEN);

	switch(mcupk->cmd_id)
 		{
#if 0				
			case IAP_STATUS:
				//ALOGE("IAP_STATUS \n");
				handle_IAP_status_info(fd, (struct IAP_status *)(pkg+1));
				break;
#endif
            case UPDATE_RESULT:
                //ALOGE("IAP_STATUS \n");
                
				handle_update_result( fd,(struct update_result *)(pkg+1));
                break;

            case UPDATE_PACKAGE_TOTAL_NUM:
                //ALOGE("IAP_STATUS \n");
                
				
                handle_request_package_total_num(fd);
                break;




			case UPDATE_ACK:
                //ALOGE("IAP_STATUS \n");
                
				handle_update_result( fd,(struct update_result *)(pkg+1));
                break;


			case REQUEST_UPDATE_PKG:
				//ALOGE(" REQUEST_UPDATE_PKG \n");
				handle_req_update_pkg(fd, (struct req_update *)(pkg+1));
				
				break;
			case RESERVED:
				ALOGE(" RESERVED \n");
				break;
		


			
			default:
				 ALOGE("<mfd>TP cmdid(0x%x) wrong!\n", mcupk->cmd_id);
 		}
    
}else
  {

       ALOGE("fd < 0 fd=%d \n",fd );
  }
}
bool Screen::threadLoop()
{
 static int32_t count = 0;
 int Oflags;

 ALOGD("start  threadLoop \n" );
 signal(SIGIO, Screen::on_data_change);
 
 mFd = open(SCREEN_PATH , O_RDWR);
        if (mFd < 0)
        {
                ALOGE("can't open! errno=%s\n",strerror(errno));
				return false;
        }
        else
        {
                 ALOGE("open %s successful mFd =%d \n",SCREEN_PATH,mFd);
		}
        fcntl(mFd, F_SETOWN, getpid());
        Oflags = fcntl(mFd, F_GETFL);
        fcntl(mFd, F_SETFL, Oflags | FASYNC);

		fd = mFd;


 while(!mExit) {
 
 ::sleep(1);
 // ALOGE("call back g_state =%d \n",g_state);
 if((g_send_num >=1 )&&(g_send_num <g_total_num ))
 	{
 	  finish = false;
 	  restart = true;
 	  if(mCallback != NULL)
 		{  // ALOGE("service call back  =%d \n");
            mCallback->onNotify(g_send_num*100/g_total_num);
			
	    }
	  
 	}

   if(/*(g_state == IapStatus::IAP_DONE)&&*/(restart)&&(g_send_num == g_total_num))
   	{
   	    finish = true;
		restart = false;
        if(mCallback != NULL)
 		{   //ALOGE("service call back  =%d \n");
            mCallback->onNotify(100);
			
	    }
    }

 }
 ALOGD("threadLoop: exit");
 close(mFd);
  ALOGE("close  mFd =%d \n",mFd);
 return false;
}



Return<bool> Screen::beginCalibration() {
    // TODO implement

      int fd = open(SCREEN_PATH , O_RDWR);
        if (fd < 0)
        {
                ALOGE("beginCalibration can't open! errno=%s\n",strerror(errno));
                return false;
        }
        else
        {
            struct IAP_mode iap_mode_pkg;

            iap_mode_pkg.cmdid = IAP_COMMAND;
            iap_mode_pkg.cmd= 0x02;
            iap_mode_pkg.pwd[0]=0xc2;
            iap_mode_pkg.pwd[1]=0xBB;
            iap_mode_pkg.pwd[2]=0x01;

            iap_mode_pkg.EXT_length = 0;
            iap_mode_pkg.chsum = checksum((u8 *)(&iap_mode_pkg), sizeof(struct IAP_mode)-1);
            u8 len = write(fd, (u8 *)(&iap_mode_pkg), sizeof(struct IAP_mode));
            if(len<0){
                 ALOGE("<mfd>send iap mode failed!\n");
                 return false;
            }
            else
            {
               ALOGD("beginCalibration OVER write =%d! fd=%d \n",len,fd);
            }
        }
        return true;


    return bool {};
}


Return<void> Screen::tpDuanluzijian(tpDuanluzijian_cb _hidl_cb) {
    // TODO implement
        ALOGE("tpDuanluzijian Enter");
        int fd = open(SCREEN_PATH , O_RDWR);
        if (fd < 0)
        {
                ALOGE("tpDuanluzijian can't open! errno=%s\n",strerror(errno));
                 _hidl_cb(Status::FAIL,0x24);
        }
        else
        {
            struct IAP_mode iap_mode_pkg;

            iap_mode_pkg.cmdid = 0x82;
            iap_mode_pkg.cmd= 0x12;
            iap_mode_pkg.pwd[0]=0;
            iap_mode_pkg.pwd[1]=0;
            iap_mode_pkg.pwd[2]=0;
            iap_mode_pkg.EXT_length = 0;

            iap_mode_pkg.chsum = checksum((u8 *)(&iap_mode_pkg), sizeof(struct IAP_mode)-1);
            u8 len = write(fd, (u8 *)(&iap_mode_pkg), sizeof(struct IAP_mode));
            if(len<0){
                 _hidl_cb(Status::FAIL,0x24);
            }
            else
            {
                        ALOGD("tpDuanluzijian OVER write =%d! fd=%d \n",len,fd);


                        usleep(5*1000*1000);
                         ALOGE("Callback to App State %d",g_state);
                         _hidl_cb(Status::SUCCESSFUL,(int)g_state);


            }

        }


    return Void();
}



// Methods from ::android::hidl::base::V1_0::IBase follow.

//IScreen* HIDL_FETCH_IScreen(const char* /* name */) {
    //return new Screen();
//}
//
}  // namespace implementation
}  // namespace V1_0
}  // namespace screen
}  // namespace hardware
}  // namespace mediatek
}  // namespace vendor
