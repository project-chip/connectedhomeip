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

#include <messaging/CHIPMessageLayerPrivate.h>
#include <messaging/CHIPMessageLayerUtils.h>
#include <messaging/CHIPReliableMessageContext.h>
#include <messaging/CHIPReliableMessageManager.h>
#include <support/CHIPFaultInjection.h>
#include <support/CHIPLogging.h>
#include <support/CodeUtils.h>
#include <support/BitFlags.h>

namespace chip {

CHIPReliableMessageManager::RetransTableEntry::RetransTableEntry() : rc(nullptr), msgBuf(nullptr), msgId(0), msgSendFlags(0), nextRetransTime(0), sendCount(0) {}

CHIPReliableMessageManager::CHIPReliableMessageManager() : mRMPConfig(gDefaultRMPConfig), mRMPTimeStampBase(System::Timer::GetCurrentEpoch()), mRMPCurrentTimerExpiry(0), mRMPTimerInterval(CHIP_CONFIG_RMP_TIMER_DEFAULT_PERIOD) {}

void CHIPReliableMessageManager::RMPProcessDDMessage(uint32_t PauseTimeMillis, uint64_t DelayedNodeId)
{
    // Expire any virtual ticks that have expired so all wakeup sources reflect the current time
    RMPExpireTicks();

    // Go through the retrans table entries for that node and adjust the timer.
    for (int i = 0; i < CHIP_CONFIG_RMP_RETRANS_TABLE_SIZE; i++)
    {
        // Exchcontext is the sentinel object to ascertain validity of the element
        if (RetransTable[i].rc)
        {
            // Adjust the retrans timer value if Delayed Node identifier matches Peer in ExchangeContext
            if (RetransTable[i].rc->mDelegate.IsNode(DelayedNodeId))
            {
                // Paustime is specified in milliseconds; Update retrans values
                RetransTable[i].nextRetransTime = static_cast<uint16_t>(RetransTable[i].nextRetransTime + (PauseTimeMillis / mRMPTimerInterval));
                RetransTable[i].rc->mDelegate.OnDDRcvd(PauseTimeMillis);
            } // DelayedNodeId == PeerNodeId
        }     // exchContext
    }         // for loop in table entry

    // Schedule next physical wakeup
    RMPStartTimer();
}

/**
 * Return a tick counter value given a time difference and a tick interval.
 * The difference in time is not expected to exceed (2^32 - 1) within the
 * scope of two timestamp comparisons in RMP and, thus, it makes sense to cast
 * the time delta to uint32_t. This also avoids invocation of 64 bit divisions
 * in constrained platforms that do not support them.
 *
 * @param[in]  newTime        Timestamp value of in milliseconds.
 *
 * @return Tick count for the time delta.
 */
uint32_t CHIPReliableMessageManager::GetTickCounterFromTimeDelta(uint64_t newTime)
{
    // Note on math: we have a utility function that will compute U64 var / U32
    // compile-time const => U32.  At the moment, we are leaving
    // mRMPTimerInterval as a member variable in ChipExchangeManager, however,
    // given its current usage, it could be replaced by a compile time const.
    // Should we make that change, I would recommend making the timeDelta a u64,
    // and replacing the plain 32-bit division below with the utility function.
    // Note that the 32bit timeDelta overflows at around 46 days; pursuing the
    // above code strategy would extend that overflow by a factor if 200 given
    // the default mRMPPTimerInterval.  If and when such change is made, please
    // update the comment around line 1426.
    uint32_t timeDelta = static_cast<uint32_t>(newTime - mRMPTimeStampBase);

    return (timeDelta / mRMPTimerInterval);
}

#if defined(RMP_TICKLESS_DEBUG)
void CHIPReliableMessageManager::TicklessDebugDumpRetransTable(const char * log)
{
    ChipLogProgress(ExchangeManager, log);

    for (int i = 0; i < CHIP_CONFIG_RMP_RETRANS_TABLE_SIZE; i++)
    {
        if (RetransTable[i].rc)
        {
            ChipLogProgress(ExchangeManager, "EC:%04" PRIX16 " MsgId:%08" PRIX32 " NextRetransTimeCtr:%04" PRIX16,
                            RetransTable[i].rc, RetransTable[i].msgId, RetransTable[i].nextRetransTime);
        }
    }
}
#else
void CHIPReliableMessageManager::TicklessDebugDumpRetransTable(const char * log)
{
    return;
}
#endif // RMP_TICKLESS_DEBUG

/**
 * Iterate through active exchange contexts and retrans table entries.
 * If an action needs to be triggered by RMP time facilities, execute
 * that action.
 *
 */
void CHIPReliableMessageManager::RMPExecuteActions()
{
#if defined(RMP_TICKLESS_DEBUG)
    ChipLogProgress(ExchangeManager, "RMPExecuteActions");
#endif

    ExecuteForAllExchange([] (CHIPReliableMessageContext *rc) {
        if (rc->IsAckPending())
        {
            if (0 == rc->mRMPNextAckTime)
            {
#if defined(RMP_TICKLESS_DEBUG)
                ChipLogProgress(ExchangeManager, "RMPExecuteActions sending ACK");
#endif
                // Send the Ack in a Common::Null message
                rc->SendCommonNullMessage();
                rc->SetAckPending(false);
            }
        }
    });

    TicklessDebugDumpRetransTable("RMPExecuteActions Dumping RetransTable entries before processing");

    // Retransmit / cancel anything in the retrans table whose retrans timeout
    // has expired
    for (int i = 0; i < CHIP_CONFIG_RMP_RETRANS_TABLE_SIZE; i++)
    {
        CHIPReliableMessageContext* rc = RetransTable[i].rc;
        if (rc)
        {
            CHIP_ERROR err = CHIP_NO_ERROR;

            if (0 == RetransTable[i].nextRetransTime)
            {
                uint8_t sendCount = RetransTable[i].sendCount;

                if (sendCount > rc->mRMPConfig.mMaxRetrans)
                {
                    err = CHIP_ERROR_MESSAGE_NOT_ACKNOWLEDGED;

                    ChipLogError(ExchangeManager,
                                 "Failed to Send CHIP MsgId:%08" PRIX32 " sendCount: %" PRIu8 " max retries: %" PRIu8,
                                 RetransTable[i].msgId, sendCount, rc->mRMPConfig.mMaxRetrans);

                    // Remove from Table
                    ClearRetransmitTable(RetransTable[i]);
                }

                if (err == CHIP_NO_ERROR)
                {
                    // Resend from Table (if the operation fails, the entry is cleared)
                    err = SendFromRetransTable(&(RetransTable[i]));
                }

                if (err == CHIP_NO_ERROR)
                {
                    // If the retransmission was successful, update the passive timer
                    RetransTable[i].nextRetransTime = static_cast<uint16_t>(rc->GetCurrentRetransmitTimeout() / mRMPTimerInterval);
#if defined(DEBUG)
                    ChipLogProgress(ExchangeManager, "Retransmit MsgId:%08" PRIX32 " Send Cnt %d", RetransTable[i].msgId,
                                    RetransTable[i].sendCount);
#endif
                }

                if (err != CHIP_NO_ERROR)
                {
                    rc->mDelegate.OnSendError(err);
                }
            } // nextRetransTime = 0
        }
    }

    TicklessDebugDumpRetransTable("RMPExecuteActions Dumping RetransTable entries after processing");
}

static void TickProceed(uint16_t& time, uint16_t ticks)
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
 * Calculate number of virtual RMP ticks that have expired since we last
 * called this function. Iterate through active exchange contexts and
 * retrans table entries, subtracting expired virtual ticks to synchronize
 * wakeup times with the current system time. Do not perform any actions
 * beyond updating tick counts, actions will be performed by the physical
 * RMP timer tick expiry.
 *
 */
void CHIPReliableMessageManager::RMPExpireTicks()
{
    uint64_t now = System::Timer::GetCurrentEpoch();

    // Number of full ticks elapsed since last timer processing.  We always round down
    // to the previous tick.  If we are between tick boundaries, the extra time since the
    // last virtual tick is not accounted for here (it will be accounted for when resetting
    // the RMP timer)
    uint16_t deltaTicks = static_cast<uint16_t>(GetTickCounterFromTimeDelta(now));

    // Note on math involving deltaTicks: in the code below, deltaTicks, a
    // 32-bit value, is being subtracted from 16-bit expiration times.  In each
    // case, we compare the expiration time prior to subtraction to guard
    // against underflow.

#if defined(RMP_TICKLESS_DEBUG)
    ChipLogProgress(ExchangeManager, "RMPExpireTicks at %" PRIu64 ", %" PRIu64 ", %u", now, mRMPTimeStampBase, deltaTicks);
#endif

    ExecuteForAllExchange([deltaTicks] (CHIPReliableMessageContext *rc) {
        if (rc->IsAckPending())
        {
            // Decrement counter of Ack timestamp by the elapsed timer ticks
            TickProceed(rc->mRMPNextAckTime, deltaTicks);
#if defined(RMP_TICKLESS_DEBUG)
            ChipLogProgress(ExchangeManager, "RMPExpireTicks set mRMPNextAckTime to %u", rc->mRMPNextAckTime);
#endif
        }
    });

    // Process Throttle Time
    // Check Throttle timeout stored in EC to set/unset Throttle flag
    for (int i = 0; i < CHIP_CONFIG_RMP_RETRANS_TABLE_SIZE; i++)
    {
        CHIPReliableMessageContext *rc = RetransTable[i].rc;
        if (rc)
        {
            // Process Retransmit Table
            // Decrement Throttle timeout by elapsed timeticks
            TickProceed(rc->mRMPThrottleTimeout, deltaTicks);
#if defined(RMP_TICKLESS_DEBUG)
            ChipLogProgress(ExchangeManager, "RMPExpireTicks set mRMPThrottleTimeout to %u", RetransTable[i].nextRetransTime);
#endif

            // Decrement Retransmit timeout by elapsed timeticks
            TickProceed(RetransTable[i].nextRetransTime, deltaTicks);
#if defined(RMP_TICKLESS_DEBUG)
            ChipLogProgress(ExchangeManager, "RMPExpireTicks set nextRetransTime to %u", RetransTable[i].nextRetransTime);
#endif
        } // rc entry is allocated
    }

    // Re-Adjust the base time stamp to the most recent tick boundary

    // Note on math: we cast deltaTicks to a 64bit value to ensure that that we
    // produce a full 64 bit product.  At the moment this is a bit of a moot
    // conversion: right now, the math in GetTickCounterFromTimeDelta ensures
    // that the deltaTicks * mRMPTimerTick fits in 32bits.  However, I'm
    // leaving the math in this form, because I'm leaving the door open to
    // refactoring the division in GetTickCounterFromTimeDelta to use our
    // specialized utility function that computes U64 var/ U32 compile-time
    // const ==> U32
    mRMPTimeStampBase += static_cast<uint64_t>(deltaTicks) * mRMPTimerInterval;
#if defined(RMP_TICKLESS_DEBUG)
    ChipLogProgress(ExchangeManager, "RMPExpireTicks mRMPTimeStampBase to %" PRIu64, mRMPTimeStampBase);
#endif
}

/**
 * Handle physical wakeup of system due to RMP wakeup.
 *
 */
void CHIPReliableMessageManager::RMPTimeout(System::Layer * aSystemLayer, void * aAppState, System::Error aError)
{
    CHIPReliableMessageManager *manager = reinterpret_cast<CHIPReliableMessageManager*>(aAppState);

    VerifyOrDie((aSystemLayer != nullptr) && (manager != nullptr));

#if defined(RMP_TICKLESS_DEBUG)
    ChipLogProgress(ExchangeManager, "RMPTimeout\n");
#endif

    // Make sure all tick counts are sync'd to the current time
    manager->RMPExpireTicks();

    // Execute any actions that are due this tick
    manager->RMPExecuteActions();

    // Calculate next physical wakeup
    manager->RMPStartTimer();
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
CHIP_ERROR CHIPReliableMessageManager::AddToRetransTable(CHIPReliableMessageContext* rc, System::PacketBuffer * msgBuf, uint32_t messageId, uint16_t msgSendFlags, RetransTableEntry ** rEntry)
{
    bool added     = false;
    CHIP_ERROR err = CHIP_NO_ERROR;

    for (int i = 0; i < CHIP_CONFIG_RMP_RETRANS_TABLE_SIZE; i++)
    {
        // Check the exchContext pointer for finding an empty slot in Table
        if (!RetransTable[i].rc)
        {
            // Expire any virtual ticks that have expired so all wakeup sources reflect the current time
            RMPExpireTicks();

            RetransTable[i].rc              = rc;
            RetransTable[i].msgId           = messageId;
            RetransTable[i].msgBuf          = msgBuf;
            RetransTable[i].msgSendFlags    = msgSendFlags;
            RetransTable[i].sendCount       = 0;
            RetransTable[i].nextRetransTime = static_cast<uint16_t>(GetTickCounterFromTimeDelta(rc->GetCurrentRetransmitTimeout() + System::Timer::GetCurrentEpoch()));

            *rEntry                 = &RetransTable[i];
            // Increment the reference count
            rc->Retain();
            added = true;

            // Check if the timer needs to be started and start it.
            RMPStartTimer();
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

void CHIPReliableMessageManager::RMPPauseRetransTable(CHIPReliableMessageContext* rc, uint32_t PauseTimeMillis)
{
    for (int i = 0; i < CHIP_CONFIG_RMP_RETRANS_TABLE_SIZE; i++)
    {
        if (RetransTable[i].rc == rc)
        {
            RetransTable[i].nextRetransTime = static_cast<uint16_t>(RetransTable[i].nextRetransTime + PauseTimeMillis / mRMPTimerInterval);
            break;
        }
    }
}

void CHIPReliableMessageManager::RMPResumeRetransTable(CHIPReliableMessageContext* rc)
{
    for (int i = 0; i < CHIP_CONFIG_RMP_RETRANS_TABLE_SIZE; i++)
    {
        if (RetransTable[i].rc == rc)
        {
            RetransTable[i].nextRetransTime = 0;
            break;
        }
    }
}

bool CHIPReliableMessageManager::RMPCheckAndRemRetransTable(CHIPReliableMessageContext* rc, uint32_t ackMsgId)
{
    bool res = false;

    for (int i = 0; i < CHIP_CONFIG_RMP_RETRANS_TABLE_SIZE; i++)
    {
        if ((RetransTable[i].rc == rc) && RetransTable[i].msgId == ackMsgId)
        {
            // Clear the entry from the retransmision table.
            ClearRetransmitTable(RetransTable[i]);

#if defined(DEBUG)
            ChipLogProgress(ExchangeManager, "Rxd Ack; Removing MsgId:%08" PRIX32 " from Retrans Table", ackMsgId);
#endif
            res = true;
            break;
        }

        continue;
    }

    return res;
}

/**
 *  Send the specified entry from the retransmission table.
 *
 *  @param[in]    entry                A pointer to a retransmission table entry object that needs to be sent.
 *
 *  @return  #CHIP_NO_ERROR On success, else corresponding CHIP_ERROR returned from SendMessage.
 *
 */
CHIP_ERROR CHIPReliableMessageManager::SendFromRetransTable(RetransTableEntry * entry)
{
    CHIP_ERROR err        = CHIP_NO_ERROR;
    CHIPReliableMessageContext* rc  = entry->rc;

    // To trigger a call to OnSendError, set the number of transmissions so
    // that the next call to RMPExecuteActions will abort this entry,
    // restart the timer immediately, and ExitNow.

    CHIP_FAULT_INJECT(FaultInjection::kFault_RMPSendError, entry->sendCount = static_cast<uint8_t>(rc->mRMPConfig.mMaxRetrans + 1); entry->nextRetransTime = 0; RMPStartTimer(); ExitNow());

    if (rc)
    {
        // Locally store the start and length;
        uint8_t *p   = entry->msgBuf->Start();
        uint32_t len = entry->msgBuf->DataLength();

        // Send the message through
        uint16_t msgSendFlags = entry->msgSendFlags;
        SetFlag(msgSendFlags, kChipMessageFlag_RetainBuffer);
        err = rc->mDelegate.SendMessage(entry->msgBuf, msgSendFlags);

        // Reset the msgBuf start pointer and data length after sending
        entry->msgBuf->SetStart(p);
        entry->msgBuf->SetDataLength(static_cast<uint16_t>(len));

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
    if (ChipMessageLayerUtils::IsSendErrorNonCritical(err))
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
void CHIPReliableMessageManager::ClearRetransmitTable(CHIPReliableMessageContext* rc)
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
void CHIPReliableMessageManager::ClearRetransmitTable(RetransTableEntry & rEntry)
{
    if (rEntry.rc)
    {
        // Expire any virtual ticks that have expired so all wakeup sources reflect the current time
        RMPExpireTicks();

        rEntry.rc->Release();
        rEntry.rc = nullptr;

        if (rEntry.msgBuf)
        {
            System::PacketBuffer::Free(rEntry.msgBuf);
            rEntry.msgBuf = nullptr;
        }

        // Clear all other fields
        rEntry = RetransTableEntry();

        // Schedule next physical wakeup
        RMPStartTimer();
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
void CHIPReliableMessageManager::FailRetransmitTableEntries(CHIPReliableMessageContext * rc, CHIP_ERROR err)
{
    for (int i = 0; i < CHIP_CONFIG_RMP_RETRANS_TABLE_SIZE; i++)
    {
        if (RetransTable[i].rc == rc)
        {
            // Remove the entry from the retransmission table.
            ClearRetransmitTable(RetransTable[i]);

            // Application callback OnSendError.
            rc->mDelegate.OnSendError(err);
        }
    }
}

/**
 * Iterate through active exchange contexts and retrans table entries.
 * Determine how many RMP ticks we need to sleep before we need to physically
 * wake the CPU to perform an action.  Set a timer to go off when we
 * next need to wake the system.
 *
 */
void CHIPReliableMessageManager::RMPStartTimer()
{
    CHIP_ERROR res        = CHIP_NO_ERROR;
    uint32_t nextWakeTime = UINT32_MAX;
    bool foundWake        = false;

    // When do we need to next wake up to send an ACK?

    ExecuteForAllExchange([&nextWakeTime, &foundWake] (CHIPReliableMessageContext *rc) {
        if (rc->IsAckPending() && rc->mRMPNextAckTime < nextWakeTime)
        {
            nextWakeTime = rc->mRMPNextAckTime;
            foundWake    = true;
#if defined(RMP_TICKLESS_DEBUG)
            ChipLogProgress(ExchangeManager, "RMPStartTimer next ACK time %u", nextWakeTime);
#endif
        }
    });

    for (int i = 0; i < CHIP_CONFIG_RMP_RETRANS_TABLE_SIZE; i++)
    {
        CHIPReliableMessageContext *rc = RetransTable[i].rc;
        if (rc)
        {
            // When do we need to next wake up for throttle retransmission?
            if (rc->mRMPThrottleTimeout != 0 && rc->mRMPThrottleTimeout < nextWakeTime)
            {
                nextWakeTime = rc->mRMPThrottleTimeout;
                foundWake    = true;
#if defined(RMP_TICKLESS_DEBUG)
                ChipLogProgress(ExchangeManager, "RMPStartTimer throttle timeout %u", nextWakeTime);
#endif
            }

            // When do we need to next wake up for RMP retransmit?
            if (RetransTable[i].nextRetransTime < nextWakeTime)
            {
                nextWakeTime = RetransTable[i].nextRetransTime;
                foundWake    = true;
#if defined(RMP_TICKLESS_DEBUG)
                ChipLogProgress(ExchangeManager, "RMPStartTimer RetransTime %u", nextWakeTime);
#endif
            }
        }
    }

    if (foundWake)
    {
        // Set timer for next tick boundary - subtract the elapsed time from the current tick
        System::Timer::Epoch currentTime      = System::Timer::GetCurrentEpoch();
        int32_t timerArmValue                 = static_cast<uint32_t>(nextWakeTime * mRMPTimerInterval - (currentTime - mRMPTimeStampBase));
        System::Timer::Epoch timerExpiryEpoch = currentTime + timerArmValue;

#if defined(RMP_TICKLESS_DEBUG)
        ChipLogProgress(ExchangeManager, "RMPStartTimer wake in %d ms (%" PRIu64 " %u %" PRIu64 " %" PRIu64 ")", timerArmValue,
                        timerExpiryEpoch, nextWakeTime, currentTime, mRMPTimeStampBase);
#endif
        if (timerExpiryEpoch != mRMPCurrentTimerExpiry)
        {
            // If the tick boundary has expired in the past (delayed processing of event due to other system activity),
            // expire the timer immediately
            if (timerArmValue < 0)
            {
                timerArmValue = 0;
            }

#if defined(RMP_TICKLESS_DEBUG)
            ChipLogProgress(ExchangeManager, "RMPStartTimer set timer for %d %" PRIu64, timerArmValue, timerExpiryEpoch);
#endif
            RMPStopTimer();
            res = mSystemLayer->StartTimer((uint32_t) timerArmValue, RMPTimeout, this);

            VerifyOrDieWithMsg(res == CHIP_NO_ERROR, ExchangeManager, "Cannot start RMPTimeout\n");
            mRMPCurrentTimerExpiry = timerExpiryEpoch;
#if defined(RMP_TICKLESS_DEBUG)
        }
        else
        {
            ChipLogProgress(ExchangeManager, "RMPStartTimer timer already set for %" PRIu64, timerExpiryEpoch);
#endif
        }
    }
    else
    {
#if defined(RMP_TICKLESS_DEBUG)
        ChipLogProgress(ExchangeManager, "Not setting RMP timeout at %" PRIu64, System::Timer::GetCurrentEpoch());
#endif
        RMPStopTimer();
    }

    TicklessDebugDumpRetransTable("RMPStartTimer Dumping RetransTable entries after setting wakeup times");
}

void CHIPReliableMessageManager::RMPStopTimer()
{
    mSystemLayer->CancelTimer(RMPTimeout, this);
}

} // namespace chip
