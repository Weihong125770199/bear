#ifndef VENDOR_MEDIATEK_HARDWARE_SCREEN_V1_0_SCREEN_H
#define VENDOR_MEDIATEK_HARDWARE_SCREEN_V1_0_SCREEN_H

#include <vendor/mediatek/hardware/screen/1.0/IScreen.h>
#include <hidl/MQDescriptor.h>
#include <hidl/Status.h>
#include <utils/Thread.h>
#include <asm/types.h>
#include "changan_protocol_parse.h"

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

struct Screen : public IScreen,public android::Thread {
    // Methods from ::vendor::mediatek::hardware::screen::V1_0::IScreen follow.
    Return<::vendor::mediatek::hardware::screen::V1_0::IapStatus> get() override;
    Return<void> init() override;
    Return<void> release() override;
    Return<void> setCallback(const sp<::vendor::mediatek::hardware::screen::V1_0::IScreenCallBack>& callback) override;
    Return<void> upgrade(const hidl_string& path, ::vendor::mediatek::hardware::screen::V1_0::UpdateType type, upgrade_cb _hidl_cb) override;
	Return<bool> beginCalibration() override;
    Return<int32_t> set(::vendor::mediatek::hardware::screen::V1_0::IapStatus val) override;
	Return<void> tpDuanluzijian(tpDuanluzijian_cb _hidl_cb) override;
    Screen();
	bool threadLoop() override;
    private:
        static void on_data_change(int signum);
        static void handle_req_update_pkg(int fd,struct req_update * pinfo);
        static void handle_IAP_status_info(int fd,struct IAP_status * pinfo);
		static void handle_update_result(int fd,struct update_result * pinfo);
		static void handle_request_package_total_num(int fd);
		static void handle_update_ack();


        int start_update(int fd,UpdateType type);
        u8 * load_file(const char *file, unsigned *sz);
        u8 * load_data_file(const char *file, unsigned *size);
        IapStatus state;
        sp<IScreenCallBack> mCallback ;
         bool mExit;
         int mFd;
	
    // Methods from ::android::hidl::base::V1_0::IBase follow.

};

// FIXME: most likely delete, this is only for passthrough implementations
// extern "C" IScreen* HIDL_FETCH_IScreen(const char* name);

}  // namespace implementation
}  // namespace V1_0
}  // namespace screen
}  // namespace hardware
}  // namespace mediatek
}  // namespace vendor

#endif  // VENDOR_MEDIATEK_HARDWARE_SCREEN_V1_0_SCREEN_H
