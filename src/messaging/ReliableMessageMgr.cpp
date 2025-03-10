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

#include <app/icd/server/ICDServerConfig.h>
#include <lib/support/BitFlags.h>
#include <lib/support/CHIPFaultInjection.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/logging/CHIPLogging.h>
#include <messaging/ErrorCategory.h>
#include <messaging/ExchangeMessageDispatch.h>
#include <messaging/ExchangeMgr.h>
#include <messaging/Flags.h>
#include <messaging/ReliableMessageContext.h>
#include <messaging/ReliableMessageMgr.h>
#include <platform/ConnectivityManager.h>
#include <tracing/metric_event.h>

#if CHIP_CONFIG_ENABLE_ICD_SERVER
#include <app/icd/server/ICDConfigurationData.h> // nogncheck
#include <app/icd/server/ICDNotifier.h>          // nogncheck
#endif

using namespace chip::System::Clock::Literals;

namespace chip {
namespace Messaging {

System::Clock::Timeout ReliableMessageMgr::sAdditionalMRPBackoffTime = CHIP_CONFIG_MRP_RETRY_INTERVAL_SENDER_BOOST;

ReliableMessageMgr::RetransTableEntry::RetransTableEntry(ReliableMessageContext * rc) :
    ec(*rc->GetExchangeContext()), nextRetransTime(0), sendCount(0)
{
    ec->SetWaitingForAck(true);
}

ReliableMessageMgr::RetransTableEntry::~RetransTableEntry()
{
    ec->SetWaitingForAck(false);
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

void ReliableMessageMgr::TicklessDebugDumpRetransTable(const char * log)
{
#if defined(RMP_TICKLESS_DEBUG)
    ChipLogDetail(ExchangeManager, "%s", log);

    mRetransTable.ForEachActiveObject([&](auto * entry) {
        ChipLogDetail(ExchangeManager,
                      "EC:" ChipLogFormatExchange " MessageCounter:" ChipLogFormatMessageCounter
                      " NextRetransTimeCtr: 0x" ChipLogFormatX64,
                      ChipLogValueExchange(&entry->ec.Get()), entry->retainedBuf.GetMessageCounter(),
                      ChipLogValueX64(entry->nextRetransTime.count()));
        return Loop::Continue;
    });
#endif
}

#if CHIP_CONFIG_MRP_ANALYTICS_ENABLED
void ReliableMessageMgr::NotifyMessageSendAnalytics(const RetransTableEntry & entry, const SessionHandle & sessionHandle,
                                                    const ReliableMessageAnalyticsDelegate::EventType & eventType)
{
    // For now we only support sending analytics for messages being sent over an established CASE session.
    if (!mAnalyticsDelegate || !sessionHandle->IsSecureSession())
    {
        return;
    }

    auto secureSession = sessionHandle->AsSecureSession();
    if (!secureSession->IsCASESession())
    {
        return;
    }

    uint32_t messageCounter                               = entry.retainedBuf.GetMessageCounter();
    auto fabricIndex                                      = sessionHandle->GetFabricIndex();
    auto destination                                      = secureSession->GetPeerNodeId();
    ReliableMessageAnalyticsDelegate::TransmitEvent event = { .nodeId      = destination,
                                                              .fabricIndex = fabricIndex,
                                                              .sessionType =
                                                                  ReliableMessageAnalyticsDelegate::SessionType::kEstablishedCase,
                                                              .eventType      = eventType,
                                                              .messageCounter = messageCounter };

    if (eventType == ReliableMessageAnalyticsDelegate::EventType::kRetransmission)
    {
        event.retransmissionCount = entry.sendCount;
    }

    mAnalyticsDelegate->OnTransmitEvent(event);
}
#endif // CHIP_CONFIG_MRP_ANALYTICS_ENABLED

void ReliableMessageMgr::ExecuteActions()
{
    System::Clock::Timestamp now = System::SystemClock().GetMonotonicTimestamp();

#if defined(RMP_TICKLESS_DEBUG)
    ChipLogDetail(ExchangeManager, "ReliableMessageMgr::ExecuteActions at 0x" ChipLogFormatX64 "ms", ChipLogValueX64(now.count()));
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

        // Don't check whether the session in the exchange is valid, because when the session is released, the retrans entry is
        // cleared inside ExchangeContext::OnSessionReleased, so the session must be valid if the entry exists.
        auto session      = entry->ec->GetSessionHandle();
        uint8_t sendCount = entry->sendCount;
#if CHIP_ERROR_LOGGING || CHIP_PROGRESS_LOGGING
        uint32_t messageCounter = entry->retainedBuf.GetMessageCounter();
        auto fabricIndex        = session->GetFabricIndex();
        auto destination        = kUndefinedNodeId;
        if (session->IsSecureSession())
        {
            destination = session->AsSecureSession()->GetPeerNodeId();
        }
#endif // CHIP_ERROR_LOGGING || CHIP_DETAIL_LOGGING

        if (sendCount == CHIP_CONFIG_RMP_DEFAULT_MAX_RETRANS)
        {
            // Make sure our exchange stays alive until we are done working with it.
            ExchangeHandle ec(entry->ec);

            ChipLogError(ExchangeManager,
                         "<<%d [E:" ChipLogFormatExchange " S:%u M:" ChipLogFormatMessageCounter
                         "] (%s) Msg Retransmission to %u:" ChipLogFormatX64 " failure (max retries:%d)",
                         sendCount + 1, ChipLogValueExchange(&entry->ec.Get()), session->SessionIdForLogging(), messageCounter,
                         Transport::GetSessionTypeString(session), fabricIndex, ChipLogValueX64(destination),
                         CHIP_CONFIG_RMP_DEFAULT_MAX_RETRANS);

#if CHIP_CONFIG_MRP_ANALYTICS_ENABLED
            NotifyMessageSendAnalytics(*entry, session, ReliableMessageAnalyticsDelegate::EventType::kFailed);
#endif // CHIP_CONFIG_MRP_ANALYTICS_ENABLED

            // If the exchange is expecting a response, it will handle sending
            // this notification once it detects that it has not gotten a
            // response.  Otherwise, we need to do it.
            if (!ec->IsResponseExpected())
            {
                if (session->IsSecureSession() && session->AsSecureSession()->IsCASESession())
                {
                    session->AsSecureSession()->MarkAsDefunct();
                }
                session->NotifySessionHang();
            }

            // Do not StartTimer, we will schedule the timer at the end of the timer handler.
            mRetransTable.ReleaseObject(entry);

            return Loop::Continue;
        }

        entry->sendCount++;

        ChipLogProgress(ExchangeManager,
                        "<<%d [E:" ChipLogFormatExchange " S:%u M:" ChipLogFormatMessageCounter
                        "] (%s) Msg Retransmission to %u:" ChipLogFormatX64,
                        entry->sendCount, ChipLogValueExchange(&entry->ec.Get()), session->SessionIdForLogging(), messageCounter,
                        Transport::GetSessionTypeString(session), fabricIndex, ChipLogValueX64(destination));
        MATTER_LOG_METRIC(Tracing::kMetricDeviceRMPRetryCount, entry->sendCount);

        CalculateNextRetransTime(*entry);
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
    ChipLogDetail(ExchangeManager, "ReliableMessageMgr::Timeout");
#endif

    // Execute any actions that are due this tick
    manager->ExecuteActions();

    // Calculate next physical wakeup
    manager->StartTimer();
}

CHIP_ERROR ReliableMessageMgr::AddToRetransTable(ReliableMessageContext * rc, RetransTableEntry ** rEntry)
{
    VerifyOrReturnError(!rc->IsWaitingForAck(), CHIP_ERROR_INCORRECT_STATE);

    *rEntry = mRetransTable.CreateObject(rc);
    if (*rEntry == nullptr)
    {
        ChipLogError(ExchangeManager, "mRetransTable Already Full");
        return CHIP_ERROR_RETRANS_TABLE_FULL;
    }

    return CHIP_NO_ERROR;
}

System::Clock::Timeout ReliableMessageMgr::GetBackoff(System::Clock::Timeout baseInterval, uint8_t sendCount,
                                                      bool computeMaxPossible)
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

    // Implement `i = MRP_BACKOFF_MARGIN * i` from section "4.12.2.1. Retransmissions", where:
    //   i == interval
    System::Clock::Milliseconds64 interval = baseInterval;
    interval *= MRP_BACKOFF_MARGIN_NUMERATOR;
    interval /= MRP_BACKOFF_MARGIN_DENOMINATOR;

    // Implement:
    //   mrpBackoffTime = i * MRP_BACKOFF_BASE^(max(0,n-MRP_BACKOFF_THRESHOLD)) * (1.0 + random(0,1) * MRP_BACKOFF_JITTER)
    // from section "4.12.2.1. Retransmissions", where:
    //   i == interval
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

    System::Clock::Milliseconds64 mrpBackoffTime = interval * backoffNum / backoffDenom;

    // 3. Calculate `mrpBackoffTime *= (1.0 + random(0,1) * MRP_BACKOFF_JITTER)`
    uint32_t jitter = MRP_BACKOFF_JITTER_BASE + (computeMaxPossible ? UINT8_MAX : Crypto::GetRandU8());
    mrpBackoffTime  = mrpBackoffTime * jitter / MRP_BACKOFF_JITTER_BASE;

#if CHIP_CONFIG_ENABLE_ICD_SERVER
    // Implement:
    //   "An ICD sender SHOULD increase t to also account for its own sleepy interval
    //   required to receive the acknowledgment"
    mrpBackoffTime += ICDConfigurationData::GetInstance().GetFastPollingInterval();
#endif

    mrpBackoffTime += sAdditionalMRPBackoffTime;

    return std::chrono::duration_cast<System::Clock::Timeout>(mrpBackoffTime);
}

void ReliableMessageMgr::StartRetransmision(RetransTableEntry * entry)
{
    CalculateNextRetransTime(*entry);
#if CHIP_CONFIG_MRP_ANALYTICS_ENABLED
    NotifyMessageSendAnalytics(*entry, entry->ec->GetSessionHandle(), ReliableMessageAnalyticsDelegate::EventType::kInitialSend);
#endif // CHIP_CONFIG_MRP_ANALYTICS_ENABLED
    StartTimer();
}

bool ReliableMessageMgr::CheckAndRemRetransTable(ReliableMessageContext * rc, uint32_t ackMessageCounter)
{
    bool removed = false;
    mRetransTable.ForEachActiveObject([&](auto * entry) {
        if (entry->ec->GetReliableMessageContext() == rc && entry->retainedBuf.GetMessageCounter() == ackMessageCounter)
        {
#if CHIP_CONFIG_MRP_ANALYTICS_ENABLED
            auto session = entry->ec->GetSessionHandle();
            NotifyMessageSendAnalytics(*entry, session, ReliableMessageAnalyticsDelegate::EventType::kAcknowledged);
#endif // CHIP_CONFIG_MRP_ANALYTICS_ENABLED

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
#if CHIP_CONFIG_ENABLE_ICD_SERVER
        app::ICDNotifier::GetInstance().NotifyNetworkActivityNotification();
#endif // CHIP_CONFIG_ENABLE_ICD_SERVER
#if CHIP_CONFIG_MRP_ANALYTICS_ENABLED
        NotifyMessageSendAnalytics(*entry, entry->ec->GetSessionHandle(),
                                   ReliableMessageAnalyticsDelegate::EventType::kRetransmission);
#endif // CHIP_CONFIG_MRP_ANALYTICS_ENABLED
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

    StopTimer();

    if (nextWakeTime != System::Clock::Timestamp::max())
    {
        const System::Clock::Timestamp now = System::SystemClock().GetMonotonicTimestamp();
        const auto nextWakeDelay           = (nextWakeTime > now) ? nextWakeTime - now : 0_ms;

#if defined(RMP_TICKLESS_DEBUG)
        ChipLogDetail(ExchangeManager,
                      "ReliableMessageMgr::StartTimer at 0x" ChipLogFormatX64 "ms wake at 0x" ChipLogFormatX64
                      "ms (in 0x" ChipLogFormatX64 "ms)",
                      ChipLogValueX64(now.count()), ChipLogValueX64(nextWakeTime.count()), ChipLogValueX64(nextWakeDelay.count()));
#endif
        VerifyOrDie(mSystemLayer->StartTimer(nextWakeDelay, Timeout, this) == CHIP_NO_ERROR);
    }
    else
    {
#if defined(RMP_TICKLESS_DEBUG)
        ChipLogDetail(ExchangeManager, "ReliableMessageMgr skipped timer");
#endif
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

#if CHIP_CONFIG_MRP_ANALYTICS_ENABLED
void ReliableMessageMgr::RegisterAnalyticsDelegate(ReliableMessageAnalyticsDelegate * analyticsDelegate)
{
    mAnalyticsDelegate = analyticsDelegate;
}
#endif // CHIP_CONFIG_MRP_ANALYTICS_ENABLED

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

void ReliableMessageMgr::SetAdditionalMRPBackoffTime(const Optional<System::Clock::Timeout> & additionalTime)
{
    sAdditionalMRPBackoffTime = additionalTime.ValueOr(CHIP_CONFIG_MRP_RETRY_INTERVAL_SENDER_BOOST);
}

void ReliableMessageMgr::CalculateNextRetransTime(RetransTableEntry & entry)
{
    System::Clock::Timeout baseTimeout = System::Clock::Timeout(0);
    const auto sessionHandle           = entry.ec->GetSessionHandle();

    // Check if we have received at least one application-level message
    if (entry.ec->HasReceivedAtLeastOneMessage())
    {
        // If we have received at least one message, assume peer is active and use ActiveRetransTimeout
        baseTimeout = sessionHandle->GetRemoteMRPConfig().mActiveRetransTimeout;
    }
    else
    {
        // If we haven't received at least one message
        // Choose active/idle timeout from PeerActiveMode of session per 4.11.2.1. Retransmissions.
        baseTimeout = sessionHandle->GetMRPBaseTimeout();
    }

    System::Clock::Timeout backoff = ReliableMessageMgr::GetBackoff(baseTimeout, entry.sendCount);
    entry.nextRetransTime          = System::SystemClock().GetMonotonicTimestamp() + backoff;

#if CHIP_PROGRESS_LOGGING
    const auto config       = sessionHandle->GetRemoteMRPConfig();
    uint32_t messageCounter = entry.retainedBuf.GetMessageCounter();
    auto fabricIndex        = sessionHandle->GetFabricIndex();
    auto destination        = kUndefinedNodeId;
    bool peerIsActive       = false;

    if (sessionHandle->IsSecureSession())
    {
        peerIsActive = sessionHandle->AsSecureSession()->IsPeerActive();
        destination  = sessionHandle->AsSecureSession()->GetPeerNodeId();
    }
    else if (sessionHandle->IsUnauthenticatedSession())
    {
        peerIsActive = sessionHandle->AsUnauthenticatedSession()->IsPeerActive();
    }

    ChipLogProgress(ExchangeManager,
                    "??%d [E:" ChipLogFormatExchange " S:%u M:" ChipLogFormatMessageCounter
                    "] (%s) Msg Retransmission to %u:" ChipLogFormatX64 " scheduled for %" PRIu32
                    "ms from now [State:%s II:%" PRIu32 " AI:%" PRIu32 " AT:%u]",
                    entry.sendCount + 1, ChipLogValueExchange(&entry.ec.Get()), sessionHandle->SessionIdForLogging(),
                    messageCounter, Transport::GetSessionTypeString(sessionHandle), fabricIndex, ChipLogValueX64(destination),
                    backoff.count(), peerIsActive ? "Active" : "Idle", config.mIdleRetransTimeout.count(),
                    config.mActiveRetransTimeout.count(), config.mActiveThresholdTime.count());
#endif // CHIP_PROGRESS_LOGGING
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
