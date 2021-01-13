/*
 *    Copyright (c) 2020 Project CHIP Authors
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

#include <messaging/ReliableMessageManager.h>

#include <messaging/ErrorCategory.h>
#include <messaging/Flags.h>
#include <messaging/ReliableMessageContext.h>
#include <support/BitFlags.h>
#include <support/CHIPFaultInjection.h>
#include <support/CodeUtils.h>
#include <support/logging/CHIPLogging.h>

namespace chip {
namespace Messaging {

ReliableMessageManager::RetransTableEntry::RetransTableEntry() : rc(nullptr), nextRetransTimeTick(0), sendCount(0) {}

ReliableMessageManager::ReliableMessageManager(std::array<ExchangeContext, CHIP_CONFIG_MAX_EXCHANGE_CONTEXTS> & contextPool) :
    mContextPool(contextPool), mSystemLayer(nullptr), mSessionMgr(nullptr), mCurrentTimerExpiry(0),
    mTimerIntervalShift(CHIP_CONFIG_RMP_TIMER_DEFAULT_PERIOD_SHIFT)
{}

ReliableMessageManager::~ReliableMessageManager() {}

void ReliableMessageManager::Init(chip::System::Layer * systemLayer, SecureSessionMgr * sessionMgr)
{
    mSystemLayer = systemLayer;
    mSessionMgr  = sessionMgr;

    mTimeStampBase      = System::Timer::GetCurrentEpoch();
    mCurrentTimerExpiry = 0;
}

void ReliableMessageManager::Shutdown()
{
    mSystemLayer = nullptr;
    mSessionMgr  = nullptr;

    StopTimer();

    // Clear the retransmit table
    for (RetransTableEntry & rEntry : mRetransTable)
    {
        ClearRetransTable(rEntry);
    }
}

/**
 * Return a tick counter value given a time period.
 *
 * @param[in]  newTime        Timestamp value of in milliseconds.
 *
 * @return Tick count for the time period.
 */
uint64_t ReliableMessageManager::GetTickCounterFromTimePeriod(uint64_t period)
{
    return (period >> mTimerIntervalShift);
}

/**
 * Return a tick counter value between the given time and the stored time.
 *
 * @param[in]  newTime        Timestamp value of in milliseconds.
 *
 * @return Tick count of the difference between the given time and the stored time.
 */
uint64_t ReliableMessageManager::GetTickCounterFromTimeDelta(uint64_t newTime)
{
    return GetTickCounterFromTimePeriod(newTime - mTimeStampBase);
}

#if defined(RMP_TICKLESS_DEBUG)
void ReliableMessageManager::TicklessDebugDumpRetransTable(const char * log)
{
    ChipLogProgress(ExchangeManager, log);

    for (int i = 0; i < CHIP_CONFIG_RMP_RETRANS_TABLE_SIZE; i++)
    {
        if (mRetransTable[i].rc)
        {
            ChipLogProgress(ExchangeManager, "EC:%04" PRIX16 " MsgId:%08" PRIX32 " NextRetransTimeCtr:%04" PRIX16,
                            mRetransTable[i].rc, mRetransTable[i].msgId, mRetransTable[i].nextRetransTimeTick);
        }
    }
}
#else
void ReliableMessageManager::TicklessDebugDumpRetransTable(const char * log)
{
    return;
}
#endif // RMP_TICKLESS_DEBUG

/**
 * Iterate through active exchange contexts and retrans table entries.  If an
 * action needs to be triggered by ReliableMessageProtocol time facilities,
 * execute that action.
 */
