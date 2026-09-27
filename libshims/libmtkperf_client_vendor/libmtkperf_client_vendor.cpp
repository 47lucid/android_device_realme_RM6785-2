#define LOG_TAG "mtkperf_client_vendor"

#include <android/binder_manager.h>
#include <aidl/android/hardware/power/IPower.h>
#include <aidl/android/hardware/power/Mode.h>
#include <log/log.h>
#include <mutex>

using aidl::android::hardware::power::IPower;
using aidl::android::hardware::power::Mode;

static std::shared_ptr<IPower> gPowerHal = nullptr;
static std::mutex gLock;
static int gHandleCounter = 0;

static std::shared_ptr<IPower> getPowerHal() {
    std::lock_guard<std::mutex> lock(gLock);
    if (gPowerHal) return gPowerHal;
    
    auto name = std::string(IPower::descriptor) + "/default";
    if (AServiceManager_isDeclared(name.c_str())) {
        ndk::SpAIBinder binder(AServiceManager_waitForService(name.c_str()));
        gPowerHal = IPower::fromBinder(binder);
    }
    
    if (!gPowerHal) {
        ALOGE("Failed to get IPower HAL");
    } else {
        ALOGI("Successfully connected to IPower HAL");
    }
    return gPowerHal;
}

static void setBoost(bool enable) {
    auto hal = getPowerHal();
    if (!hal) return;
    
    // We map the video perf boost to EXPENSIVE_RENDERING since it provides a solid boost.
    auto status = hal->setMode(Mode::EXPENSIVE_RENDERING, enable);
    if (!status.isOk()) {
        ALOGE("Failed to set EXPENSIVE_RENDERING mode: %s", status.getDescription().c_str());
        // Clear the HAL handle if dead so we reconnect next time
        if (status.getExceptionCode() == EX_TRANSACTION_FAILED) {
            std::lock_guard<std::mutex> lock(gLock);
            gPowerHal = nullptr;
        }
    } else {
        ALOGI("EXPENSIVE_RENDERING mode set to %d", enable);
    }
}

extern "C" {

__attribute__((visibility("default")))
int perf_lock_acq(int handle, int duration, int list[], int numArgs) {
    (void)duration;
    (void)list;
    (void)numArgs;
    
    if (handle > 0) return handle; // Extending existing lock

    setBoost(true);
    
    gHandleCounter++;
    return gHandleCounter > 0 ? gHandleCounter : 1;
}

__attribute__((visibility("default")))
int perf_lock_rel(int handle) {
    (void)handle;
    setBoost(false);
    return 0;
}

__attribute__((visibility("default")))
int perf_lock_rel_async(int handle) {
    return perf_lock_rel(handle);
}

__attribute__((visibility("default")))
int perf_cus_lock_hint(int hint, int duration, int list[], int numArgs) {
    return perf_lock_acq(0, duration, list, numArgs);
}

} // extern "C"
