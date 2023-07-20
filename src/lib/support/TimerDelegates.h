#include <app/InteractionModelEngine.h>
#include <app/reporting/ReportScheduler.h>
#include <system/SystemClock.h>

namespace chip {

class DefaultTimerDelegate : public app::reporting::ReportScheduler::TimerDelegate
{
public:
    using ReadHandlerNode        = app::reporting::ReportScheduler::ReadHandlerNode;
    using InteractionModelEngine = app::InteractionModelEngine;
    using Timeout                = System::Clock::Timeout;
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

    System::Clock::Timestamp GetCurrentMonotonicTimestamp() override { return System::SystemClock().GetMonotonicTimestamp(); }
};

class SynchronizedTimerDelegate : public DefaultTimerDelegate
{
public:
    static void TimerCallbackInterface(System::Layer * aLayer, void * aAppState)
    {
        app::reporting::ReportScheduler * scheduler = static_cast<app::reporting::ReportScheduler *>(aAppState);
        scheduler->ReportTimerCallback();
    }
};

} // namespace chip