void ReliableMessageManager::ExecuteActions()
{
#if defined(RMP_TICKLESS_DEBUG)
    ChipLogProgress(ExchangeManager, "ReliableMessageManager::ExecuteActions");
#endif

    ExecuteForAllContext([](ReliableMessageContext * rc) {
        if (rc->IsAckPending())
        {
            if (0 == rc->mNextAckTimeTick)
            {
#if defined(RMP_TICKLESS_DEBUG)
                ChipLogProgress(ExchangeManager, "ReliableMessageManager::ExecuteActions sending ACK");
#endif
                // Send the Ack in a SecureChannel::StandaloneAck message
                rc->SendStandaloneAckMessage();
                rc->SetAckPending(false);
            }
        }
    });

    TicklessDebugDumpRetransTable("ReliableMessageManager::ExecuteActions Dumping mRetransTable entries before processing");

    // Retransmit / cancel anything in the retrans table whose retrans timeout
    // has expired
    for (int i = 0; i < CHIP_CONFIG_RMP_RETRANS_TABLE_SIZE; i++)
    {
        ReliableMessageContext * rc = mRetransTable[i].rc;
        CHIP_ERROR err              = CHIP_NO_ERROR;

        if (!rc || mRetransTable[i].nextRetransTimeTick != 0)
            continue;

        uint8_t sendCount = mRetransTable[i].sendCount;

        if (sendCount == rc->mConfig.mMaxRetrans)
        {
            err = CHIP_ERROR_MESSAGE_NOT_ACKNOWLEDGED;

            ChipLogError(ExchangeManager, "Failed to Send CHIP MsgId:%08" PRIX32 " sendCount: %" PRIu8 " max retries: %" PRIu8,
                         mRetransTable[i].retainedBuf.GetMsgId(), sendCount, rc->mConfig.mMaxRetrans);

            // Remove from Table
            ClearRetransTable(mRetransTable[i]);
        }

        // Resend from Table (if the operation fails, the entry is cleared)
        if (err == CHIP_NO_ERROR)
            err = SendFromRetransTable(&(mRetransTable[i]));

        if (err == CHIP_NO_ERROR)
        {
            // If the retransmission was successful, update the passive timer
            mRetransTable[i].nextRetransTimeTick = static_cast<uint16_t>(rc->GetCurrentRetransmitTimeoutTick());
#if !defined(NDEBUG)
            ChipLogProgress(ExchangeManager, "Retransmit MsgId:%08" PRIX32 " Send Cnt %d", mRetransTable[i].retainedBuf.GetMsgId(),
                            mRetransTable[i].sendCount);
#endif
        }

        if (err != CHIP_NO_ERROR && rc->mDelegate)
            rc->mDelegate->OnSendError(err);
    }

    TicklessDebugDumpRetransTable("ReliableMessageManager::ExecuteActions Dumping mRetransTable entries after processing");
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

/**
 * Calculate number of virtual ReliableMessageProtocol ticks that have expired
 * since we last called this function. Iterate through active exchange contexts
 * and retrans table entries, subtracting expired virtual ticks to synchronize
 * wakeup times with the current system time. Do not perform any actions beyond
 * updating tick counts, actions will be performed by the physical
 * ReliableMessageProtocol timer tick expiry.
 *
 */
void ReliableMessageManager::ExpireTicks()
{
    uint64_t now = System::Timer::GetCurrentEpoch();

    // Number of full ticks elapsed since last timer processing.  We always round down
    // to the previous tick.  If we are between tick boundaries, the extra time since the
    // last virtual tick is not accounted for here (it will be accounted for when resetting
    // the ReliableMessageProtocol timer)
    uint64_t deltaTicks = GetTickCounterFromTimeDelta(now);

#if defined(RMP_TICKLESS_DEBUG)
    ChipLogProgress(ExchangeManager, "ReliableMessageManager::ExpireTicks at %" PRIu64 ", %" PRIu64 ", %u", now, mTimeStampBase,
                    deltaTicks);
#endif

    ExecuteForAllContext([deltaTicks](ReliableMessageContext * rc) {
        if (rc->IsAckPending())
        {
            // Decrement counter of Ack timestamp by the elapsed timer ticks
            TickProceed(rc->mNextAckTimeTick, deltaTicks);
#if defined(RMP_TICKLESS_DEBUG)
            ChipLogProgress(ExchangeManager, "ReliableMessageManager::ExpireTicks set mNextAckTimeTick to %u",
                            rc->mNextAckTimeTick);
#endif
        }
    });

    for (int i = 0; i < CHIP_CONFIG_RMP_RETRANS_TABLE_SIZE; i++)
    {
        ReliableMessageContext * rc = mRetransTable[i].rc;
        if (rc)
        {
            // Decrement Retransmit timeout by elapsed timeticks
            TickProceed(mRetransTable[i].nextRetransTimeTick, deltaTicks);
#if defined(RMP_TICKLESS_DEBUG)
            ChipLogProgress(ExchangeManager, "ReliableMessageManager::ExpireTicks set nextRetransTimeTick to %u",
                            mRetransTable[i].nextRetransTimeTick);
#endif
        } // rc entry is allocated
    }

    // Re-Adjust the base time stamp to the most recent tick boundary
    mTimeStampBase += (deltaTicks << mTimerIntervalShift);

#if defined(RMP_TICKLESS_DEBUG)
    ChipLogProgress(ExchangeManager, "ReliableMessageManager::ExpireTicks mTimeStampBase to %" PRIu64, mTimeStampBase);
#endif
}

/**
 * Handle physical wakeup of system due to ReliableMessageProtocol wakeup.
 *
 */
void ReliableMessageManager::Timeout(System::Layer * aSystemLayer, void * aAppState, System::Error aError)
{
    ReliableMessageManager * manager = reinterpret_cast<ReliableMessageManager *>(aAppState);

    VerifyOrDie((aSystemLayer != nullptr) && (manager != nullptr));

#if defined(RMP_TICKLESS_DEBUG)
    ChipLogProgress(ExchangeManager, "ReliableMessageManager::Timeout\n");
#endif

    // Make sure all tick counts are sync'd to the current time
    manager->ExpireTicks();

    // Execute any actions that are due this tick
    manager->ExecuteActions();

    // Calculate next physical wakeup
    manager->StartTimer();
}

/**
 *  Add a CHIP message into the retransmission table to be subsequently resent if a corresponding acknowledgment
 *  is not received within the retransmission timeout.
 *
 *  @param[in]    rc        A pointer to the ExchangeContext object.
 *
 *  @param[out]   rEntry    A pointer to a pointer of a retransmission table entry added into the table.
 *
 *  @retval  #CHIP_ERROR_RETRANS_TABLE_FULL If there is no empty slot left in the table for addition.
 *  @retval  #CHIP_NO_ERROR On success.
 *
 */
CHIP_ERROR ReliableMessageManager::AddToRetransTable(ReliableMessageContext * rc, RetransTableEntry ** rEntry)
{
    bool added     = false;
    CHIP_ERROR err = CHIP_NO_ERROR;

    VerifyOrDie(rc != nullptr && rc->mExchange != nullptr);

    for (int i = 0; i < CHIP_CONFIG_RMP_RETRANS_TABLE_SIZE; i++)
    {
        // Check the exchContext pointer for finding an empty slot in Table
        if (!mRetransTable[i].rc)
        {
            // Expire any virtual ticks that have expired so all wakeup sources reflect the current time
            ExpireTicks();

            mRetransTable[i].rc          = rc;
            mRetransTable[i].sendCount   = 0;
            mRetransTable[i].retainedBuf = EncryptedPacketBufferHandle();

            *rEntry = &mRetransTable[i];

            // Increment the reference count
            rc->Retain();
            added = true;

            break;
        }
    }

    if (!added)
    {
        ChipLogError(ExchangeManager, "mRetransTable Already Full");
        err = CHIP_ERROR_RETRANS_TABLE_FULL;
    }

    return err;
}

void ReliableMessageManager::StartRetransmision(RetransTableEntry * entry)
{
    VerifyOrDie(entry != nullptr && entry->rc != nullptr);

    entry->nextRetransTimeTick = static_cast<uint16_t>(entry->rc->GetCurrentRetransmitTimeoutTick() +
                                                       GetTickCounterFromTimeDelta(System::Timer::GetCurrentEpoch()));

    // Check if the timer needs to be started and start it.
    StartTimer();
}

void ReliableMessageManager::PauseRetransmision(ReliableMessageContext * rc, uint32_t PauseTimeMillis)
{
    for (int i = 0; i < CHIP_CONFIG_RMP_RETRANS_TABLE_SIZE; i++)
    {
        if (mRetransTable[i].rc == rc)
        {
            mRetransTable[i].nextRetransTimeTick =
                static_cast<uint16_t>(mRetransTable[i].nextRetransTimeTick + (PauseTimeMillis >> mTimerIntervalShift));
            break;
        }
    }
}

void ReliableMessageManager::ResumeRetransmision(ReliableMessageContext * rc)
{
    for (int i = 0; i < CHIP_CONFIG_RMP_RETRANS_TABLE_SIZE; i++)
    {
        if (mRetransTable[i].rc == rc)
        {
            mRetransTable[i].nextRetransTimeTick = 0;
            break;
        }
    }
}

bool ReliableMessageManager::CheckAndRemRetransTable(ReliableMessageContext * rc, uint32_t ackMsgId)
{
    for (int i = 0; i < CHIP_CONFIG_RMP_RETRANS_TABLE_SIZE; i++)
    {
        if ((mRetransTable[i].rc == rc) && mRetransTable[i].retainedBuf.GetMsgId() == ackMsgId)
        {
            // Clear the entry from the retransmision table.
            ClearRetransTable(mRetransTable[i]);

#if !defined(NDEBUG)
            ChipLogProgress(ExchangeManager, "Rxd Ack; Removing MsgId:%08" PRIX32 " from Retrans Table", ackMsgId);
#endif
            return true;
        }
    }

    return false;
}

/**
 *  Send the specified entry from the retransmission table.
 *
 *  @param[in]    entry                A pointer to a retransmission table entry object that needs to be sent.
 *
 *  @return  #CHIP_NO_ERROR On success, else corresponding CHIP_ERROR returned from SendMessage.
 *
 */
CHIP_ERROR ReliableMessageManager::SendFromRetransTable(RetransTableEntry * entry)
{
    CHIP_ERROR err              = CHIP_NO_ERROR;
    ReliableMessageContext * rc = entry->rc;

    if (rc)
    {
        err = mSessionMgr->SendEncryptedMessage(entry->rc->mExchange->GetSecureSession(), std::move(entry->retainedBuf),
                                                &entry->retainedBuf);

        if (err == CHIP_NO_ERROR)
        {
            // Update the counters
            entry->sendCount++;
        }
        else
        {
            // Remove from table
            ChipLogError(ExchangeManager, "Crit-err %ld when sending CHIP MsgId:%08" PRIX32 ", send tries: %d", long(err),
                         entry->retainedBuf.GetMsgId(), entry->sendCount);

            ClearRetransTable(*entry);
        }
    }
    else
    {
        ChipLogError(ExchangeManager, "Table entry invalid");
    }

    return err;
}

/**
 *  Clear entries matching a specified ExchangeContext.
 *
 *  @param[in]    rc    A pointer to the ExchangeContext object.
 *
 */
void ReliableMessageManager::ClearRetransTable(ReliableMessageContext * rc)
{
    for (int i = 0; i < CHIP_CONFIG_RMP_RETRANS_TABLE_SIZE; i++)
    {
        if (mRetransTable[i].rc == rc)
        {
            // Clear the retransmit table entry.
            ClearRetransTable(mRetransTable[i]);
        }
    }
}

/**
 *  Clear an entry in the retransmission table.
 *
 *  @param[in]    rEntry   A reference to the RetransTableEntry object.
 *
 */
void ReliableMessageManager::ClearRetransTable(RetransTableEntry & rEntry)
{
    if (rEntry.rc)
    {
        VerifyOrDie(rEntry.rc->mExchange != nullptr);

        // Expire any virtual ticks that have expired so all wakeup sources reflect the current time
        ExpireTicks();

        rEntry.rc->Release();
        rEntry.rc = nullptr;

        // Clear all other fields
        rEntry = RetransTableEntry();

        // Schedule next physical wakeup
        StartTimer();
    }
}

/**
 *  Fail entries matching a specified ExchangeContext.
 *
 *  @param[in]    rc    A pointer to the ExchangeContext object.
 *
 *  @param[in]    err   The error for failing table entries.
 *
 */
void ReliableMessageManager::FailRetransTableEntries(ReliableMessageContext * rc, CHIP_ERROR err)
{
    for (int i = 0; i < CHIP_CONFIG_RMP_RETRANS_TABLE_SIZE; i++)
    {
        if (mRetransTable[i].rc == rc)
        {
            // Remove the entry from the retransmission table.
            ClearRetransTable(mRetransTable[i]);

            // Application callback OnSendError.
            rc->mDelegate->OnSendError(err);
        }
    }
}

/**
 * Iterate through active exchange contexts and retrans table entries.
 * Determine how many ReliableMessageProtocol ticks we need to sleep before we
 * need to physically wake the CPU to perform an action.  Set a timer to go off
 * when we next need to wake the system.
 */
void ReliableMessageManager::StartTimer()
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
            ChipLogProgress(ExchangeManager, "ReliableMessageManager::StartTimer next ACK time %u", nextWakeTimeTick);
#endif
        }
    });

    for (int i = 0; i < CHIP_CONFIG_RMP_RETRANS_TABLE_SIZE; i++)
    {
        ReliableMessageContext * rc = mRetransTable[i].rc;
        if (rc)
        {
            // When do we need to next wake up for ReliableMessageProtocol retransmit?
            if (mRetransTable[i].nextRetransTimeTick < nextWakeTimeTick)
            {
                nextWakeTimeTick = mRetransTable[i].nextRetransTimeTick;
                foundWake        = true;
#if defined(RMP_TICKLESS_DEBUG)
                ChipLogProgress(ExchangeManager, "ReliableMessageManager::StartTimer RetransTime %u", nextWakeTimeTick);
#endif
            }
        }
    }

    if (foundWake)
    {
        // Set timer for next tick boundary - subtract the elapsed time from the current tick
        System::Timer::Epoch timerExpiryEpoch = (nextWakeTimeTick << mTimerIntervalShift) + mTimeStampBase;

#if defined(RMP_TICKLESS_DEBUG)
        ChipLogProgress(ExchangeManager, "ReliableMessageManager::StartTimer wake at %" PRIu64 " ms (%" PRIu64 " %" PRIu64 ")",
                        timerExpiryEpoch, nextWakeTimeTick, mTimeStampBase);
#endif
        if (timerExpiryEpoch != mCurrentTimerExpiry)
        {
            // If the tick boundary has expired in the past (delayed processing of event due to other system activity),
            // expire the timer immediately
            uint64_t now           = System::Timer::GetCurrentEpoch();
            uint64_t timerArmValue = (timerExpiryEpoch > now) ? timerExpiryEpoch - now : 0;

#if defined(RMP_TICKLESS_DEBUG)
            ChipLogProgress(ExchangeManager, "ReliableMessageManager::StartTimer set timer for %" PRIu64, timerArmValue);
#endif
            StopTimer();
            res = mSystemLayer->StartTimer((uint32_t) timerArmValue, Timeout, this);

            VerifyOrDieWithMsg(res == CHIP_NO_ERROR, ExchangeManager, "Cannot start ReliableMessageManager::Timeout\n");
            mCurrentTimerExpiry = timerExpiryEpoch;
#if defined(RMP_TICKLESS_DEBUG)
        }
        else
        {
            ChipLogProgress(ExchangeManager, "ReliableMessageManager::StartTimer timer already set for %" PRIu64, timerExpiryEpoch);
#endif
        }
    }
    else
    {
#if defined(RMP_TICKLESS_DEBUG)
        ChipLogProgress(ExchangeManager, "Not setting ReliableMessageProtocol timeout at %" PRIu64,
                        System::Timer::GetCurrentEpoch());
#endif
        StopTimer();
    }

    TicklessDebugDumpRetransTable("ReliableMessageManager::StartTimer Dumping mRetransTable entries after setting wakeup times");
}

void ReliableMessageManager::StopTimer()
{
    mSystemLayer->CancelTimer(Timeout, this);
}

int ReliableMessageManager::TestGetCountRetransTable()
{
    int count = 0;
    for (int i = 0; i < CHIP_CONFIG_RMP_RETRANS_TABLE_SIZE; i++)
    {
        ReliableMessageContext * rc = mRetransTable[i].rc;
        if (rc)
            count++;
    }
    return count;
}

} // namespace Messaging
} // namespace chip
