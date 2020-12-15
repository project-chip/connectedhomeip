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

ReliableMessageManager::RetransTableEntry::RetransTableEntry() :
    rc(nullptr), msgBuf(nullptr), msgId(0), msgSendFlags(0), nextRetransTimeTick(0), sendCount(0)
{}

ReliableMessageManager::ReliableMessageManager() :
    mTimeStampBase(System::Timer::GetCurrentEpoch()), mCurrentTimerExpiry(0),
    mTimerIntervalShift(CHIP_CONFIG_RMP_TIMER_DEFAULT_PERIOD_SHIFT)
{}

ReliableMessageManager::~ReliableMessageManager() {}

void ReliableMessageManager::ProcessDelayedDeliveryMessage(ReliableMessageContext * rc, uint32_t PauseTimeMillis)
{
    // Expire any virtual ticks that have expired so all wakeup sources reflect the current time
    ExpireTicks();

    // Go through the retrans table entries for that node and adjust the timer.
    for (int i = 0; i < CHIP_CONFIG_RMP_RETRANS_TABLE_SIZE; i++)
    {
        // Exchcontext is the sentinel object to ascertain validity of the element
        if (RetransTable[i].rc && RetransTable[i].rc == rc)
        {
            // Paustime is specified in milliseconds; Update retrans values
            RetransTable[i].nextRetransTimeTick =
                static_cast<uint16_t>(RetransTable[i].nextRetransTimeTick + (PauseTimeMillis >> mTimerIntervalShift));
        } // exchContext
    }     // for loop in table entry

    // Schedule next physical wakeup
    StartTimer();
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
        if (RetransTable[i].rc)
        {
            ChipLogProgress(ExchangeManager, "EC:%04" PRIX16 " MsgId:%08" PRIX32 " NextRetransTimeCtr:%04" PRIX16,
                            RetransTable[i].rc, RetransTable[i].msgId, RetransTable[i].nextRetransTimeTick);
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
                // Send the Ack in a Common::Null message
                rc->SendCommonNullMessage();
                rc->SetAckPending(false);
            }
        }
    });

    TicklessDebugDumpRetransTable("ReliableMessageManager::ExecuteActions Dumping RetransTable entries before processing");

    // Retransmit / cancel anything in the retrans table whose retrans timeout
    // has expired
    for (int i = 0; i < CHIP_CONFIG_RMP_RETRANS_TABLE_SIZE; i++)
    {
        ReliableMessageContext * rc = RetransTable[i].rc;
        CHIP_ERROR err              = CHIP_NO_ERROR;

        if (!rc || RetransTable[i].nextRetransTimeTick != 0)
            continue;

        uint8_t sendCount = RetransTable[i].sendCount;

        if (sendCount == rc->mConfig.mMaxRetrans)
        {
            err = CHIP_ERROR_MESSAGE_NOT_ACKNOWLEDGED;

            ChipLogError(ExchangeManager, "Failed to Send CHIP MsgId:%08" PRIX32 " sendCount: %" PRIu8 " max retries: %" PRIu8,
                         RetransTable[i].msgId, sendCount, rc->mConfig.mMaxRetrans);

            // Remove from Table
            ClearRetransmitTable(RetransTable[i]);
        }

        // Resend from Table (if the operation fails, the entry is cleared)
        if (err == CHIP_NO_ERROR)
            err = SendFromRetransTable(&(RetransTable[i]));

        if (err == CHIP_NO_ERROR)
        {
            // If the retransmission was successful, update the passive timer
            RetransTable[i].nextRetransTimeTick = static_cast<uint16_t>(rc->GetCurrentRetransmitTimeoutTick());
#if !defined(NDEBUG)
            ChipLogProgress(ExchangeManager, "Retransmit MsgId:%08" PRIX32 " Send Cnt %d", RetransTable[i].msgId,
                            RetransTable[i].sendCount);
#endif
        }

        if (err != CHIP_NO_ERROR)
            rc->mDelegate->OnSendError(err);
    }

    TicklessDebugDumpRetransTable("ReliableMessageManager::ExecuteActions Dumping RetransTable entries after processing");
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

    // Process Throttle Time
    // Check Throttle timeout stored in EC to set/unset Throttle flag
    for (int i = 0; i < CHIP_CONFIG_RMP_RETRANS_TABLE_SIZE; i++)
    {
        ReliableMessageContext * rc = RetransTable[i].rc;
        if (rc)
        {
            // Process Retransmit Table
            // Decrement Throttle timeout by elapsed timeticks
            TickProceed(rc->mThrottleTimeoutTick, deltaTicks);
#if defined(RMP_TICKLESS_DEBUG)
            ChipLogProgress(ExchangeManager, "ReliableMessageManager::ExpireTicks set mThrottleTimeoutTick to %u",
                            RetransTable[i].nextRetransTimeTick);
#endif

            // Decrement Retransmit timeout by elapsed timeticks
            TickProceed(RetransTable[i].nextRetransTimeTick, deltaTicks);
#if defined(RMP_TICKLESS_DEBUG)
            ChipLogProgress(ExchangeManager, "ReliableMessageManager::ExpireTicks set nextRetransTimeTick to %u",
                            RetransTable[i].nextRetransTimeTick);
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
 *  @param[in]    msgBuf    A pointer to the message buffer holding the CHIP message to be retransmitted.
 *
 *  @param[in]    messageId The message identifier of the stored CHIP message.
 *
 *  @param[out]   rEntry    A pointer to a pointer of a retransmission table entry added into the table.
 *
 *  @retval  #CHIP_ERROR_RETRANS_TABLE_FULL If there is no empty slot left in the table for addition.
 *  @retval  #CHIP_NO_ERROR On success.
 *
 */
CHIP_ERROR ReliableMessageManager::AddToRetransTable(ReliableMessageContext * rc, System::PacketBufferHandle msgBuf,
                                                     uint32_t messageId, uint16_t msgSendFlags, RetransTableEntry ** rEntry)
{
    bool added     = false;
    CHIP_ERROR err = CHIP_NO_ERROR;

    for (int i = 0; i < CHIP_CONFIG_RMP_RETRANS_TABLE_SIZE; i++)
    {
        // Check the exchContext pointer for finding an empty slot in Table
        if (!RetransTable[i].rc)
        {
            // Expire any virtual ticks that have expired so all wakeup sources reflect the current time
            ExpireTicks();

            RetransTable[i].rc                  = rc;
            RetransTable[i].msgId               = messageId;
            RetransTable[i].msgBuf              = std::move(msgBuf);
            RetransTable[i].msgSendFlags        = msgSendFlags;
            RetransTable[i].sendCount           = 0;
            RetransTable[i].nextRetransTimeTick = static_cast<uint16_t>(
                rc->GetCurrentRetransmitTimeoutTick() + GetTickCounterFromTimeDelta(System::Timer::GetCurrentEpoch()));

            *rEntry = &RetransTable[i];
            // Increment the reference count
            rc->Retain();
            added = true;

            // Check if the timer needs to be started and start it.
            StartTimer();
            break;
        }
    }

    if (!added)
    {
        ChipLogError(ExchangeManager, "RetransTable Already Full");
        err = CHIP_ERROR_RETRANS_TABLE_FULL;
    }

    return err;
}

void ReliableMessageManager::PauseRetransTable(ReliableMessageContext * rc, uint32_t PauseTimeMillis)
{
    for (int i = 0; i < CHIP_CONFIG_RMP_RETRANS_TABLE_SIZE; i++)
    {
        if (RetransTable[i].rc == rc)
        {
            RetransTable[i].nextRetransTimeTick =
                static_cast<uint16_t>(RetransTable[i].nextRetransTimeTick + (PauseTimeMillis >> mTimerIntervalShift));
            break;
        }
    }
}

void ReliableMessageManager::ResumeRetransTable(ReliableMessageContext * rc)
{
    for (int i = 0; i < CHIP_CONFIG_RMP_RETRANS_TABLE_SIZE; i++)
    {
        if (RetransTable[i].rc == rc)
        {
            RetransTable[i].nextRetransTimeTick = 0;
            break;
        }
    }
}

bool ReliableMessageManager::CheckAndRemRetransTable(ReliableMessageContext * rc, uint32_t ackMsgId)
{
    for (int i = 0; i < CHIP_CONFIG_RMP_RETRANS_TABLE_SIZE; i++)
    {
        if ((RetransTable[i].rc == rc) && RetransTable[i].msgId == ackMsgId)
        {
            // Clear the entry from the retransmision table.
            ClearRetransmitTable(RetransTable[i]);

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

    // To trigger a call to OnSendError, set the number of transmissions so
    // that the next call to ExecuteActions will abort this entry,
    // restart the timer immediately, and ExitNow.

    CHIP_FAULT_INJECT(FaultInjection::kFault_RMPSendError, entry->sendCount = static_cast<uint8_t>(rc->mConfig.mMaxRetrans + 1);
                      entry->nextRetransTimeTick = 0; StartTimer(); ExitNow());

    if (rc)
    {
        // Locally store the start and length;
        uint8_t * p  = entry->msgBuf->Start();
        uint16_t len = entry->msgBuf->DataLength();

        // Send the message through
        uint16_t msgSendFlags = entry->msgSendFlags;
        err                   = SendMessage(rc, entry->msgBuf.Retain(), msgSendFlags);

        // Reset the msgBuf start pointer and data length after sending
        entry->msgBuf->SetStart(p);
        entry->msgBuf->SetDataLength(len);

        // Update the counters
        entry->sendCount++;
    }
    else
    {
        ChipLogError(ExchangeManager, "Table entry invalid");
    }

    VerifyOrExit(err != CHIP_NO_ERROR, err = CHIP_NO_ERROR);

    // Any error generated during initial sending is evaluated for criticality which would
    // qualify it to be reportable back to the caller. If it is non-critical then
    // err is set to CHIP_NO_ERROR.
    if (IsSendErrorNonCritical(err))
    {
        ChipLogError(ExchangeManager, "Non-crit err %ld sending CHIP MsgId:%08" PRIX32 " from retrans table", long(err),
                     entry->msgId);
        err = CHIP_NO_ERROR;
    }
    else
    {
        // Remove from table
        ChipLogError(ExchangeManager, "Crit-err %ld when sending CHIP MsgId:%08" PRIX32 ", send tries: %d", long(err), entry->msgId,
                     entry->sendCount);

        ClearRetransmitTable(*entry);
    }

exit:
    return err;
}

/**
 *  Clear entries matching a specified ExchangeContext.
 *
 *  @param[in]    rc    A pointer to the ExchangeContext object.
 *
 */
void ReliableMessageManager::ClearRetransmitTable(ReliableMessageContext * rc)
{
    for (int i = 0; i < CHIP_CONFIG_RMP_RETRANS_TABLE_SIZE; i++)
    {
        if (RetransTable[i].rc == rc)
        {
            // Clear the retransmit table entry.
            ClearRetransmitTable(RetransTable[i]);
        }
    }
}

/**
 *  Clear an entry in the retransmission table.
 *
 *  @param[in]    rEntry   A reference to the RetransTableEntry object.
 *
 */
void ReliableMessageManager::ClearRetransmitTable(RetransTableEntry & rEntry)
{
    if (rEntry.rc)
    {
        // Expire any virtual ticks that have expired so all wakeup sources reflect the current time
        ExpireTicks();

        rEntry.rc->Release();
        rEntry.rc     = nullptr;
        rEntry.msgBuf = nullptr;

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
void ReliableMessageManager::FailRetransmitTableEntries(ReliableMessageContext * rc, CHIP_ERROR err)
{
    for (int i = 0; i < CHIP_CONFIG_RMP_RETRANS_TABLE_SIZE; i++)
    {
        if (RetransTable[i].rc == rc)
        {
            // Remove the entry from the retransmission table.
            ClearRetransmitTable(RetransTable[i]);

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
        ReliableMessageContext * rc = RetransTable[i].rc;
        if (rc)
        {
            // When do we need to next wake up for throttle retransmission?
            if (rc->mThrottleTimeoutTick != 0 && rc->mThrottleTimeoutTick < nextWakeTimeTick)
            {
                nextWakeTimeTick = rc->mThrottleTimeoutTick;
                foundWake        = true;
#if defined(RMP_TICKLESS_DEBUG)
                ChipLogProgress(ExchangeManager, "ReliableMessageManager::StartTimer throttle timeout %u", nextWakeTimeTick);
#endif
            }

            // When do we need to next wake up for ReliableMessageProtocol retransmit?
            if (RetransTable[i].nextRetransTimeTick < nextWakeTimeTick)
            {
                nextWakeTimeTick = RetransTable[i].nextRetransTimeTick;
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

    TicklessDebugDumpRetransTable("ReliableMessageManager::StartTimer Dumping RetransTable entries after setting wakeup times");
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
        ReliableMessageContext * rc = RetransTable[i].rc;
        if (rc)
            count++;
    }
    return count;
}

} // namespace Messaging
} // namespace chip
