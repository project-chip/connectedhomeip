/*
 *    Copyright (c) 2020-2021 Project CHIP Authors
 *    All rights reserved.
 *
 *    Licensed under the Apache License, Version 2.0 (the "License");
 *    you may not use this file except in compliance with the License.
 *    You may obtain a copy of the License at
 *
 *        http://www.apache.org/licenses/LICENSE-2.0
 *
 *    Unless required by applicable law or agreed to in writing, software
 *    distributed under the License is distributed on an "AS IS" BASIS,
 *    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *    See the License for the specific language governing permissions and
 *    limitations under the License.
 */

/**
 *    @file
 *      This file implements the CHIP reliable message protocol.
 *
 */

#include <inttypes.h>

#include <messaging/ReliableMessageMgr.h>

#include <lib/support/BitFlags.h>
#include <lib/support/CHIPFaultInjection.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/logging/CHIPLogging.h>
#include <messaging/ErrorCategory.h>
#include <messaging/ExchangeMessageDispatch.h>
#include <messaging/Flags.h>
#include <messaging/ReliableMessageContext.h>

namespace chip {
namespace Messaging {

ReliableMessageMgr::RetransTableEntry::RetransTableEntry(ReliableMessageContext * rc) :
    ec(*rc->GetExchangeContext()), retainedBuf(EncryptedPacketBufferHandle()), nextRetransTimeTick(0), sendCount(0)
{
    ec->SetMessageNotAcked(true);
}

ReliableMessageMgr::RetransTableEntry::~RetransTableEntry()
{
    ec->SetMessageNotAcked(false);
}

ReliableMessageMgr::ReliableMessageMgr(BitMapObjectPool<ExchangeContext, CHIP_CONFIG_MAX_EXCHANGE_CONTEXTS> & contextPool) :
    mContextPool(contextPool), mSystemLayer(nullptr), mCurrentTimerExpiry(0),
    mTimerIntervalShift(CHIP_CONFIG_RMP_TIMER_DEFAULT_PERIOD_SHIFT)
{}

ReliableMessageMgr::~ReliableMessageMgr() {}

void ReliableMessageMgr::Init(chip::System::Layer * systemLayer, SessionManager * sessionManager)
{
    mSystemLayer        = systemLayer;
    mTimeStampBase      = System::SystemClock().GetMonotonicMilliseconds();
    mCurrentTimerExpiry = 0;
}

void ReliableMessageMgr::Shutdown()
{
    StopTimer();

    // Clear the retransmit table
    mRetransTable.ForEachActiveObject([&](auto * entry) {
        ClearRetransTable(*entry);
        return true;
    });

    mSystemLayer = nullptr;
}

uint64_t ReliableMessageMgr::GetTickCounterFromTimePeriod(uint64_t period)
{
    return (period >> mTimerIntervalShift);
}

uint64_t ReliableMessageMgr::GetTickCounterFromTimeDelta(uint64_t newTime)
{
    return GetTickCounterFromTimePeriod(newTime - mTimeStampBase);
}

#if defined(RMP_TICKLESS_DEBUG)
void ReliableMessageMgr::TicklessDebugDumpRetransTable(const char * log)
{
    ChipLogDetail(ExchangeManager, log);

    mRetransTable.ForEachActiveObject([&](auto * entry) {
        ChipLogDetail(ExchangeManager,
                      "EC:" ChipLogFormatExchange " MessageCounter:" ChipLogFormatMessageCounter " NextRetransTimeCtr:%04" PRIX16,
                      ChipLogValueExchange(&entry->ec.Get()), entry->retainedBuf.GetMessageCounter(), entry->nextRetransTimeTick);
        return true;
    });
}
#else
void ReliableMessageMgr::TicklessDebugDumpRetransTable(const char * log)
{
    return;
}
#endif // RMP_TICKLESS_DEBUG

void ReliableMessageMgr::ExecuteActions()
{
#if defined(RMP_TICKLESS_DEBUG)
    ChipLogDetail(ExchangeManager, "ReliableMessageMgr::ExecuteActions");
#endif

    ExecuteForAllContext([](ReliableMessageContext * rc) {
        if (rc->IsAckPending())
        {
            if (0 == rc->mNextAckTimeTick)
            {
#if defined(RMP_TICKLESS_DEBUG)
                ChipLogDetail(ExchangeManager, "ReliableMessageMgr::ExecuteActions sending ACK");
#endif
                // Send the Ack in a SecureChannel::StandaloneAck message
                rc->SendStandaloneAckMessage();
            }
        }
    });

    TicklessDebugDumpRetransTable("ReliableMessageMgr::ExecuteActions Dumping mRetransTable entries before processing");

    // Retransmit / cancel anything in the retrans table whose retrans timeout
    // has expired
    mRetransTable.ForEachActiveObject([&](auto * entry) {
        CHIP_ERROR err = CHIP_NO_ERROR;

        if (entry->nextRetransTimeTick != 0)
            return true;

        if (entry->retainedBuf.IsNull())
        {
            // We generally try to prevent entries with a null buffer being in a table, but it could happen
            // if the message dispatch (which is supposed to fill in the buffer) fails to do so _and_ returns
            // success (so its caller doesn't clear out the bogus table entry).
            //
            // If that were to happen, we would crash in the code below.  Guard against it, just in case.
            ClearRetransTable(*entry);
            return true;
        }

        uint8_t sendCount       = entry->sendCount;
        uint32_t messageCounter = entry->retainedBuf.GetMessageCounter();

        if (sendCount == CHIP_CONFIG_RMP_DEFAULT_MAX_RETRANS)
        {
            err = CHIP_ERROR_MESSAGE_NOT_ACKNOWLEDGED;

            ChipLogError(ExchangeManager,
                         "Failed to Send CHIP MessageCounter:" ChipLogFormatMessageCounter " on exchange " ChipLogFormatExchange
                         " sendCount: %" PRIu8 " max retries: %d",
                         messageCounter, ChipLogValueExchange(&entry->ec.Get()), sendCount, CHIP_CONFIG_RMP_DEFAULT_MAX_RETRANS);

            // Remove from Table
            ClearRetransTable(*entry);
        }

        // Resend from Table (if the operation fails, the entry is cleared)
        if (err == CHIP_NO_ERROR)
            err = SendFromRetransTable(entry);

        if (err == CHIP_NO_ERROR)
        {
            // If the retransmission was successful, update the passive timer
            entry->nextRetransTimeTick = static_cast<uint16_t>(entry->ec->GetActiveRetransmitTimeoutTick());
#if !defined(NDEBUG)
            ChipLogDetail(ExchangeManager,
                          "Retransmitted MessageCounter:" ChipLogFormatMessageCounter " on exchange " ChipLogFormatExchange
                          " Send Cnt %d",
                          messageCounter, ChipLogValueExchange(&entry->ec.Get()), entry->sendCount);
#endif
        }

        return true;
    });

    TicklessDebugDumpRetransTable("ReliableMessageMgr::ExecuteActions Dumping mRetransTable entries after processing");
}

static void TickProceed(uint16_t & time, uint64_t ticks)
{
    if (time >= ticks)
    {
        time = static_cast<uint16_t>(time - ticks);
    }
    else
    {
        time = 0;
    }
}

void ReliableMessageMgr::ExpireTicks()
{
    uint64_t now = System::SystemClock().GetMonotonicMilliseconds();

    // Number of full ticks elapsed since last timer processing.  We always round down
    // to the previous tick.  If we are between tick boundaries, the extra time since the
    // last virtual tick is not accounted for here (it will be accounted for when resetting
    // the ReliableMessageProtocol timer)
    uint64_t deltaTicks = GetTickCounterFromTimeDelta(now);

#if defined(RMP_TICKLESS_DEBUG)
    ChipLogDetail(ExchangeManager, "ReliableMessageMgr::ExpireTicks at %" PRIu64 ", %" PRIu64 ", %" PRIu64, now, mTimeStampBase,
                  deltaTicks);
#endif

    ExecuteForAllContext([deltaTicks](ReliableMessageContext * rc) {
        if (rc->IsAckPending())
        {
            // Decrement counter of Ack timestamp by the elapsed timer ticks
            TickProceed(rc->mNextAckTimeTick, deltaTicks);
#if defined(RMP_TICKLESS_DEBUG)
            ChipLogDetail(ExchangeManager, "ReliableMessageMgr::ExpireTicks set mNextAckTimeTick to %u", rc->mNextAckTimeTick);
#endif
        }
    });

    mRetransTable.ForEachActiveObject([&](auto * entry) {
        // Decrement Retransmit timeout by elapsed timeticks
        TickProceed(entry->nextRetransTimeTick, deltaTicks);
#if defined(RMP_TICKLESS_DEBUG)
        ChipLogDetail(ExchangeManager, "ReliableMessageMgr::ExpireTicks set nextRetransTimeTick to %u", entry->nextRetransTimeTick);
#endif
        return true;
    });

    // Re-Adjust the base time stamp to the most recent tick boundary
    mTimeStampBase += (deltaTicks << mTimerIntervalShift);

#if defined(RMP_TICKLESS_DEBUG)
    ChipLogDetail(ExchangeManager, "ReliableMessageMgr::ExpireTicks mTimeStampBase to %" PRIu64, mTimeStampBase);
#endif
}

void ReliableMessageMgr::Timeout(System::Layer * aSystemLayer, void * aAppState)
{
    ReliableMessageMgr * manager = reinterpret_cast<ReliableMessageMgr *>(aAppState);

    VerifyOrDie((aSystemLayer != nullptr) && (manager != nullptr));

#if defined(RMP_TICKLESS_DEBUG)
    ChipLogDetail(ExchangeManager, "ReliableMessageMgr::Timeout\n");
#endif

    // Make sure all tick counts are sync'd to the current time
    manager->ExpireTicks();

    // Execute any actions that are due this tick
    manager->ExecuteActions();

    // Calculate next physical wakeup
    manager->StartTimer();
}

CHIP_ERROR ReliableMessageMgr::AddToRetransTable(ReliableMessageContext * rc, RetransTableEntry ** rEntry)
{
    VerifyOrDie(!rc->IsMessageNotAcked());

    // Expire any virtual ticks that have expired so all wakeup sources reflect the current time
    ExpireTicks();

    *rEntry = mRetransTable.CreateObject(rc);

    if (*rEntry == nullptr)
    {
        ChipLogError(ExchangeManager, "mRetransTable Already Full");
        return CHIP_ERROR_RETRANS_TABLE_FULL;
    }

    return CHIP_NO_ERROR;
}

void ReliableMessageMgr::StartRetransmision(RetransTableEntry * entry)
{
    entry->nextRetransTimeTick =
        static_cast<uint16_t>(entry->ec->GetInitialRetransmitTimeoutTick() +
                              GetTickCounterFromTimeDelta(System::SystemClock().GetMonotonicMilliseconds()));

    // Check if the timer needs to be started and start it.
    StartTimer();
}

void ReliableMessageMgr::PauseRetransmision(ReliableMessageContext * rc, uint32_t PauseTimeMillis)
{
    mRetransTable.ForEachActiveObject([&](auto * entry) {
        if (entry->ec->GetReliableMessageContext() == rc)
        {
            entry->nextRetransTimeTick =
                static_cast<uint16_t>(entry->nextRetransTimeTick + (PauseTimeMillis >> mTimerIntervalShift));
            return false;
        }
        return true;
    });
}

void ReliableMessageMgr::ResumeRetransmision(ReliableMessageContext * rc)
{
    mRetransTable.ForEachActiveObject([&](auto * entry) {
        if (entry->ec->GetReliableMessageContext() == rc)
        {
            entry->nextRetransTimeTick = 0;
            return false;
        }
        return true;
    });
}

bool ReliableMessageMgr::CheckAndRemRetransTable(ReliableMessageContext * rc, uint32_t ackMessageCounter)
{
    bool removed = false;
    mRetransTable.ForEachActiveObject([&](auto * entry) {
        if (entry->ec->GetReliableMessageContext() == rc && entry->retainedBuf.GetMessageCounter() == ackMessageCounter)
        {
            // Clear the entry from the retransmision table.
            ClearRetransTable(*entry);

#if !defined(NDEBUG)
            ChipLogDetail(ExchangeManager,
                          "Rxd Ack; Removing MessageCounter:" ChipLogFormatMessageCounter
                          " from Retrans Table on exchange " ChipLogFormatExchange,
                          ackMessageCounter, ChipLogValueExchange(rc->GetExchangeContext()));
#endif
            removed = true;
            return false;
        }
        return true;
    });

    return removed;
}

CHIP_ERROR ReliableMessageMgr::SendFromRetransTable(RetransTableEntry * entry)
{
    const ExchangeMessageDispatch * dispatcher = entry->ec->GetMessageDispatch();
    if (dispatcher == nullptr || !entry->ec->HasSecureSession())
    {
        // Using same error message for all errors to reduce code size.
        ChipLogError(ExchangeManager,
                     "Crit-err %" CHIP_ERROR_FORMAT " when sending CHIP MessageCounter:" ChipLogFormatMessageCounter
                     " on exchange " ChipLogFormatExchange ", send tries: %d",
                     CHIP_ERROR_INCORRECT_STATE.Format(), entry->retainedBuf.GetMessageCounter(),
                     ChipLogValueExchange(&entry->ec.Get()), entry->sendCount);
        ClearRetransTable(*entry);
        return CHIP_ERROR_INCORRECT_STATE;
    }

    CHIP_ERROR err = dispatcher->SendPreparedMessage(entry->ec->GetSecureSession(), entry->retainedBuf);

    if (err == CHIP_NO_ERROR)
    {
        // Update the counters
        entry->sendCount++;
    }
    else
    {
        // Remove from table
        // Using same error message for all errors to reduce code size.
        ChipLogError(ExchangeManager,
                     "Crit-err %" CHIP_ERROR_FORMAT " when sending CHIP MessageCounter:" ChipLogFormatMessageCounter
                     " on exchange " ChipLogFormatExchange ", send tries: %d",
                     err.Format(), entry->retainedBuf.GetMessageCounter(), ChipLogValueExchange(&entry->ec.Get()),
                     entry->sendCount);

        ClearRetransTable(*entry);
    }
    return err;
}

void ReliableMessageMgr::ClearRetransTable(ReliableMessageContext * rc)
{
    RetransTableEntry * result = nullptr;
    mRetransTable.ForEachActiveObject([&](auto * entry) {
        if (entry->ec->GetReliableMessageContext() == rc)
        {
            result = entry;
            return false;
        }
        return true;
    });
    if (result != nullptr)
    {
        ClearRetransTable(*result);
    }
}

void ReliableMessageMgr::ClearRetransTable(RetransTableEntry & entry)
{
    mRetransTable.ReleaseObject(&entry);
    // Expire any virtual ticks that have expired so all wakeup sources reflect the current time
    ExpireTicks();
    StartTimer();
}

void ReliableMessageMgr::FailRetransTableEntries(ReliableMessageContext * rc, CHIP_ERROR err)
{
    ClearRetransTable(rc);
}

void ReliableMessageMgr::StartTimer()
{
    CHIP_ERROR res            = CHIP_NO_ERROR;
    uint64_t nextWakeTimeTick = UINT64_MAX;
    bool foundWake            = false;

    // When do we need to next wake up to send an ACK?

    ExecuteForAllContext([&nextWakeTimeTick, &foundWake](ReliableMessageContext * rc) {
        if (rc->IsAckPending() && rc->mNextAckTimeTick < nextWakeTimeTick)
        {
            nextWakeTimeTick = rc->mNextAckTimeTick;
            foundWake        = true;
#if defined(RMP_TICKLESS_DEBUG)
            ChipLogDetail(ExchangeManager, "ReliableMessageMgr::StartTimer next ACK time %" PRIu64, nextWakeTimeTick);
#endif
        }
    });

    mRetransTable.ForEachActiveObject([&](auto * entry) {
        // When do we need to next wake up for ReliableMessageProtocol retransmit?
        if (entry->nextRetransTimeTick < nextWakeTimeTick)
        {
            nextWakeTimeTick = entry->nextRetransTimeTick;
            foundWake        = true;
#if defined(RMP_TICKLESS_DEBUG)
            ChipLogDetail(ExchangeManager, "ReliableMessageMgr::StartTimer RetransTime %" PRIu64, nextWakeTimeTick);
#endif
        }
        return true;
    });

    if (foundWake)
    {
        // Set timer for next tick boundary - subtract the elapsed time from the current tick
        System::Clock::MonotonicMilliseconds timerExpiry = (nextWakeTimeTick << mTimerIntervalShift) + mTimeStampBase;

#if defined(RMP_TICKLESS_DEBUG)
        ChipLogDetail(ExchangeManager, "ReliableMessageMgr::StartTimer wake at %" PRIu64 " ms (%" PRIu64 " %" PRIu64 ")",
                      timerExpiry, nextWakeTimeTick, mTimeStampBase);
#endif
        if (timerExpiry != mCurrentTimerExpiry)
        {
            // If the tick boundary has expired in the past (delayed processing of event due to other system activity),
            // expire the timer immediately
            uint64_t now           = System::SystemClock().GetMonotonicMilliseconds();
            uint64_t timerArmValue = (timerExpiry > now) ? timerExpiry - now : 0;

#if defined(RMP_TICKLESS_DEBUG)
            ChipLogDetail(ExchangeManager, "ReliableMessageMgr::StartTimer set timer for %" PRIu64, timerArmValue);
#endif
            StopTimer();
            res = mSystemLayer->StartTimer((uint32_t) timerArmValue, Timeout, this);

            VerifyOrDieWithMsg(res == CHIP_NO_ERROR, ExchangeManager,
                               "Cannot start ReliableMessageMgr::Timeout %" CHIP_ERROR_FORMAT, res.Format());
            mCurrentTimerExpiry = timerExpiry;
#if defined(RMP_TICKLESS_DEBUG)
        }
        else
        {
            ChipLogDetail(ExchangeManager, "ReliableMessageMgr::StartTimer timer already set for %" PRIu64, timerExpiry);
#endif
        }
    }
    else
    {
#if defined(RMP_TICKLESS_DEBUG)
        ChipLogDetail(ExchangeManager, "Not setting ReliableMessageProtocol timeout at %" PRIu64,
                      System::SystemClock().GetMonotonicMilliseconds());
#endif
        StopTimer();
    }

    TicklessDebugDumpRetransTable("ReliableMessageMgr::StartTimer Dumping mRetransTable entries after setting wakeup times");
}

void ReliableMessageMgr::StopTimer()
{
    mSystemLayer->CancelTimer(Timeout, this);
}

#if CHIP_CONFIG_TEST
int ReliableMessageMgr::TestGetCountRetransTable()
{
    int count = 0;
    mRetransTable.ForEachActiveObject([&](auto * entry) {
        count++;
        return true;
    });
    return count;
}
#endif // CHIP_CONFIG_TEST

} // namespace Messaging
} // namespace chip
