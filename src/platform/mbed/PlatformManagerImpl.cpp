#include <new>

#include "platform/internal/CHIPDeviceLayerInternal.h"

#include <platform/PlatformManager.h>
#include <platform/internal/GenericPlatformManagerImpl.cpp>

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

        // Reinitialize the Mutex
        mChipStackMutex.~Mutex();
        new (&mChipStackMutex) rtos::Mutex();
    }
    else
    {
        ChipLogError(DeviceLayer, "Trying to reinitialize the stack");
        return CHIP_ERROR_INCORRECT_STATE;
    }

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

void PlatformManagerImpl::_PostEvent(const ChipDeviceEvent * event)
{
    auto handle = mQueue.call([event, this] {
        LockChipStack();
        DispatchEvent(event);
        UnlockChipStack();
    });

    if (!handle)
    {
        ChipLogError(DeviceLayer, "Error posting event: Not enough memory");
    }
}

void PlatformManagerImpl::_RunEventLoop()
{
    // Note: no reason to lock the chip task here, it is locked by the
    // event and timer callback
    mQueue.dispatch_forever();
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
    CHIP_ERROR err = CHIP_NO_ERROR;
    int event      = 0;

    VerifyOrExit(mInitialized == true, err = CHIP_ERROR_INCORRECT_STATE);

    event = mQueue.call_in(std::chrono::milliseconds(durationMS), [this] {
        LockChipStack();
        auto err = SystemLayer.HandlePlatformTimer();
        if (err != CHIP_SYSTEM_NO_ERROR)
        {
            ChipLogError(DeviceLayer, "Error handling CHIP timers: %s", ErrorStr(err));
        }
        UnlockChipStack();
    });

    VerifyOrExit(event != 0, err = CHIP_ERROR_NO_MEMORY);

exit:
    return err;
}

CHIP_ERROR PlatformManagerImpl::_Shutdown()
{
    LockChipStack();

    // If running, break out of the loop
    if (IsLoopActive())
    {
        mQueue.break_dispatch();
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