#define LOG_TAG "SCREEN_CLINET"
//#include <android/hardware/screen/1.0/IScreen.h>
#include <vendor/mediatek/hardware/screen/1.0/IScreen.h>
#include <vendor/mediatek/hardware/screen/1.0/IScreenCallBack.h>
#include <log/log.h>
 #include <sys/times.h>
using vendor::mediatek::hardware::screen::V1_0::IScreen;
using vendor::mediatek::hardware::screen::V1_0::IScreenCallBack;
using vendor::mediatek::hardware::screen::V1_0::IapStatus;
using vendor::mediatek::hardware::screen::V1_0::UpdateType;
using vendor::mediatek::hardware::screen::V1_0::Status;
using vendor::mediatek::hardware::screen::V1_0::BrightnessRange;
using android::hardware::hidl_vec;
using android::sp;
using android::hardware::Void;
using android::hardware::Return;

 #define PROGRESS_BAR_WIDTH 25

void dfu_progress_bar(const char *desc, unsigned  long curr,
                unsigned  long max)
{
        static char buf[PROGRESS_BAR_WIDTH + 1];
        static unsigned  long last_progress = -1;
         static unsigned long last_time =0;
        unsigned long curr_time = (unsigned long ) times(NULL);
        unsigned  long progress;
        unsigned  long x;
        
        /* check for not known maximum */
        if (max < curr)
                max = curr + 1;
        /* make none out of none give zero */
        if (max == 0 && curr == 0)
                max = 1;

        /* compute completion */
        progress = (PROGRESS_BAR_WIDTH * curr) / max;
        if (progress > PROGRESS_BAR_WIDTH)
                progress = PROGRESS_BAR_WIDTH;
        if (progress == last_progress &&
            curr_time == last_time)
                return;
        last_progress = progress;
        last_time = curr_time;

        for (x = 0; x != PROGRESS_BAR_WIDTH; x++) {
                if (x < progress)
                        buf[x] = '=';
                else
                        buf[x] = ' ';
        }
        buf[x] = 0;       
		 printf("\r%s\t[%s] %3lld%% %12lld bytes", desc, buf,
            (long long)((100 * curr) / max),(long long) (curr*128));
         fflush(stdout);
        if (progress == PROGRESS_BAR_WIDTH)
                printf("\n%s done.\n", desc);
}


 void update_callback(Status result, int32_t process)
 {
       // ALOGE("update ret: %d",result);
		ALOGE("update result=%d  precentage : %d \n",result,process);
		if(result == Status::SUCCESSFUL)
			{
              printf("update successful \n");
		    }
		if(result == Status::TIME_OUT)
			{
              printf("update time out \n");
		    }

		if(result == Status::FAIL)
			{
              printf("update fail \n");
		    }
	   //return ;
 }


class ScreenCallback: public IScreenCallBack {
public:
 ScreenCallback() {

}

~ScreenCallback() {

}

Return <void> onNotify(int32_t process) {

 //printf("onNotify: update  = %lu/100 \n", (unsigned long)process);
 dfu_progress_bar("updateing",(unsigned long)process,100);
 

 return Void();
}

};




int main(int arg,char **args){
	// BrightnessRange range;
const char *MCU_PATH = "/sdcard/TPMcuUpdate.mhx";
const char *TOUCH_PATH = "/sdcard/Coagent F580_15.6inch_AT81688.bin";
    if(arg >=2)
    	{

          MCU_PATH=args[1];
		  
	    }

	  if(arg >=3)
    	{

          TOUCH_PATH=args[2];
		  
	    }
	  
	sp<IScreen> service = IScreen::getService();
	if( service == nullptr ){
		ALOGE("Can't find ILed service...");
		return -1;
	}

	

	sp<ScreenCallback> callback = new ScreenCallback();
    service->setCallback(callback);
	
	service->init();
	printf("IScreen init \n");

    

	
	
	printf("IScreen get \n");
	IapStatus ret = service->get();
	printf("IScreen get: %d \n",ret);

	printf("IScreen upgade:%s \n",MCU_PATH);    

    service->upgrade(MCU_PATH,UpdateType::FIRMWARE_MCU,update_callback);


	printf("IScreen upgade:%s \n",TOUCH_PATH);
	service->upgrade(TOUCH_PATH,UpdateType::FIRMWARE_TOUCH,update_callback);
	
    service->release();
	return 0;
}
