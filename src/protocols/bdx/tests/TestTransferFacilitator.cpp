#include <string.h>

#include <pw_unit_test/framework.h>

#include <functional>
#include <optional>
#include <protocols/bdx/TransferFacilitator.h>
#include <system/SystemClock.h>
#include <system/SystemTimer.h>

using namespace ::chip;
using namespace ::chip::bdx;
using namespace ::chip::Protocols;
using namespace ::chip::System::Clock::Literals;

using TransferSessionOutputHandler = std::function<void(TransferSession::OutputEvent & event)>;

namespace chip {
namespace System {

using StartTimerHook = std::function<void(Clock::Timeout aDelay, TimerCompleteCallback aComplete, void * aAppState)>;

class SystemLayerWithMockClock : public Clock::Internal::MockClock, public Layer
{
public:
    // System Layer overrides
    CriticalFailure Init() override { return CHIP_NO_ERROR; }
    void Shutdown() override { Clear(); }
    void Clear()
    {
        mTimerList.Clear();
        mTimerNodes.ReleaseAll();
    }
    bool IsInitialized() const override { return true; }

    CriticalFailure StartTimer(Clock::Timeout aDelay, TimerCompleteCallback aComplete, void * aAppState) override
    {
        Clock::Timestamp awakenTime = GetMonotonicMilliseconds64() + std::chrono::duration_cast<Clock::Milliseconds64>(aDelay);
        TimerList::Node * node      = mTimerNodes.Create(*this, awakenTime, aComplete, aAppState);
        mTimerList.Add(node);

        if (mStartTimerHook.has_value())
        {
            mStartTimerHook.value()(aDelay, aComplete, aAppState);
        }

        return CHIP_NO_ERROR;
    }
    void CancelTimer(TimerCompleteCallback aComplete, void * aAppState) override
    {
        TimerList::Node * cancelled = mTimerList.Remove(aComplete, aAppState);
        if (cancelled != nullptr)
        {
            mTimerNodes.Release(cancelled);
        }
    }
    CHIP_ERROR ExtendTimerTo(Clock::Timeout aDelay, TimerCompleteCallback aComplete, void * aAppState) override
    {
        return CHIP_ERROR_NOT_IMPLEMENTED;
    }
    bool IsTimerActive(TimerCompleteCallback onComplete, void * appState) override
    {
        return mTimerList.GetRemainingTime(onComplete, appState) != Clock::Timeout(0);
    }
    Clock::Timeout GetRemainingTime(TimerCompleteCallback onComplete, void * appState) override
    {
        return mTimerList.GetRemainingTime(onComplete, appState);
    }
    CriticalFailure ScheduleWork(TimerCompleteCallback aComplete, void * aAppState) override { return CHIP_ERROR_NOT_IMPLEMENTED; }

    // Clock overrides
    void SetMonotonic(Clock::Milliseconds64 timestamp)
    {
        MockClock::SetMonotonic(timestamp);
        // Find all the timers that fired at this time or before and invoke the callbacks
        TimerList::Node * node;
        while ((node = mTimerList.Earliest()) != nullptr && node->AwakenTime() <= timestamp)
        {
            mTimerList.PopEarliest();
            // Invoke auto-releases
            mTimerNodes.Invoke(node);
        }
    }

    void AdvanceMonotonic(Clock::Milliseconds64 increment) { SetMonotonic(GetMonotonicMilliseconds64() + increment); }

    std::optional<StartTimerHook> mStartTimerHook{ std::nullopt };

private:
    TimerPool<> mTimerNodes;
    TimerList mTimerList;
};

} // namespace System
} // namespace chip

// These are globals because SetUpTestSuite is static which requires static variables
System::SystemLayerWithMockClock gSystemLayerAndClock = System::SystemLayerWithMockClock();
System::Clock::ClockBase * gSavedClock                = nullptr;

class TestTransferFacilitator : public ::testing::Test
{
public:
    static void SetUpTestSuite()
    {
        EXPECT_EQ(Platform::MemoryInit(), CHIP_NO_ERROR);
        gSavedClock = &System::SystemClock();
        System::Clock::Internal::SetSystemClockForTesting(&gSystemLayerAndClock);
    }

