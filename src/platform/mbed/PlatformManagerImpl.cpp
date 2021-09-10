#include <new>

#include "platform/internal/CHIPDeviceLayerInternal.h"

#include <inet/InetLayer.h>
#include <platform/CHIPDeviceLayer.h>
#include <platform/PlatformManager.h>
#include <platform/ScopedLock.h>
#include <platform/internal/GenericPlatformManagerImpl.cpp>
#include <rtos/ThisThread.h>

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

        mChipTaskId = 0;

        // Reinitialize the EventQueue
        mQueue.~EventQueue();
        new (&mQueue) events::EventQueue(event_size * CHIP_DEVICE_CONFIG_MAX_EVENT_QUEUE_SIZE);

        mQueue.background(
            [&](int t) { MbedEventTimeout::AttachTimeout([&] { SystemLayer.Signal(); }, std::chrono::milliseconds{ t }); });

        // Reinitialize the Mutexes
        mThisStateMutex.~Mutex();
        new (&mThisStateMutex) rtos::Mutex();

        mChipStackMutex.~Mutex();
        new (&mChipStackMutex) rtos::Mutex();

        // Reinitialize the condition variable
        mEvenLoopStopCond.~ConditionVariable();
        new (&mEvenLoopStopCond) rtos::ConditionVariable(mThisStateMutex);

        mShouldRunEventLoop.store(false);

        mEventLoopHasStopped = false;
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

void PlatformManagerImpl::_RunEventLoop()
{
    // Update the internal state first.
    // We may run on the current thread instead of the external task
    {
        mbed::ScopedLock<rtos::Mutex> lock(mThisStateMutex);

        // That's a programmign error to run the event loop if it is already running.
        // return early
        if (mShouldRunEventLoop.load())
        {
            ChipLogError(DeviceLayer, "Error trying to run the event loop while it is already running");
            return;
        }
        mShouldRunEventLoop.store(true);

        // Look if a task ID has already been assigned or not.
        // If not, it means we run in the thread that called RunEventLoop
        if (!mChipTaskId)
        {
            ChipLogDetail(DeviceLayer, "Run CHIP event loop on external thread");
            mChipTaskId = rtos::ThisThread::get_id();
        }

        mEventLoopHasStopped = false;
    }

    LockChipStack();

    ChipLogProgress(DeviceLayer, "CHIP Run event loop");
    SystemLayer.EventLoopBegins();
    while (true)
    {
        SystemLayer.PrepareEvents();

        UnlockChipStack();
        SystemLayer.WaitForEvents();
        LockChipStack();

        SystemLayer.HandleEvents();

        ProcessDeviceEvents();
    }
    SystemLayer.EventLoopEnds();

    UnlockChipStack();

    // Notify threads waiting on the event loop to stop
    {
        mbed::ScopedLock<rtos::Mutex> lock(mThisStateMutex);
        mEventLoopHasStopped = true;
        mEvenLoopStopCond.notify_all();
    }
}

CHIP_ERROR PlatformManagerImpl::_StartEventLoopTask()
{
    mbed::ScopedLock<rtos::Mutex> lock(mThisStateMutex);

    // This function start the Thread that run the chip event loop.
    // If no threads are needed, the application can directly call RunEventLoop.
    auto error = mLoopTask.start([this] {
        ChipLogDetail(DeviceLayer, "CHIP task running");
        RunEventLoop();
    });

    if (!error)
    {
        mChipTaskId = mLoopTask.get_id();
    }
    else
    {
        ChipLogError(DeviceLayer, "Fail to start internal loop task thread");
    }

    return TranslateOsStatus(error);
}

CHIP_ERROR PlatformManagerImpl::_StopEventLoopTask()
{
    mbed::ScopedLock<rtos::Mutex> lock(mThisStateMutex);

    // early return if the event loop is not running
    if (!mShouldRunEventLoop.load())
    {
        return CHIP_NO_ERROR;
    }

    // Indicate that the event loop store
    mShouldRunEventLoop.store(false);

    // Wake from select so it unblocks processing
    LockChipStack();
    SystemLayer.Signal();
    UnlockChipStack();

    osStatus err = osOK;

    // If the thread running the event loop is different from the caller
    // then wait it to finish
    if (mChipTaskId != rtos::ThisThread::get_id())
    {
        // First it waits for the condition variable to finish
        mEvenLoopStopCond.wait([this] { return mEventLoopHasStopped == true; });

        // Then if it was running on the internal task, wait for it to finish
        if (mChipTaskId == mLoopTask.get_id())
        {
            err          = mLoopTask.join();
            mInitialized = false; // the internal thread requires initialization again.
        }
    }

    mChipTaskId = 0;
    return TranslateOsStatus(err);
}

CHIP_ERROR PlatformManagerImpl::_StartChipTimer(int64_t durationMS)
{
    // Let SystemLayer.PrepareSelect() handle timers.
    return CHIP_NO_ERROR;
}

CHIP_ERROR PlatformManagerImpl::_Shutdown()
{
    //
    // Call up to the base class _Shutdown() to perform the actual stack de-initialization
    // and clean-up
    //
    return GenericPlatformManagerImpl<ImplClass>::_Shutdown();
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
