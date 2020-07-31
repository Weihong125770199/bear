#include "ScreenCallBack.h"

namespace vendor {
namespace mediatek {
namespace hardware {
namespace screen {
namespace V1_0 {
namespace implementation {

// Methods from ::vendor::mediatek::hardware::screen::V1_0::IScreenCallBack follow.
Return<void> ScreenCallBack::onNotify(int32_t process) {
    // TODO implement
    return Void();
}


// Methods from ::android::hidl::base::V1_0::IBase follow.

//IScreenCallBack* HIDL_FETCH_IScreenCallBack(const char* /* name */) {
    //return new ScreenCallBack();
//}
//
}  // namespace implementation
}  // namespace V1_0
}  // namespace screen
}  // namespace hardware
}  // namespace mediatek
}  // namespace vendor
