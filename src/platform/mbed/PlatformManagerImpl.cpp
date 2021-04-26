#include <new>

#include "platform/internal/CHIPDeviceLayerInternal.h"

#include <inet/InetLayer.h>
#include <platform/CHIPDeviceLayer.h>
#include <platform/PlatformManager.h>
#include <platform/internal/GenericPlatformManagerImpl.cpp>

#include "MbedEventTimeout.h"

#if CHIP_SYSTEM_CONFIG_USE_LWIP
#include <lwip/tcpip.h>
#endif

#define DEFAULT_MIN_SLEEP_PERIOD (60 * 60 * 24 * 30) // Month [sec]

using namespace ::chip;
using namespace ::chip::Inet;
using namespace ::chip::System;

namespace chip {
namespace DeviceLayer {

// TODO: Event and timer processing is not efficient from a memory perspective.
// Both occupy at least 24 bytes when only 4 bytes is required.
// An optimized designed could use a separate circular buffer to store events
// and register a single mbed event in the event queue to process all of them.
// A similar design can be used for timers.

CHIP_ERROR PlatformManagerImpl::_InitChipStack(void)
{
    // Members are initialized by the stack
    if (!mInitialized)
    {
        // reinitialize a new thread if it was terminated earlier
        mLoopTask.~Thread();
        new (&mLoopTask) rtos::Thread(osPriorityNormal, CHIP_DEVICE_CONFIG_CHIP_TASK_STACK_SIZE,
                                      /* memory provided */ nullptr, CHIP_DEVICE_CONFIG_CHIP_TASK_NAME);

        // Reinitialize the EventQueue
        mQueue.~EventQueue();
        new (&mQueue) events::EventQueue(event_size * CHIP_DEVICE_CONFIG_MAX_EVENT_QUEUE_SIZE);

        mQueue.background(
            [&](int t) { MbedEventTimeout::AttachTimeout([&] { SystemLayer.WakeSelect(); }, std::chrono::milliseconds{ t }); });

        // Reinitialize the Mutex
        mChipStackMutex.~Mutex();
        new (&mChipStackMutex) rtos::Mutex();

        mShouldRunEventLoop.store(true);
    }
    else
    {
        ChipLogError(DeviceLayer, "Trying to reinitialize the stack");
        return CHIP_ERROR_INCORRECT_STATE;
    }

#if CHIP_SYSTEM_CONFIG_USE_LWIP
    // Initialize LwIP.
    tcpip_init(NULL, NULL);
#endif

    // Call up to the base class _InitChipStack() to perform the bulk of the initialization.
    auto err = GenericPlatformManagerImpl<ImplClass>::_InitChipStack();
    SuccessOrExit(err);
    mInitialized = true;

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

void PlatformManagerImpl::_PostEvent(const ChipDeviceEvent * eventPtr)
{
    auto handle = mQueue.call([event = *eventPtr, this] {
        LockChipStack();
        DispatchEvent(&event);
        UnlockChipStack();
    });

    if (!handle)
    {
        ChipLogError(DeviceLayer, "Error posting event: Not enough memory");
    }
}

void PlatformManagerImpl::ProcessDeviceEvents()
{
    mQueue.dispatch(0);
}

void PlatformManagerImpl::SysUpdate()
{
    FD_ZERO(&mReadSet);
    FD_ZERO(&mWriteSet);
    FD_ZERO(&mErrorSet);
    mMaxFd = 0;

    // Max out this duration and let CHIP set it appropriately.
    mNextTimeout.tv_sec  = DEFAULT_MIN_SLEEP_PERIOD;
    mNextTimeout.tv_usec = 0;

    if (SystemLayer.State() == System::kLayerState_Initialized)
    {
        SystemLayer.PrepareSelect(mMaxFd, &mReadSet, &mWriteSet, &mErrorSet, mNextTimeout);
    }

    if (InetLayer.State == InetLayer::kState_Initialized)
    {
        InetLayer.PrepareSelect(mMaxFd, &mReadSet, &mWriteSet, &mErrorSet, mNextTimeout);
    }
}

void PlatformManagerImpl::SysProcess()
{
    UnlockChipStack();
    int selectRes = select(mMaxFd + 1, &mReadSet, &mWriteSet, &mErrorSet, &mNextTimeout);
    LockChipStack();

    if (selectRes < 0)
    {
        ChipLogError(DeviceLayer, "select failed: %s\n", ErrorStr(System::MapErrorPOSIX(errno)));
        return;
    }

    if (SystemLayer.State() == System::kLayerState_Initialized)
    {
        SystemLayer.HandleSelectResult(mMaxFd, &mReadSet, &mWriteSet, &mErrorSet);
    }

    if (InetLayer.State == InetLayer::kState_Initialized)
    {
        InetLayer.HandleSelectResult(mMaxFd, &mReadSet, &mWriteSet, &mErrorSet);
    }

    ProcessDeviceEvents();
}

void PlatformManagerImpl::_RunEventLoop()
{
    LockChipStack();

    do
    {
        SysUpdate();
        SysProcess();
    } while (mShouldRunEventLoop.load());

    UnlockChipStack();
}

CHIP_ERROR PlatformManagerImpl::_StartEventLoopTask()
{
    // This function start the Thread that run the chip event loop.
    // If no threads are needed, the application can directly call RunEventLoop.
    auto error = mLoopTask.start([this] {
        ChipLogDetail(DeviceLayer, "CHIP task running");
        RunEventLoop();
    });

    CHIP_ERROR err = TranslateOsStatus(error);
    SuccessOrExit(err);

exit:
    return err;
}

CHIP_ERROR PlatformManagerImpl::_StartChipTimer(int64_t durationMS)
{
    // Let SystemLayer.PrepareSelect() handle timers.
    return CHIP_NO_ERROR;
}

CHIP_ERROR PlatformManagerImpl::_Shutdown()
{
    LockChipStack();

    mShouldRunEventLoop.store(false);
    // If running, break out of the loop
    if (IsLoopActive())
    {
        SystemLayer.WakeSelect();
        mLoopTask.join();
    }

    // Note: we leave the event queue as is. It will be reinitialized in Init()
    mInitialized = false;
    UnlockChipStack();

    return CHIP_NO_ERROR;
}

CHIP_ERROR PlatformManagerImpl::TranslateOsStatus(osStatus error)
{
    switch (error)
    {
    case osErrorNoMemory:
        return CHIP_ERROR_NO_MEMORY;

    case osOK:
        return CHIP_NO_ERROR;

    default:
        return CHIP_ERROR_INTERNAL;
    }
}

bool PlatformManagerImpl::IsLoopActive()
{
    switch (mLoopTask.get_state())
    {
    case rtos::Thread::Inactive:
    case rtos::Thread::Ready:
    case rtos::Thread::Deleted:
        return false;

    default:
        return true;
    }
}

// ===== Members for internal use by the following friends.

PlatformManagerImpl PlatformManagerImpl::sInstance;

} // namespace DeviceLayer
} // namespace chip
