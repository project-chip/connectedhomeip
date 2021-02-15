#include <platform/internal/CHIPDeviceLayerInternal.h>

#include <platform/PlatformManager.h>
#include <platform/internal/GenericPlatformManagerImpl.cpp>

namespace chip {
namespace DeviceLayer {

CHIP_ERROR PlatformManagerImpl::_InitChipStack(void)
{
    return 0;
}

void PlatformManagerImpl::_LockChipStack() {}

bool PlatformManagerImpl::_TryLockChipStack()
{
    return true;
}
void PlatformManagerImpl::_UnlockChipStack() {}

void PlatformManagerImpl::_PostEvent(const ChipDeviceEvent * event) {}

void PlatformManagerImpl::_RunEventLoop() {}

CHIP_ERROR PlatformManagerImpl::_StartEventLoopTask()
{
    return 0;
}

CHIP_ERROR PlatformManagerImpl::_StartChipTimer(int64_t durationMS)
{
    return 0;
}

CHIP_ERROR PlatformManagerImpl::_Shutdown()
{
    return 0;
}

// ===== Members for internal use by the following friends.

PlatformManagerImpl PlatformManagerImpl::sInstance;

} // namespace DeviceLayer
} // namespace chip