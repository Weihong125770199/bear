#ifndef VENDOR_MEDIATEK_HARDWARE_SCREEN_V1_0_SCREENCALLBACK_H
#define VENDOR_MEDIATEK_HARDWARE_SCREEN_V1_0_SCREENCALLBACK_H

#include <vendor/mediatek/hardware/screen/1.0/IScreenCallBack.h>
#include <hidl/MQDescriptor.h>
#include <hidl/Status.h>

namespace vendor {
namespace mediatek {
namespace hardware {
namespace screen {
namespace V1_0 {
namespace implementation {

using ::android::hardware::hidl_array;
using ::android::hardware::hidl_memory;
using ::android::hardware::hidl_string;
using ::android::hardware::hidl_vec;
using ::android::hardware::Return;
using ::android::hardware::Void;
using ::android::sp;

struct ScreenCallBack : public IScreenCallBack {
    // Methods from ::vendor::mediatek::hardware::screen::V1_0::IScreenCallBack follow.
    Return<void> onNotify(int32_t process) override;

    // Methods from ::android::hidl::base::V1_0::IBase follow.

};

// FIXME: most likely delete, this is only for passthrough implementations
// extern "C" IScreenCallBack* HIDL_FETCH_IScreenCallBack(const char* name);

}  // namespace implementation
}  // namespace V1_0
}  // namespace screen
}  // namespace hardware
}  // namespace mediatek
}  // namespace vendor

#endif  // VENDOR_MEDIATEK_HARDWARE_SCREEN_V1_0_SCREENCALLBACK_H
