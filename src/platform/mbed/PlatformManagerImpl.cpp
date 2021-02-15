#include "platform/internal/CHIPDeviceLayerInternal.h"

#include <platform/PlatformManager.h>
#include <platform/internal/GenericPlatformManagerImpl.cpp>

namespace chip {
namespace DeviceLayer {

CHIP_ERROR PlatformManagerImpl::_InitChipStack(void)
{
    return CHIP_ERROR_NOT_IMPLEMENTED;
}

void PlatformManagerImpl::_LockChipStack()
{
    mChipStackMutex.lock();
}

bool PlatformManagerImpl::_TryLockChipStack()
{
    return mChipStackMutex.trylock();
}
void PlatformManagerImpl::_UnlockChipStack()
{
    mChipStackMutex.unlock();
}

void PlatformManagerImpl::_PostEvent(const ChipDeviceEvent * event) {}

void PlatformManagerImpl::_RunEventLoop() {}

CHIP_ERROR PlatformManagerImpl::_StartEventLoopTask()
{
    return CHIP_ERROR_NOT_IMPLEMENTED;
}

CHIP_ERROR PlatformManagerImpl::_StartChipTimer(int64_t durationMS)
{
    return CHIP_ERROR_NOT_IMPLEMENTED;
}

CHIP_ERROR PlatformManagerImpl::_Shutdown()
{
    return CHIP_ERROR_NOT_IMPLEMENTED;
}

// ===== Members for internal use by the following friends.

PlatformManagerImpl PlatformManagerImpl::sInstance;

} // namespace DeviceLayer
} // namespace chip