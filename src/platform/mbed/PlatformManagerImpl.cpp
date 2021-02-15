#include "platform/internal/CHIPDeviceLayerInternal.h"

#include <platform/PlatformManager.h>
#include <platform/internal/GenericPlatformManagerImpl.cpp>

namespace chip {
namespace DeviceLayer {

CHIP_ERROR PlatformManagerImpl::_InitChipStack(void)
{
    // Members are initialized by the stack
    // TODO: understand if they should be dynamicaly allocated or not.

    // Call up to the base class _InitChipStack() to perform the bulk of the initialization.
    auto err = GenericPlatformManagerImpl<ImplClass>::_InitChipStack();
    SuccessOrExit(err);

exit:
    return err;
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