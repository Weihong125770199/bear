#define LOG_TAG "vendor.mediatek.hardware.screen@1.0-service"

#include <vendor/mediatek/hardware/screen/1.0/IScreen.h>
#include <hidl/LegacySupport.h>
#include "Screen.h"
using vendor::mediatek::hardware::screen::V1_0::IScreen;
using vendor::mediatek::hardware::screen::V1_0::implementation::Screen;
using android::hardware::defaultPassthroughServiceImplementation;
using android::hardware::configureRpcThreadpool;
using android::hardware::joinRpcThreadpool;
using android::sp;


int main() {

#if 0
// Passthrough   dlopen so方式
    return defaultPassthroughServiceImplementation<IScreen>();
#else
// Binder 方式

  sp<IScreen> service =new Screen();
  configureRpcThreadpool(1, true /*callerWillJoin*/);
    if(android::OK !=  service->registerAsService())
      return 1;
    joinRpcThreadpool();
#endif
}

