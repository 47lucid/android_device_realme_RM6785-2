#include <dlfcn.h>
#include <stdint.h>
#include <unistd.h>
#include <poll.h>

namespace android {
    struct Rect { int32_t left, top, right, bottom; };
    struct native_handle;
}

extern "C" {
    static void* get_libui() {
        static void* libui = dlopen("libui.so", RTLD_NOW);
        return libui;
    }

    __attribute__((visibility("default")))
    int _ZN7android19GraphicBufferMapper4lockEPK13native_handlejRKNS_4RectEPPvPiS9_(
            void* mapper_this, 
            const android::native_handle* handle, 
            unsigned int usage, 
            const android::Rect& bounds, 
            void** vaddr, 
            int* outBytesPerPixel, 
            int* outBytesPerStride) {
        
        static void* sym = get_libui() ? dlsym(get_libui(), "_ZN7android19GraphicBufferMapper4lockEPK13native_handleyRKNS_4RectEPPv") : nullptr;
        
        int err = -1;
        if (sym) {
            typedef int (*LockFunc)(void*, const android::native_handle*, uint64_t, const android::Rect&, void**);
            LockFunc lock_func = (LockFunc)sym;
            err = lock_func(mapper_this, handle, (uint64_t)usage, bounds, vaddr);
        }
        
        // Prevent undefined behavior by zeroing outputs for legacy callers
        if (outBytesPerPixel) *outBytesPerPixel = 0;
        if (outBytesPerStride) *outBytesPerStride = 0;
        
        return err;
    }

    __attribute__((visibility("default")))
    int _ZN7android19GraphicBufferMapper9lockYCbCrEPK13native_handlejRKNS_4RectEP13android_ycbcr(
            void* mapper_this,
            const android::native_handle* handle,
            unsigned int usage,
            const android::Rect& bounds,
            void* ycbcr) {

        // Try modern 64-bit usage first, fallback to 32-bit if necessary
        static void* sym_m = get_libui() ? dlsym(get_libui(), "_ZN7android19GraphicBufferMapper9lockYCbCrEPK13native_handleyRKNS_4RectEP13android_ycbcr") : nullptr;
        static void* sym_j = get_libui() ? dlsym(get_libui(), "_ZN7android19GraphicBufferMapper9lockYCbCrEPK13native_handlejRKNS_4RectEP13android_ycbcr") : nullptr;
        
        void* sym = sym_m ? sym_m : sym_j;
        int err = -1;
        if (sym) {
            if (sym == sym_m) {
                typedef int (*LockYCbCrFunc64)(void*, const android::native_handle*, uint64_t, const android::Rect&, void*);
                LockYCbCrFunc64 lock_func = (LockYCbCrFunc64)sym;
                err = lock_func(mapper_this, handle, (uint64_t)usage, bounds, ycbcr);
            } else {
                typedef int (*LockYCbCrFunc32)(void*, const android::native_handle*, unsigned int, const android::Rect&, void*);
                LockYCbCrFunc32 lock_func = (LockYCbCrFunc32)sym;
                err = lock_func(mapper_this, handle, usage, bounds, ycbcr);
            }
        }
        return err;
    }

    __attribute__((visibility("default")))
    int _ZN7android19GraphicBufferMapper6unlockEPK13native_handle(
            void* mapper_this,
            const android::native_handle* handle) {
        
        static void* sym = get_libui() ? dlsym(get_libui(), "_ZN7android19GraphicBufferMapper6unlockEPK13native_handlePNS_4base14unique_fd_implINS4_13DefaultCloserEEE") : nullptr;
        
        if (sym) {
            typedef int (*UnlockFunc)(void*, const android::native_handle*, int*);
            UnlockFunc unlock_func = (UnlockFunc)sym;
            int fenceFd = -1;
            int err = unlock_func(mapper_this, handle, &fenceFd);
            if (fenceFd >= 0) {
                struct pollfd pfd = {
                    .fd = fenceFd,
                    .events = POLLIN,
                    .revents = 0
                };
                poll(&pfd, 1, -1);
                close(fenceFd);
            }
            return err;
        }
        
        return -1;
    }
}
