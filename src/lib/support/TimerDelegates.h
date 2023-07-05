#include <app/InteractionModelEngine.h>
#include <app/reporting/ReportScheduler.h>
#include <system/SystemClock.h>

using ReportScheduler        = chip::app::reporting::ReportScheduler;
using ReadHandlerNode        = chip::app::reporting::ReportScheduler::ReadHandlerNode;
using InteractionModelEngine = chip::app::InteractionModelEngine;
using Timestamp              = chip::System::Clock::Timestamp;
using Timeout                = chip::System::Clock::Timeout;

namespace chip {

class DefaultTimerDelegate : public ReportScheduler::TimerDelegate
{
public:
    static void TimerCallbackInterface(System::Layer * aLayer, void * aAppState)
    {
        ReadHandlerNode * node = static_cast<ReadHandlerNode *>(aAppState);
        node->RunCallback();
    }
    CHIP_ERROR StartTimer(void * context, Timeout aTimeout) override
    {
        return InteractionModelEngine::GetInstance()->GetExchangeManager()->GetSessionManager()->SystemLayer()->StartTimer(
            aTimeout, TimerCallbackInterface, context);
    }
    void CancelTimer(void * context) override
    {
        InteractionModelEngine::GetInstance()->GetExchangeManager()->GetSessionManager()->SystemLayer()->CancelTimer(
            TimerCallbackInterface, context);
    }
    bool IsTimerActive(void * context) override
    {
        return InteractionModelEngine::GetInstance()->GetExchangeManager()->GetSessionManager()->SystemLayer()->IsTimerActive(
            TimerCallbackInterface, context);
    }

    Timestamp GetCurrentMonotonicTimestamp() override { return System::SystemClock().GetMonotonicTimestamp(); }
};

class SynchronizedTimerDelegate : public DefaultTimerDelegate
{
public:
    static void TimerCallbackInterface(System::Layer * aLayer, void * aAppState)
    {
        ReportScheduler * scheduler = static_cast<ReportScheduler *>(aAppState);
        scheduler->ReportTimerCallback();
    }
};

} // namespace chip
