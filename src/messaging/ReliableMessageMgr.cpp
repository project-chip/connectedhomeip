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

#include <errno.h>
#include <inttypes.h>

#include <messaging/ReliableMessageMgr.h>

#include <lib/support/BitFlags.h>
#include <lib/support/CHIPFaultInjection.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/logging/CHIPLogging.h>
#include <messaging/ErrorCategory.h>
#include <messaging/ExchangeMessageDispatch.h>
#include <messaging/ExchangeMgr.h>
#include <messaging/Flags.h>
#include <messaging/ReliableMessageContext.h>
#include <platform/ConnectivityManager.h>

using namespace chip::System::Clock::Literals;

namespace chip {
namespace Messaging {

ReliableMessageMgr::RetransTableEntry::RetransTableEntry(ReliableMessageContext * rc) :
    ec(*rc->GetExchangeContext()), nextRetransTime(0), sendCount(0)
{
    ec->SetMessageNotAcked(true);
}

ReliableMessageMgr::RetransTableEntry::~RetransTableEntry()
{
    ec->SetMessageNotAcked(false);
}

ReliableMessageMgr::ReliableMessageMgr(ObjectPool<ExchangeContext, CHIP_CONFIG_MAX_EXCHANGE_CONTEXTS> & contextPool) :
    mContextPool(contextPool), mSystemLayer(nullptr)
{}

ReliableMessageMgr::~ReliableMessageMgr() {}

void ReliableMessageMgr::Init(chip::System::Layer * systemLayer)
{
    mSystemLayer = systemLayer;
}

void ReliableMessageMgr::Shutdown()
{
    StopTimer();

    // Clear the retransmit table
    mRetransTable.ForEachActiveObject([&](auto * entry) {
        mRetransTable.ReleaseObject(entry);
        return Loop::Continue;
    });

    mSystemLayer = nullptr;
}

#if defined(RMP_TICKLESS_DEBUG)
void ReliableMessageMgr::TicklessDebugDumpRetransTable(const char * log)
{
    ChipLogDetail(ExchangeManager, log);

    mRetransTable.ForEachActiveObject([&](auto * entry) {
        ChipLogDetail(ExchangeManager,
                      "EC:" ChipLogFormatExchange " MessageCounter:" ChipLogFormatMessageCounter " NextRetransTimeCtr:%" PRIu64,
                      ChipLogValueExchange(&entry->ec.Get()), entry->retainedBuf.GetMessageCounter(),
                      entry->nextRetransTime.count());
        return Loop::Continue;
    });
}
#else
void ReliableMessageMgr::TicklessDebugDumpRetransTable(const char * log) {}
#endif // RMP_TICKLESS_DEBUG

void ReliableMessageMgr::ExecuteActions()
{
    System::Clock::Timestamp now = System::SystemClock().GetMonotonicTimestamp();

#if defined(RMP_TICKLESS_DEBUG)
    ChipLogDetail(ExchangeManager, "ReliableMessageMgr::ExecuteActions at % " PRIu64 "ms", now.count());
#endif

    ExecuteForAllContext([&](ReliableMessageContext * rc) {
        if (rc->IsAckPending())
        {
            if (rc->mNextAckTime <= now)
            {
#if defined(RMP_TICKLESS_DEBUG)
                ChipLogDetail(ExchangeManager, "ReliableMessageMgr::ExecuteActions sending ACK %p", rc);
#endif
                rc->SendStandaloneAckMessage();
            }
        }
    });

    // Retransmit / cancel anything in the retrans table whose retrans timeout has expired
    mRetransTable.ForEachActiveObject([&](auto * entry) {
        if (entry->nextRetransTime > now)
            return Loop::Continue;

        VerifyOrDie(!entry->retainedBuf.IsNull());

        uint8_t sendCount = entry->sendCount;
#if CHIP_ERROR_LOGGING || CHIP_DETAIL_LOGGING
        uint32_t messageCounter = entry->retainedBuf.GetMessageCounter();
#endif // CHIP_ERROR_LOGGING || CHIP_DETAIL_LOGGING

        if (sendCount == CHIP_CONFIG_RMP_DEFAULT_MAX_RETRANS)
        {
            ChipLogError(ExchangeManager,
                         "Failed to Send CHIP MessageCounter:" ChipLogFormatMessageCounter " on exchange " ChipLogFormatExchange
                         " sendCount: %u max retries: %d",
                         messageCounter, ChipLogValueExchange(&entry->ec.Get()), sendCount, CHIP_CONFIG_RMP_DEFAULT_MAX_RETRANS);

            // Don't check whether the session in the exchange is valid, because when the session is released, the retrans entry is
            // cleared inside ExchangeContext::OnSessionReleased, so the session must be valid if the entry exists.
            SessionHandle session = entry->ec->GetSessionHandle();

            // If the exchange is expecting a response, it will handle sending
            // this notification once it detects that it has not gotten a
            // response.  Otherwise, we need to do it.
            if (!entry->ec->IsResponseExpected())
            {
                if (session->IsSecureSession() && session->AsSecureSession()->IsCASESession())
                {
                    session->AsSecureSession()->MarkAsDefunct();
                }
                session->DispatchSessionEvent(&SessionDelegate::OnSessionHang);
            }

            // Do not StartTimer, we will schedule the timer at the end of the timer handler.
            mRetransTable.ReleaseObject(entry);
            return Loop::Continue;
        }

        entry->sendCount++;
        ChipLogDetail(ExchangeManager,
                      "Retransmitting MessageCounter:" ChipLogFormatMessageCounter " on exchange " ChipLogFormatExchange
                      " Send Cnt %d",
                      messageCounter, ChipLogValueExchange(&entry->ec.Get()), entry->sendCount);

        // Choose active/idle timeout from PeerActiveMode of session per 4.11.2.1. Retransmissions.
        System::Clock::Timestamp baseTimeout = entry->ec->GetSessionHandle()->GetMRPBaseTimeout();
        System::Clock::Timestamp backoff     = ReliableMessageMgr::GetBackoff(baseTimeout, entry->sendCount);
        entry->nextRetransTime               = System::SystemClock().GetMonotonicTimestamp() + backoff;
        SendFromRetransTable(entry);

        return Loop::Continue;
    });

    TicklessDebugDumpRetransTable("ReliableMessageMgr::ExecuteActions Dumping mRetransTable entries after processing");
}

void ReliableMessageMgr::Timeout(System::Layer * aSystemLayer, void * aAppState)
{
    ReliableMessageMgr * manager = reinterpret_cast<ReliableMessageMgr *>(aAppState);

    VerifyOrDie((aSystemLayer != nullptr) && (manager != nullptr));

#if defined(RMP_TICKLESS_DEBUG)
    ChipLogDetail(ExchangeManager, "ReliableMessageMgr::Timeout\n");
#endif

    // Execute any actions that are due this tick
    manager->ExecuteActions();

    // Calculate next physical wakeup
    manager->StartTimer();
}

CHIP_ERROR ReliableMessageMgr::AddToRetransTable(ReliableMessageContext * rc, RetransTableEntry ** rEntry)
{
    VerifyOrDie(!rc->IsMessageNotAcked());

    *rEntry = mRetransTable.CreateObject(rc);
    if (*rEntry == nullptr)
    {
        ChipLogError(ExchangeManager, "mRetransTable Already Full");
        return CHIP_ERROR_RETRANS_TABLE_FULL;
    }

    return CHIP_NO_ERROR;
}

System::Clock::Timestamp ReliableMessageMgr::GetBackoff(System::Clock::Timestamp baseInterval, uint8_t sendCount)
{
    // See section "4.11.8. Parameters and Constants" for the parameters below:
    // MRP_BACKOFF_JITTER = 0.25
    constexpr uint32_t MRP_BACKOFF_JITTER_BASE = 1024;
    // MRP_BACKOFF_MARGIN = 1.1
    constexpr uint32_t MRP_BACKOFF_MARGIN_NUMERATOR   = 1127;
    constexpr uint32_t MRP_BACKOFF_MARGIN_DENOMINATOR = 1024;
    // MRP_BACKOFF_BASE = 1.6
    constexpr uint32_t MRP_BACKOFF_BASE_NUMERATOR   = 16;
    constexpr uint32_t MRP_BACKOFF_BASE_DENOMINATOR = 10;
    constexpr int MRP_BACKOFF_THRESHOLD             = 1;

    // Implement `i = MRP_BACKOFF_MARGIN * i` from section "4.11.2.1. Retransmissions", where:
    //   i == baseInterval
    baseInterval = baseInterval * MRP_BACKOFF_MARGIN_NUMERATOR / MRP_BACKOFF_MARGIN_DENOMINATOR;

    // Implement:
    //   mrpBackoffTime = i * MRP_BACKOFF_BASE^(max(0,n-MRP_BACKOFF_THRESHOLD)) * (1.0 + random(0,1) * MRP_BACKOFF_JITTER)
    // from section "4.11.2.1. Retransmissions", where:
    //   i == baseInterval
    //   n == sendCount

    // 1. Calculate exponent `max(0,n−MRP_BACKOFF_THRESHOLD)`
    int exponent = sendCount - MRP_BACKOFF_THRESHOLD;
    if (exponent < 0)
        exponent = 0; // Enforce floor
    if (exponent > 4)
        exponent = 4; // Enforce reasonable maximum after 5 tries

    // 2. Calculate `mrpBackoffTime = i * MRP_BACKOFF_BASE^(max(0,n-MRP_BACKOFF_THRESHOLD))`
    uint32_t backoffNum   = 1;
    uint32_t backoffDenom = 1;

    for (int i = 0; i < exponent; i++)
    {
        backoffNum *= MRP_BACKOFF_BASE_NUMERATOR;
        backoffDenom *= MRP_BACKOFF_BASE_DENOMINATOR;
    }

    System::Clock::Timestamp mrpBackoffTime = baseInterval * backoffNum / backoffDenom;

    // 3. Calculate `mrpBackoffTime *= (1.0 + random(0,1) * MRP_BACKOFF_JITTER)`
    uint32_t jitter = MRP_BACKOFF_JITTER_BASE + Crypto::GetRandU8();
    mrpBackoffTime  = mrpBackoffTime * jitter / MRP_BACKOFF_JITTER_BASE;

#if CHIP_DEVICE_CONFIG_ENABLE_SED
    // Implement:
    //   "A sleepy sender SHOULD increase t to also account for its own sleepy interval
    //   required to receive the acknowledgment"
    DeviceLayer::ConnectivityManager::SEDIntervalsConfig sedIntervals;

    if (DeviceLayer::ConnectivityMgr().GetSEDIntervalsConfig(sedIntervals) == CHIP_NO_ERROR)
    {
        mrpBackoffTime += System::Clock::Timestamp(sedIntervals.ActiveIntervalMS);
    }
#endif

    return mrpBackoffTime;
}

void ReliableMessageMgr::StartRetransmision(RetransTableEntry * entry)
{
    // Choose active/idle timeout from PeerActiveMode of session per 4.11.2.1. Retransmissions.
    System::Clock::Timestamp baseTimeout = entry->ec->GetSessionHandle()->GetMRPBaseTimeout();
    System::Clock::Timestamp backoff     = ReliableMessageMgr::GetBackoff(baseTimeout, entry->sendCount);
    entry->nextRetransTime               = System::SystemClock().GetMonotonicTimestamp() + backoff;
    StartTimer();
}

bool ReliableMessageMgr::CheckAndRemRetransTable(ReliableMessageContext * rc, uint32_t ackMessageCounter)
{
    bool removed = false;
    mRetransTable.ForEachActiveObject([&](auto * entry) {
        if (entry->ec->GetReliableMessageContext() == rc && entry->retainedBuf.GetMessageCounter() == ackMessageCounter)
        {
            // Clear the entry from the retransmision table.
            ClearRetransTable(*entry);

            ChipLogDetail(ExchangeManager,
                          "Rxd Ack; Removing MessageCounter:" ChipLogFormatMessageCounter
                          " from Retrans Table on exchange " ChipLogFormatExchange,
                          ackMessageCounter, ChipLogValueExchange(rc->GetExchangeContext()));
            removed = true;
            return Loop::Break;
        }
        return Loop::Continue;
    });

    return removed;
}

CHIP_ERROR ReliableMessageMgr::SendFromRetransTable(RetransTableEntry * entry)
{
    if (!entry->ec->HasSessionHandle())
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

    auto * sessionManager = entry->ec->GetExchangeMgr()->GetSessionManager();
    CHIP_ERROR err        = sessionManager->SendPreparedMessage(entry->ec->GetSessionHandle(), entry->retainedBuf);
    err                   = MapSendError(err, entry->ec->GetExchangeId(), entry->ec->IsInitiator());

    if (err == CHIP_NO_ERROR)
    {
#if CHIP_CONFIG_RESOLVE_PEER_ON_FIRST_TRANSMIT_FAILURE
        const ExchangeManager * exchangeMgr = entry->ec->GetExchangeMgr();
        // TODO: investigate why in ReliableMessageMgr::CheckResendApplicationMessageWithPeerExchange unit test released exchange
        // context with mExchangeMgr==nullptr is used.
        if (exchangeMgr)
        {
            // After the first failure notify session manager to refresh device data
            if (entry->sendCount == 1 && mSessionUpdateDelegate != nullptr && entry->ec->GetSessionHandle()->IsSecureSession() &&
                entry->ec->GetSessionHandle()->AsSecureSession()->IsCASESession())
            {
                ChipLogDetail(ExchangeManager, "Notify session manager to update peer address");
                mSessionUpdateDelegate->UpdatePeerAddress(entry->ec->GetSessionHandle()->GetPeer());
            }
        }
#endif // CHIP_CONFIG_RESOLVE_PEER_ON_FIRST_TRANSMIT_FAILURE
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
    mRetransTable.ForEachActiveObject([&](auto * entry) {
        if (entry->ec->GetReliableMessageContext() == rc)
        {
            ClearRetransTable(*entry);
            return Loop::Break;
        }
        return Loop::Continue;
    });
}

void ReliableMessageMgr::ClearRetransTable(RetransTableEntry & entry)
{
    mRetransTable.ReleaseObject(&entry);
    // Expire any virtual ticks that have expired so all wakeup sources reflect the current time
    StartTimer();
}

void ReliableMessageMgr::StartTimer()
{
    // When do we need to next wake up to send an ACK?
    System::Clock::Timestamp nextWakeTime = System::Clock::Timestamp::max();

    ExecuteForAllContext([&](ReliableMessageContext * rc) {
        if (rc->IsAckPending() && rc->mNextAckTime < nextWakeTime)
        {
            nextWakeTime = rc->mNextAckTime;
        }
    });

    // When do we need to next wake up for ReliableMessageProtocol retransmit?
    mRetransTable.ForEachActiveObject([&](auto * entry) {
        if (entry->nextRetransTime < nextWakeTime)
        {
            nextWakeTime = entry->nextRetransTime;
        }
        return Loop::Continue;
    });

    if (nextWakeTime != System::Clock::Timestamp::max())
    {
#if defined(RMP_TICKLESS_DEBUG)
        ChipLogDetail(ExchangeManager, "ReliableMessageMgr::StartTimer wake at %" PRIu64 "ms", nextWakeTime);
#endif

        StopTimer();

        const System::Clock::Timestamp now = System::SystemClock().GetMonotonicTimestamp();
        const auto nextWakeDelay           = (nextWakeTime > now) ? nextWakeTime - now : 0_ms;
        VerifyOrDie(mSystemLayer->StartTimer(nextWakeDelay, Timeout, this) == CHIP_NO_ERROR);
    }
    else
    {
#if defined(RMP_TICKLESS_DEBUG)
        ChipLogDetail(ExchangeManager, "ReliableMessageMgr skipped timer");
#endif
        StopTimer();
    }

    TicklessDebugDumpRetransTable("ReliableMessageMgr::StartTimer Dumping mRetransTable entries after setting wakeup times");
}

void ReliableMessageMgr::StopTimer()
{
    mSystemLayer->CancelTimer(Timeout, this);
}

void ReliableMessageMgr::RegisterSessionUpdateDelegate(SessionUpdateDelegate * sessionUpdateDelegate)
{
    mSessionUpdateDelegate = sessionUpdateDelegate;
}

CHIP_ERROR ReliableMessageMgr::MapSendError(CHIP_ERROR error, uint16_t exchangeId, bool isInitiator)
{
    if (
#if CHIP_SYSTEM_CONFIG_USE_LWIP
        error == System::MapErrorLwIP(ERR_MEM)
#else
        error == CHIP_ERROR_POSIX(ENOBUFS)
#endif // CHIP_SYSTEM_CONFIG_USE_LWIP
    )
    {
        // sendmsg on BSD-based systems never blocks, no matter how the
        // socket is configured, and will return ENOBUFS in situation in
        // which Linux, for example, blocks.
        //
        // This is typically a transient situation, so we pretend like this
        // packet drop happened somewhere on the network instead of inside
        // sendmsg and will just resend it in the normal MRP way later.
        //
        // Similarly, on LwIP an ERR_MEM on send indicates a likely
        // temporary lack of TX buffers.
        ChipLogError(ExchangeManager, "Ignoring transient send error: %" CHIP_ERROR_FORMAT " on exchange " ChipLogFormatExchangeId,
                     error.Format(), ChipLogValueExchangeId(exchangeId, isInitiator));
        error = CHIP_NO_ERROR;
    }

    return error;
}

#if CHIP_CONFIG_TEST
int ReliableMessageMgr::TestGetCountRetransTable()
{
    int count = 0;
    mRetransTable.ForEachActiveObject([&](auto * entry) {
        count++;
        return Loop::Continue;
    });
    return count;
}
#endif // CHIP_CONFIG_TEST

} // namespace Messaging
} // namespace chip