    static void TearDownTestSuite()
    {
        gSystemLayerAndClock.Shutdown();
        System::Clock::Internal::SetSystemClockForTesting(gSavedClock);
        Platform::MemoryShutdown();
    }
};

class TestInitiator : public Initiator
{

private:
    void HandleTransferSessionOutput(TransferSession::OutputEvent & event) override
    {
        if (mTransferSessionOutputHandler.has_value())
        {
            mTransferSessionOutputHandler.value()(event);
        }
    }

public:
    void PollForOutput() { Initiator::PollForOutput(); }

    void ScheduleImmediatePoll() { Initiator::ScheduleImmediatePoll(); }

    std::optional<TransferSessionOutputHandler> mTransferSessionOutputHandler{ std::nullopt };
};

TEST_F(TestTransferFacilitator, InitiatesTransfer)
{
    TestInitiator initiator;

    auto r = initiator.InitiateTransfer(&gSystemLayerAndClock, TransferRole::kSender, TransferSession::TransferInitData(),
                                        System::Clock::Seconds16(60), System::Clock::Milliseconds32(500));

    EXPECT_EQ(r, CHIP_NO_ERROR); // Placeholder for actual test logic
}

TEST_F(TestTransferFacilitator, PollsForOutputAfterTimeoutExpires)
{
    TestInitiator initiator;

    auto initData = TransferSession::TransferInitData();

    initData.TransferCtlFlags = TransferControlFlags::kSenderDrive;
    initData.MaxBlockSize     = 1024;
    initData.StartOffset      = 0;
    initData.Length           = 10000; // 10 KB transfer
    initData.FileDesignator   = reinterpret_cast<const uint8_t *>("test_file.txt");
    initData.FileDesLength    = static_cast<uint16_t>(strlen(reinterpret_cast<const char *>(initData.FileDesignator)));
    initData.Metadata         = nullptr; // No metadata for this test
    initData.MetadataLength   = 0;

    // hook a callback to the output handler
    auto outputEvent                        = TransferSession::OutputEvent(TransferSession::OutputEventType::kNone);
    initiator.mTransferSessionOutputHandler = [&outputEvent](TransferSession::OutputEvent & event) {
        outputEvent.EventType = event.EventType; // Capture the output event type
    };

    gSystemLayerAndClock.SetMonotonic(0_ms);

    auto r = initiator.InitiateTransfer(&gSystemLayerAndClock, TransferRole::kSender, initData, System::Clock::Seconds16(10),
                                        System::Clock::Milliseconds32(2000));

    // advancing clock to 5s expires poll frequency timer and triggers a call to PollForOutput
    // which sets initial time for starting to count timeout set in InitiateTransfer
    gSystemLayerAndClock.SetMonotonic(5000_ms); // Set the clock to 5 seconds

    // advancing clock to 15s expires the timeout set in InitiateTransfer since 15000_ms - 5000_ms = 10000_ms
    // which triggers a call to PollForOutput that returns a TransferTimeout event type
    gSystemLayerAndClock.SetMonotonic(15000_ms);

    EXPECT_EQ(r, CHIP_NO_ERROR);

    EXPECT_EQ(outputEvent.EventType,
              TransferSession::OutputEventType::kTransferTimeout); // Check if the output event is a timeout event
}

TEST_F(TestTransferFacilitator, PollsForOutput)
{
    TestInitiator initiator;

    // hook a callback to the StartTimerHook
    bool timerStarted                    = false;
    gSystemLayerAndClock.mStartTimerHook = [&timerStarted](auto, auto, void *) { timerStarted = true; };

    auto r = initiator.InitiateTransfer(&gSystemLayerAndClock, TransferRole::kSender, TransferSession::TransferInitData(),
                                        System::Clock::Seconds16(60), System::Clock::Milliseconds32(500));

    EXPECT_EQ(r, CHIP_NO_ERROR); // Placeholder for actual test logic

    // hook a callback to the output handler
    bool outputHandled                      = false;
    initiator.mTransferSessionOutputHandler = [&outputHandled](TransferSession::OutputEvent & event) { outputHandled = true; };

    // Simulate a poll
    initiator.PollForOutput();

    // Check if the poll was handled correctly
    EXPECT_TRUE(outputHandled);

    // Check if the timer was started
    EXPECT_TRUE(timerStarted);
}
