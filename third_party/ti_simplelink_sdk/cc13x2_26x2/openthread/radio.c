/******************************************************************************

 @file radio.c

 @brief TIRTOS platform specific radio functions for OpenThread

 Group: CMCU, LPC
 Target Device: cc13x2_26x2

 ******************************************************************************
 
 Copyright (c) 2017-2020, Texas Instruments Incorporated
 All rights reserved.

 Redistribution and use in source and binary forms, with or without
 modification, are permitted provided that the following conditions
 are met:

 *  Redistributions of source code must retain the above copyright
    notice, this list of conditions and the following disclaimer.

 *  Redistributions in binary form must reproduce the above copyright
    notice, this list of conditions and the following disclaimer in the
    documentation and/or other materials provided with the distribution.

 *  Neither the name of Texas Instruments Incorporated nor the names of
    its contributors may be used to endorse or promote products derived
    from this software without specific prior written permission.

 THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

 ******************************************************************************
 
 
 *****************************************************************************/

#include <openthread/config.h>

#include "radio.h"

#include <stddef.h>

#include <openthread/diag.h>
#include <openthread/platform/alarm-milli.h>
#include <openthread/platform/diag.h>
#include <openthread/platform/radio.h>
#include <openthread/platform/entropy.h>
#include <utils/code_utils.h>

#include <ti/devices/DeviceFamily.h>
#include DeviceFamily_constructPath(driverlib/aon_rtc.h)
#include DeviceFamily_constructPath(driverlib/rf_data_entry.h)
#include DeviceFamily_constructPath(driverlib/rf_common_cmd.h)
#include DeviceFamily_constructPath(driverlib/rf_mailbox.h)
#include DeviceFamily_constructPath(driverlib/rf_ieee_cmd.h)
#include DeviceFamily_constructPath(driverlib/rf_ieee_mailbox.h)
#include DeviceFamily_constructPath(inc/hw_ccfg.h)
#include DeviceFamily_constructPath(inc/hw_fcfg1.h)

#include <ti/drivers/rf/RF.h>

#include <ti_radio_config.h>

#include <FreeRTOS.h>
#include <queue.h>

#include "system.h"
#include "ti_drivers_config.h"
#include "tiop_config.h"

#define RF_NUM_RAT_TICKS_IN_1_US   4

/* The sync word used by the radio TX test command */
#define PLATFORM_RADIO_TX_TEST_SYNC_WORD 0x71764129

/* word sent by the TX test command in modulated mode
 * 0b10101010101010101010
 */
#define PLATFORM_RADIO_TX_TEST_MODULATED_WORD 0xAAAA

/* Word sent by the TX test command in unmodulated mode
 * 0b11111111111111111111
 */
#define PLATFORM_RADIO_TX_TEST_UNMODULATED_WORD 0xFFFF

/* forward declaration for startTransmit */
static RF_CmdHandle startTransmit(RF_Handle aRfHandle);

/* state of the RF interface */
static volatile platformRadio_phyState sState;

/* Control flag to disable channel switching by the stack. This is used by
 * `rfCoreDiagChannelDisable` and `rfCoreDiagChannelEnable`.
 */
static bool sDisableChannel = false;

/*
 * Number of retry counts left to the currently transmitting frame.
 *
 * Initialized when a frame is passed to be sent over the air, and decremented
 * by the radio ISR every time the transmit command string fails to receive a
 * corresponding ack.
 */
static volatile unsigned int sTransmitRetryCount = 0;

/**
 * This structure is used when processing the RX queue entries.
 * Rather then pass around many parameters, they all collected into a
 * single structure and a reference is passed.
 */
struct rx_queue_info
{
    otInstance *aInstance;              /**< Openthread instance for the OpenThread callback */
    unsigned int events;                /**< The events being processed */
    otRadioFrame receiveFrame;          /**< The RX Frame to pass to OpenThread */
    uint8_t *payload;                   /**< The payload of the rx frame */
    rfc_dataEntryGeneral_t *curEntry;   /**< Points to the rx queue entry being processed */
    rfc_dataEntryGeneral_t *startEntry; /**< The first entry processed, used to terminate the loop */
};

/**
 * Structure to store the maximum power of a given channel found from
 * characterization of the radio.
 */
struct tx_power_max
{
    uint8_t channel;                    /**< Channel in IEEE Page 0 */
    int8_t maxPower;                    /**< Maximum power for the Channel */
};

/*
 * Radio command structures that run on the CM0.
 */
static volatile rfc_CMD_RADIO_SETUP_t        sRadioSetupCmd;

static volatile rfc_CMD_IEEE_MOD_FILT_t      sModifyReceiveFilterCmd;
static volatile rfc_CMD_IEEE_MOD_SRC_MATCH_t sModifyReceiveSrcMatchCmd;

static volatile rfc_CMD_IEEE_ED_SCAN_t       sEdScanCmd;

static volatile rfc_CMD_TX_TEST_t            sTxTestCmd;

static volatile rfc_CMD_IEEE_RX_t            sReceiveCmd;

static volatile rfc_CMD_IEEE_CSMA_t          sCsmaBackoffCmd;
static volatile rfc_CMD_IEEE_TX_t            sTransmitCmd;
static volatile rfc_CMD_IEEE_RX_ACK_t        sTransmitRxAckCmd;

static volatile ext_src_match_data_t         sSrcMatchExtData;
static volatile short_src_match_data_t       sSrcMatchShortData;

/* struct containing radio stats */
static volatile rfc_ieeeRxOutput_t sRfStats;

/*
 * Four receive buffers entries with room for 1 max IEEE802.15.4 frame in each
 *
 * These will be setup in a circular buffer configuration by /ref sRxDataQueue.
 */
static __attribute__((aligned(4))) uint8_t sRxBuf0[RX_BUF_SIZE];
static __attribute__((aligned(4))) uint8_t sRxBuf1[RX_BUF_SIZE];
static __attribute__((aligned(4))) uint8_t sRxBuf2[RX_BUF_SIZE];
static __attribute__((aligned(4))) uint8_t sRxBuf3[RX_BUF_SIZE];

/*
 * The RX Data Queue used by @ref sReceiveCmd.
 */
static __attribute__((aligned(4))) dataQueue_t sRxDataQueue = { 0 };

/* openthread data primitives */
static otRadioFrame sTransmitFrame;
static otError      sTransmitError;

static __attribute__((aligned(4))) uint8_t sTransmitPsdu[OT_RADIO_FRAME_MAX_SIZE];

static RF_Object sRfObject;

static RF_Handle sRfHandle;

static RF_CmdHandle sReceiveCmdHandle;
static RF_CmdHandle sTransmitCmdHandle;
static RF_CmdHandle sTxTestCmdHandle;

/* seed the radio random using random from TRNG */
static uint16_t seedRandom;

/**
 * Value requested in dBm from the upper layers on the last call to
 * @ref otPlatRadioSetTransmitPower.
 */
static int8_t sReqTxPower = TIOP_CONFIG_TX_POWER;

static QueueHandle_t sRadioEventQueue;

/**
 * @brief Post a Radio Signal
 *
 * Some Radio event has occurred, wake the process loops.
 *
 * @param [in] evts Events to post
 */
static void radioSignal(unsigned int evts)
{
    if (evts != 0U)
    {
        BaseType_t taskPrio;

        xQueueSendToBackFromISR(sRadioEventQueue, &evts, &taskPrio);
        /* disabled, nothing pends on this queue. Let the platformManager manage.
        if (taskPrio)
        {
            taskYIELD_FROM_ISR ();
        }
        */
        platformRadioSignal();
    }
}

/**
 * @brief initialize the RX/TX buffers
 *
 * Zeros out the receive and transmit buffers and sets up the data structures
 * of the receive queue.
 */
static void rfCoreInitBufs(void)
{
    rfc_dataEntry_t *entry;
    memset(sRxBuf0, 0x00, sizeof(sRxBuf0));
    memset(sRxBuf1, 0x00, sizeof(sRxBuf1));
    memset(sRxBuf2, 0x00, sizeof(sRxBuf2));
    memset(sRxBuf3, 0x00, sizeof(sRxBuf3));

    entry               = (rfc_dataEntry_t *)sRxBuf0;
    entry->pNextEntry   = sRxBuf1;
    entry->config.lenSz = DATA_ENTRY_LENSZ_BYTE;
    entry->length       = sizeof(sRxBuf0) - sizeof(rfc_dataEntry_t);

    entry               = (rfc_dataEntry_t *)sRxBuf1;
    entry->pNextEntry   = sRxBuf2;
    entry->config.lenSz = DATA_ENTRY_LENSZ_BYTE;
    entry->length       = sizeof(sRxBuf1) - sizeof(rfc_dataEntry_t);

    entry               = (rfc_dataEntry_t *)sRxBuf2;
    entry->pNextEntry   = sRxBuf3;
    entry->config.lenSz = DATA_ENTRY_LENSZ_BYTE;
    entry->length       = sizeof(sRxBuf2) - sizeof(rfc_dataEntry_t);

    entry               = (rfc_dataEntry_t *)sRxBuf3;
    entry->pNextEntry   = sRxBuf0;
    entry->config.lenSz = DATA_ENTRY_LENSZ_BYTE;
    entry->length       = sizeof(sRxBuf3) - sizeof(rfc_dataEntry_t);

    sRxDataQueue.pCurrEntry = sRxBuf0;
    sRxDataQueue.pLastEntry = NULL;

    sTransmitFrame.mPsdu   = sTransmitPsdu;
    sTransmitFrame.mLength = 0;
}

/**
 * @brief initializes the setup command structure
 *
 * The sRadioSetupCmd struct is used by the RF driver to bring the
 */
void rfCoreInitSetupCmd(void)
{
    /* initialize radio setup command */
    sRadioSetupCmd = RF_cmdRadioSetup;

    sRadioSetupCmd.startTrigger.pastTrig = 1; // XXX: workaround for RF scheduler
}

/**
 * @brief initialize the RX command structure
 *
 * Sets the default values for the receive command structure.
 */
static void rfCoreInitReceiveParams(void)
{
    sReceiveCmd = RF_cmdIeeeRx;

    sReceiveCmd.pRxQ                          = &sRxDataQueue;
    sReceiveCmd.pOutput                       = (rfc_ieeeRxOutput_t *) &sRfStats;
    sReceiveCmd.numShortEntries               = PLATFORM_RADIO_SHORTADD_SRC_MATCH_NUM;
    sReceiveCmd.pShortEntryList               = (void *) &sSrcMatchShortData;
    sReceiveCmd.numExtEntries                 = PLATFORM_RADIO_EXTADD_SRC_MATCH_NUM;
    sReceiveCmd.pExtEntryList                 = (uint32_t *) &sSrcMatchExtData;
    sReceiveCmd.channel                       = OT_RADIO_2P4GHZ_OQPSK_CHANNEL_MIN;
    sReceiveCmd.ccaRssiThr                    = -90;

    sReceiveCmd.startTrigger.pastTrig         = 1; // XXX: workaround for RF scheduler
    sReceiveCmd.condition.rule                = COND_NEVER;

    sReceiveCmd.rxConfig.bAutoFlushCrc        = 1;
    sReceiveCmd.rxConfig.bAppendCorrCrc       = 1;
    sReceiveCmd.rxConfig.bAppendRssi          = 1;
    sReceiveCmd.rxConfig.bAppendSrcInd        = 1;
    sReceiveCmd.rxConfig.bAppendTimestamp     = 1;

    sReceiveCmd.frameFiltOpt.frameFiltEn      = 1;
    sReceiveCmd.frameFiltOpt.frameFiltStop    = 1;
    sReceiveCmd.frameFiltOpt.autoAckEn        = 1;
    sReceiveCmd.frameFiltOpt.bStrictLenFilter = 1;

    sReceiveCmd.ccaOpt.ccaEnEnergy            = 1;
    sReceiveCmd.ccaOpt.ccaEnCorr              = 1;
    sReceiveCmd.ccaOpt.ccaEnSync              = 1;
    sReceiveCmd.ccaOpt.ccaSyncOp              = 1;
    sReceiveCmd.ccaOpt.ccaCorrThr             = 1;
}

/**
 * @brief Get the receive command's sensitivity.
 */
int8_t otPlatRadioGetReceiveSensitivity(otInstance *aInstance)
{
    (void)aInstance;
    return sReceiveCmd.ccaRssiThr;
}

/**
 * @brief sends the direct abort command to the radio core
 *
 * @param [in] aRfHandle    The handle for the RF core client
 * @param [in] aRfCmdHandle The command handle to be aborted
 *
 * @return the value from the command status register
 * @retval RF_StatSuccess the command completed correctly
 */
static RF_Stat rfCoreExecuteAbortCmd(RF_Handle aRfHandle,
        RF_CmdHandle aRfCmdHandle)
{
    return RF_cancelCmd(aRfHandle, aRfCmdHandle, RF_DRIVER_ABORT);
}

/**
 * @brief enable/disable filtering
 *
 * Uses the radio core to alter the current running RX command filtering
 * options. This ensures there is no access fault between the CM3 and CM0 for
 * the RX command.
 *
 * This function leaves the type of frames to be filtered the same as the
 * receive command.
 *
 * @note An IEEE RX command *must* be running while this command executes.
 *
 * @param [in] aRfHandle The handle for the RF core client
 * @param [in] aEnable   TRUE: enable frame filtering
 *                       FALSE: disable frame filtering
 *
 * @return the value from the command status register
 * @retval RF_StatCmdDoneSuccess the command completed correctly
 */
static RF_Stat rfCoreModifyRxFrameFilter(RF_Handle aRfHandle, bool aEnable)
{
    /* memset skipped because sModifyReceiveFilterCmd has only 3 members */
    sModifyReceiveFilterCmd.commandNo = CMD_IEEE_MOD_FILT;
    /* copy current frame filtering and frame types from running RX command */
    memcpy((void *)&sModifyReceiveFilterCmd.newFrameFiltOpt, (void *)&sReceiveCmd.frameFiltOpt,
           sizeof(sModifyReceiveFilterCmd.newFrameFiltOpt));
    memcpy((void *)&sModifyReceiveFilterCmd.newFrameTypes, (void *)&sReceiveCmd.frameTypes,
           sizeof(sModifyReceiveFilterCmd.newFrameTypes));

    sModifyReceiveFilterCmd.newFrameFiltOpt.frameFiltEn = aEnable ? 1 : 0;

    return RF_runImmediateCmd(aRfHandle, (uint32_t *)&sModifyReceiveFilterCmd);
}

/**
 * @brief enable/disable autoPend
 *
 * Uses the radio core to alter the current running RX command filtering
 * options. This ensures there is no access fault between the CM3 and CM0 for
 * the RX command.
 *
 * This function leaves the type of frames to be filtered the same as the
 * receive command.
 *
 * @note An IEEE RX command *must* be running while this command executes.
 *
 * @param [in] aRfHandle The handle for the RF core client
 * @param [in] aEnable TRUE: enable autoPend, FALSE: disable autoPend
 *
 * @return the value from the command status register
 * @retval RF_StatCmdDoneSuccess the command completed correctly
 */
static RF_Stat rfCoreModifyRxAutoPend(RF_Handle aRfHandle, bool aEnable)
{
    /* memset skipped because sModifyReceiveFilterCmd has only 3 members */
    sModifyReceiveFilterCmd.commandNo = CMD_IEEE_MOD_FILT;
    /* copy current frame filtering and frame types from running RX command */
    memcpy((void *)&sModifyReceiveFilterCmd.newFrameFiltOpt, (void *)&sReceiveCmd.frameFiltOpt,
           sizeof(sModifyReceiveFilterCmd.newFrameFiltOpt));
    memcpy((void *)&sModifyReceiveFilterCmd.newFrameTypes, (void *)&sReceiveCmd.frameTypes,
           sizeof(sModifyReceiveFilterCmd.newFrameTypes));

    sModifyReceiveFilterCmd.newFrameFiltOpt.autoPendEn = aEnable ? 1 : 0;

    return RF_runImmediateCmd(aRfHandle, (uint32_t *)&sModifyReceiveFilterCmd);
}

/**
 * @brief sends the immediate modify source matching command to the radio core
 *
 * Uses the radio core to alter the current source matching parameters used by
 * the running RX command. This ensures there is no access fault between the
 * CM3 and CM0, and ensures that the RX command has cohesive view of the data.
 * The CM3 may make alterations to the source matching entries if the entry is
 * marked as disabled.
 *
 * @note An IEEE RX command *must* be running while this command executes.
 *
 * @param [in] aRfHandle The handle for the RF core client
 * @param [in] aEntryNo  The index of the entry to alter
 * @param [in] aType     TRUE: the entry is a short address
 *                       FALSE: the entry is an extended address
 * @param [in] aEnable   Whether the given entry is to be enabled or disabled
 *
 * @return the value from the command status register
 * @retval RF_StatCmdDoneSuccess the command completed correctly
 */
static RF_Stat rfCoreModifySourceMatchEntry(RF_Handle aRfHandle,
        uint8_t aEntryNo, platformRadio_address aType, bool aEnable)
{
    /* memset kept to save 60 bytes of text space, gcc can't optimize the
     * following bitfield operation if it doesn't know the fields are zero
     * already.
     */
    memset((void *)&sModifyReceiveSrcMatchCmd, 0,
            sizeof(sModifyReceiveSrcMatchCmd));

    sModifyReceiveSrcMatchCmd.commandNo = CMD_IEEE_MOD_SRC_MATCH;

    /* we only use source matching for pending data bit, so enabling and
     * pending are the same to us.
     */
    if (aEnable)
    {
        sModifyReceiveSrcMatchCmd.options.bEnable = 1;
        sModifyReceiveSrcMatchCmd.options.srcPend = 1;
    }
    else
    {
        sModifyReceiveSrcMatchCmd.options.bEnable = 0;
        sModifyReceiveSrcMatchCmd.options.srcPend = 0;
    }

    sModifyReceiveSrcMatchCmd.options.entryType = aType;
    sModifyReceiveSrcMatchCmd.entryNo = aEntryNo;

    return RF_runImmediateCmd(aRfHandle,
            (uint32_t *)&sModifyReceiveSrcMatchCmd);
}

/**
 * @brief walks the short address source match list to find an address
 *
 * @param [in] aAddress the short address to search for
 *
 * @return the index where the address was found
 * @retval PLATFORM_RADIO_SRC_MATCH_NONE the address was not found
 */
static uint8_t rfCoreFindShortSrcMatchIdx(const uint16_t aAddress)
{
    uint8_t i;

    for (i = 0; i < PLATFORM_RADIO_SHORTADD_SRC_MATCH_NUM; i++)
    {
        if (sSrcMatchShortData.shortAddrEnt[i].shortAddr == aAddress)
        {
            return i;
        }
    }

    return PLATFORM_RADIO_SRC_MATCH_NONE;
}

/**
 * @brief walks the short address source match list to find an empty slot
 *
 * @return the index of an unused address slot
 * @retval PLATFORM_RADIO_SRC_MATCH_NONE no unused slots available
 */
static uint8_t rfCoreFindEmptyShortSrcMatchIdx(void)
{
    uint8_t i;

    for (i = 0; i < PLATFORM_RADIO_SHORTADD_SRC_MATCH_NUM; i++)
    {
        if ((sSrcMatchShortData.srcMatchEn[i / 32] & (1 << (i % 32))) == 0u)
        {
            return i;
        }
    }

    return PLATFORM_RADIO_SRC_MATCH_NONE;
}

/**
 * @brief walks the ext address source match list to find an address
 *
 * @param [in] aAddress the ext address to search for
 *
 * @return the index where the address was found
 * @retval PLATFORM_RADIO_SRC_MATCH_NONE the address was not found
 */
static uint8_t rfCoreFindExtSrcMatchIdx(uint64_t aAddress)
{
    uint8_t  i;

    for (i = 0; i < PLATFORM_RADIO_EXTADD_SRC_MATCH_NUM; i++)
    {
        if (sSrcMatchExtData.extAddrEnt[i] == aAddress)
        {
            return i;
        }
    }

    return PLATFORM_RADIO_SRC_MATCH_NONE;
}

/**
 * @brief walks the ext address source match list to find an empty slot
 *
 * @return the index of an unused address slot
 * @retval PLATFORM_RADIO_SRC_MATCH_NONE no unused slots available
 */
static uint8_t rfCoreFindEmptyExtSrcMatchIdx(void)
{
    uint8_t i;

    for (i = 0; i < PLATFORM_RADIO_EXTADD_SRC_MATCH_NUM; i++)
    {
        if ((sSrcMatchExtData.srcMatchEn[i / 32] & (1 << (i % 32))) == 0u)
        {
            return i;
        }
    }

    return PLATFORM_RADIO_SRC_MATCH_NONE;
}

/**
 * Helper function for handling retries when transmitting.
 *
 * @retval  event mask to post if done
 */
static unsigned int handleTxAck(RF_Handle aRfHandle)
{
    otError      error;
    unsigned int ret;

    if (sCsmaBackoffCmd.status == IEEE_DONE_BUSY)
    {
        /* CSMA-CA could not complete successfully */
        error = OT_ERROR_CHANNEL_ACCESS_FAILURE;
    }
    else if (sTransmitCmd.status != IEEE_DONE_OK)
    {
        error = OT_ERROR_FAILED;
    }
    else
    {
        switch (sTransmitRxAckCmd.status)
        {
            case IEEE_DONE_TIMEOUT:
            {
                error = OT_ERROR_NO_ACK;
                break;
            }

            case IEEE_DONE_ACK:
            case IEEE_DONE_ACKPEND:
            {
                error = OT_ERROR_NONE;
                break;
            }

            case IEEE_DONE_BUSY:
            {
                error = OT_ERROR_CHANNEL_ACCESS_FAILURE;
                break;
            }

            default:
            {
                /* general failure */
                error = OT_ERROR_FAILED;
                break;
            }
        }
    }

    if (OT_ERROR_NONE != error
            && sTransmitRetryCount < IEEE802154_MAC_MAX_FRAMES_RETRIES)
    {
        /* re-submit the tx command chain to re-try the transmission*/
        sTransmitRetryCount++;
        sTransmitCmdHandle = startTransmit(aRfHandle);
        ret = 0U;
    }
    else
    {
        /* signal polling function we are done transmitting, we failed
         * to send the packet
         */
        sTransmitError = error;
        ret = RF_EVENT_TX_DONE;
    }

    return ret;
}

/**
 * Callback for the TX command chain.
 *
 * This callback is called on completion of the command string or failure of
 * an individual command in the string.
 *
 * @param [in] aRfHandle    Handle of the RF object
 * @param [in] aRfCmdHandle Handle of the command chain that finished
 * @param [in] aRfEventMask Events that happened to trigger this callback
 */
static void rfCoreTxCallback(RF_Handle aRfHandle,
                             RF_CmdHandle aRfCmdHandle,
                             RF_EventMask aRfEventMask)
{
    unsigned int evts = 0U;

    if (aRfEventMask & (RF_EventCmdPreempted))
    {
        /* The RF driver has preempted this command string and the main
         * processing loop will need to re-submit the TX.
         */
        evts |= RF_EVENT_TX_CMD_PREEMPTED;
    }
    else if (aRfEventMask & (RF_EventCmdCancelled | RF_EventCmdAborted | RF_EventCmdStopped))
    {
        /* General failure of the command string, notify processing loop.
         * Likely the TX was aborted to return to RX for some reason.
         */
        sTransmitError = OT_ERROR_FAILED;
        evts |= RF_EVENT_TX_DONE;
    }
    else if (aRfEventMask & (RF_EventLastFGCmdDone | RF_EventLastCmdDone))
    {
        seedRandom = sCsmaBackoffCmd.randomState;

        if (sTransmitCmd.pPayload[0] & IEEE802154_ACK_REQUEST)
        {
            /* The transmitted frame is requesting an acknowledgment, see if we
             * received it or if we need to try again. */
            evts |= handleTxAck(aRfHandle);
        }
        else
        {
            if (sCsmaBackoffCmd.status == IEEE_DONE_BUSY)
            {
                /* CSMA-CA could not complete successfully */
                sTransmitError = OT_ERROR_CHANNEL_ACCESS_FAILURE;
            }
            else if (sTransmitCmd.status == IEEE_DONE_OK)
            {
                /* The CSMA-CA command completed successfully and the transmit
                 * command completed successfully, successful transmission */
                sTransmitError = OT_ERROR_NONE;
            }
            else
            {
                /* Transmission failed */
                sTransmitError = OT_ERROR_FAILED;
            }
            evts |= RF_EVENT_TX_DONE;
        }
    }

    /* tell radio processing loop what happened */
    radioSignal(evts);
}

/**
 * @brief Start a transmission, or a start a retry
 *
 * @param [in] aRfHandle The rf handle
 * @return RF command handle for the transmission.
 */
static RF_CmdHandle startTransmit(RF_Handle aRfHandle)
{
    RF_CmdHandle r;
    RF_ScheduleCmdParams rfScheduleCmdParams;

    /* no error has occured (yet) */
    sTransmitError = OT_ERROR_NONE;

    RF_ScheduleCmdParams_init(&rfScheduleCmdParams);

    r = RF_scheduleCmd(aRfHandle, (RF_Op *)&sCsmaBackoffCmd,
                       &rfScheduleCmdParams, rfCoreTxCallback,
                       RF_EventLastFGCmdDone);
    return r;
}

/**
 * @brief sends the tx command to the radio core
 *
 * Sends the packet to the radio core to be sent asynchronously.
 *
 * @param [in] aRfHandle The handle for the RF core client
 * @param [in] aPsdu     A pointer to the data to be sent
 * @note this *must* be 4 byte aligned and not include the FCS, that is
 * calculated in hardware.
 * @param [in] aLen      The length in bytes of data pointed to by psdu.
 *
 * @return handle of the running command returned by the command scheduler
 */
static RF_CmdHandle rfCoreSendTransmitCmd(otInstance *aInstance,
                                          RF_Handle aRfHandle,
                                          uint8_t *aPsdu,
                                          uint8_t aLen)
{
    /* reset retry count */
    sTransmitRetryCount = 0;

    sCsmaBackoffCmd = RF_cmdIeeeCsma;

    sCsmaBackoffCmd.randomState            = seedRandom;
    sCsmaBackoffCmd.pNextOp                = (rfc_radioOp_t *) &sTransmitCmd;
    sCsmaBackoffCmd.BE                     = IEEE802154_MAC_MIN_BE;

    sCsmaBackoffCmd.startTrigger.pastTrig  = 1; // XXX: workaround for RF scheduler
    sCsmaBackoffCmd.condition.rule         = COND_STOP_ON_FALSE;

    sCsmaBackoffCmd.macMaxBE               = IEEE802154_MAC_MAX_BE;
    sCsmaBackoffCmd.macMaxCSMABackoffs     = IEEE802154_MAC_MAX_CSMA_BACKOFFS;

    sCsmaBackoffCmd.csmaConfig.initCW      = 1;
    sCsmaBackoffCmd.csmaConfig.bSlotted    = 0;
    sCsmaBackoffCmd.csmaConfig.rxOffMode   = 0;

    sCsmaBackoffCmd.endTrigger.triggerType = TRIG_NEVER;

    sTransmitCmd = RF_cmdIeeeTx;
    /* no need to look for an ack if the tx operation was stopped */
    sTransmitCmd.payloadLen             = aLen;

    sTransmitCmd.startTrigger.pastTrig  = 1; // XXX: workaround for RF scheduler
    sTransmitCmd.condition.rule         = COND_NEVER;

    /* XXX: the command callback uses the pPayload pointer is a pseudo
     * "transmit active" flag, if NULL a transmission is not active
     */
    sTransmitCmd.pPayload = aPsdu;

    /* XXX: we ignore the ack request flag if the diag module is enabled. */
    if (!otDiagIsEnabled(aInstance) && (aPsdu[0] & IEEE802154_ACK_REQUEST))
    {
        /* setup the receive ack command to follow the tx command */
        sTransmitCmd.condition.rule = COND_STOP_ON_FALSE;
        sTransmitCmd.pNextOp = (rfc_radioOp_t *) &sTransmitRxAckCmd;

        sTransmitRxAckCmd = RF_cmdIeeeRxAck;

        sTransmitRxAckCmd.seqNo                  = aPsdu[IEEE802154_DSN_OFFSET];
        sTransmitRxAckCmd.endTime                = ((IEEE802154_MAC_ACK_WAIT_DURATION *
                    PLATFORM_RADIO_RAT_TICKS_PER_SEC)/ IEEE802154_SYMBOLS_PER_SEC);

        sTransmitRxAckCmd.startTrigger.pastTrig  = 1; // XXX: workaround for RF scheduler
        sTransmitRxAckCmd.condition.rule         = COND_NEVER;

        sTransmitRxAckCmd.endTrigger.triggerType = TRIG_REL_START;
        sTransmitRxAckCmd.endTrigger.pastTrig    = 1;

    }
    else
    {
        /* it was a broadcast, for example a beacon, no ack expected */
    }

    sTransmitError = OT_ERROR_NONE;

    return startTransmit(aRfHandle);
}

/* Forward definition for RX callback */
static RF_CmdHandle rfCoreSendReceiveCmd(RF_Handle aRfHandle);

/**
 * Callback for the receive command.
 *
 *
 *
 * @param [in] aRfHandle    Handle of the RF object
 * @param [in] aRfCmdHandle Handle of the command chain that finished
 * @param [in] aRfEventMask Events that happened to trigger this callback
 */
static void rfCoreRxCallback(RF_Handle aRfHandle,
                             RF_CmdHandle aRfCmdHandle,
                             RF_EventMask aRfEventMask)
{
    unsigned int evts = 0U;

    if (aRfEventMask & RF_EventRxBufFull)
    {
        evts |= RF_EVENT_BUF_FULL;
    }

    if (aRfEventMask & RF_EventTXAck)
    {
        /* A packet was received, that packet required an ACK and the
         * ACK has been transmitted
         */
        evts |= RF_EVENT_RX_ACK_DONE;
    }

    if (aRfEventMask & RF_EventRxEntryDone)
    {
        /* A packet was received the packet MAY require an ACK Or the
         * packet might not (ie: a broadcast)
         */
        evts |= RF_EVENT_RX_DONE;
    }

    else if (aRfEventMask & ((RF_EventLastCmdDone)
                             | (RF_EventCmdCancelled | RF_EventCmdAborted | RF_EventCmdStopped)))
    {
        /* The RX command is stopped. This may be due to a change in the RX
         * command or the RF driver might have stopped the command. Abort
         * conditions are delivered separately from regular last command done
         * events but are handled the same way.
         */
        evts |= RF_EVENT_RX_CMD_STOP;
    }

    if (aRfEventMask & RF_EventCmdPreempted)
    {
        /* The RX command was aborted. This may be due to a loss in FS lock or
         * the temperature changed requiring re-calibration. */
        evts |= RF_EVENT_RX_CMD_PREEMPTED;

        /* The RF driver has preempted the RX command. Check if the RX is
         * running and re-submit the command.
         *
         * Re-submission of the RX command is done in the RX callback to try
         * and avoid the TX command being submitted without a backgrounded RX
         * command.
         */
        if (sReceiveCmd.status != PENDING
                && sReceiveCmd.status != ACTIVE
                && sReceiveCmd.status != IEEE_SUSPENDED)
        {
            sReceiveCmdHandle = rfCoreSendReceiveCmd(sRfHandle);
        }
    }

    /* tell radio processing loop what happened */
    radioSignal(evts);
}

/**
 * @brief sends the rx command to the radio core
 *
 * Sends the pre-built receive command to the radio core. This sets up the
 * radio to receive packets according to the settings in the global rx command.
 *
 * @note This function does not alter any of the parameters of the rx command.
 * It is only concerned with sending the command to the radio core. See @ref
 * otPlatRadioSetPanId for an example of how the rx settings are set changed.
 *
 * @param [in] aRfHandle The handle for the RF core client
 *
 * @return handle of the running command returned by the command scheduler
 */
static RF_CmdHandle rfCoreSendReceiveCmd(RF_Handle aRfHandle)
{
    RF_ScheduleCmdParams rfScheduleCmdParams;
    RF_ScheduleCmdParams_init(&rfScheduleCmdParams);

    sReceiveCmd.status = IDLE;

    return RF_scheduleCmd(aRfHandle, (RF_Op *)&sReceiveCmd,
                          &rfScheduleCmdParams, rfCoreRxCallback,
                          (RF_EventLastCmdDone | RF_EventRxEntryDone |
                           RF_EventTXAck | RF_EventRxBufFull));
}

/**
 * @brief Sets the transmit.
 *
 * Sets the transmit power within the radio setup command or the override list.
 */
static otError rfCoreSetTransmitPower(int8_t aPower)
{
    otError retval = OT_ERROR_NONE;
    RF_TxPowerTable_Value oldValue;
    RF_TxPowerTable_Value newValue;

    /* find the tx power configuration */
    newValue = RF_TxPowerTable_findValue(txPowerTable, aPower);
    oldValue = RF_getTxPower(sRfHandle);
    otEXPECT_ACTION(RF_TxPowerTable_INVALID_VALUE != newValue.rawValue,
                    retval = OT_ERROR_INVALID_ARGS);

    /* set the tx power configuration */
    if (platformRadio_phyState_Sleep == sState ||
        platformRadio_phyState_Disabled == sState ||
        newValue.paType == oldValue.paType)
    {
        otEXPECT_ACTION(RF_StatSuccess == RF_setTxPower(sRfHandle, newValue),
                        retval = OT_ERROR_FAILED);
    }
    else
    {
        rfCoreExecuteAbortCmd(sRfHandle, sReceiveCmdHandle);
        otEXPECT_ACTION((sReceiveCmd.status != PENDING
                && sReceiveCmd.status != ACTIVE
                && sReceiveCmd.status != IEEE_SUSPENDED),
                        retval = OT_ERROR_FAILED);

        otEXPECT_ACTION(RF_StatSuccess == RF_setTxPower(sRfHandle, newValue),
                        retval = OT_ERROR_FAILED);

        sReceiveCmdHandle = rfCoreSendReceiveCmd(sRfHandle);
        otEXPECT_ACTION(sReceiveCmdHandle >= 0, retval = OT_ERROR_FAILED);
    }

exit:
    return retval;
}

/**
 * @brief Callback for the Energy Detect command.
 *
 * @param [in] aRfHandle    Handle of the RF object
 * @param [in] aRfCmdHandle Handle of the command chain that finished
 * @param [in] aRfEventMask Events that happened to trigger this callback
 */
static void rfCoreEdScanCmdCallback(RF_Handle aRfHandle,
                                    RF_CmdHandle aRfCmdHandle,
                                    RF_EventMask aRfEventMask)
{
    radioSignal(RF_EVENT_ED_SCAN_DONE);
}

/**
 * @brief sends the energy detect scan command to the radio core
 *
 * Sends the Energy Detect scan command to the radio core. This scans the given
 * channel for activity.
 *
 * @param [in] aRfHandle The handle for the RF core client
 * @param [in] aChannel  The IEEE page 0 channel to scan
 * @param [in] aDuration Time in ms to scan
 *
 * @return handle of the running command returned by the command scheduler
 */
static RF_CmdHandle rfCoreSendEdScanCmd(RF_Handle aRfHandle,
                                        uint8_t aChannel,
                                        uint16_t aDuration)
{
    RF_ScheduleCmdParams rfScheduleCmdParams;
    RF_ScheduleCmdParams_init(&rfScheduleCmdParams);

    sEdScanCmd = RF_cmdIeeeEdScan;

    sEdScanCmd.channel                = aChannel;
    sEdScanCmd.ccaRssiThr             = -90;

    sEdScanCmd.startTrigger.pastTrig  = 1; // XXX: workaround for RF scheduler
    sEdScanCmd.condition.rule         = COND_NEVER;

    sEdScanCmd.ccaOpt.ccaEnEnergy     = 1;
    sEdScanCmd.ccaOpt.ccaEnCorr       = 1;
    sEdScanCmd.ccaOpt.ccaEnSync       = 1;
    sEdScanCmd.ccaOpt.ccaCorrOp       = 1;
    sEdScanCmd.ccaOpt.ccaSyncOp       = 0;
    sEdScanCmd.ccaOpt.ccaCorrThr      = 3;

    sEdScanCmd.endTrigger.triggerType = TRIG_REL_START;
    sEdScanCmd.endTrigger.pastTrig    = 1;

    /* duration is in ms */
    sEdScanCmd.endTime                = aDuration * (PLATFORM_RADIO_RAT_TICKS_PER_SEC / 1000);

    return RF_scheduleCmd(aRfHandle,
                          (RF_Op *)&sEdScanCmd,
                          &rfScheduleCmdParams,
                          rfCoreEdScanCmdCallback,
                          RF_EventLastCmdDone);
}

/**
 * @brief Callback for the Transmit Test command.
 *
 * @param [in] aRfHandle    Handle of the RF object
 * @param [in] aRfCmdHandle Handle of the command chain that finished
 * @param [in] aRfEventMask Events that happened to trigger this callback
 */
static void rfCoreTxTestCmdCallback(RF_Handle aRfHandle,
                                    RF_CmdHandle aRfCmdHandle,
                                    RF_EventMask aRfEventMask)
{
    (void) aRfHandle;
    (void) aRfCmdHandle;
    (void) aRfEventMask;

    return;
}

/**
 * @param [in] aRfHandle  The handle for the RF core client
 * @param [in] aModulated TRUE: Send modulated word
 *                        FALSE: Send unmodulated word
 *
 * @return handle of the running command returned by the command scheduler
 */
static RF_CmdHandle rfCoreSendTxTestCmd(RF_Handle aRfHandle, bool aModulated)
{
    RF_ScheduleCmdParams rfScheduleCmdParams;
    RF_ScheduleCmdParams_init(&rfScheduleCmdParams);

    sTxTestCmd = RF_cmdTxTest;

    sTxTestCmd.startTrigger.pastTrig = 1; // XXX: workaround for RF scheduler
    sTxTestCmd.condition.rule        = COND_NEVER;

    if (aModulated)
    {
        sTxTestCmd.txWord = PLATFORM_RADIO_TX_TEST_MODULATED_WORD;
    }
    else
    {
        sTxTestCmd.config.bUseCw = 1;
        sTxTestCmd.txWord        = PLATFORM_RADIO_TX_TEST_UNMODULATED_WORD;
    }

    return RF_scheduleCmd(aRfHandle, (RF_Op *) &sTxTestCmd,
                          &rfScheduleCmdParams, rfCoreTxTestCmdCallback,
                          RF_EventLastCmdDone);
}

/**
 * Default error callback for RF Driver.
 *
 * Errors are unlikely, and fatal.
 */
static void rfCoreErrorCallback(RF_Handle aHandle,
                                RF_CmdHandle aCmdHandle,
                                RF_EventMask aEvents)
{
    while(1);
}

/**
 * Function documented in radio.h
 */
void rfCoreDiagChannelDisable(uint8_t aChannel)
{
    otPlatRadioReceive(NULL, aChannel);
    sDisableChannel = true;
}

/**
 * Function documented in radio.h
 */
void rfCoreDiagChannelEnable(uint8_t aChannel)
{
    sDisableChannel = false;
    otPlatRadioReceive(NULL, aChannel);
}

/**
 * Function documented in system.h
 */
void platformRadioInit(void)
{
    rfCoreInitBufs();
    rfCoreInitSetupCmd();
    /* Populate the RX parameters data structure with default values */
    rfCoreInitReceiveParams();

    /* get the seed from true random generator */
    otPlatEntropyGet((uint8_t*)&seedRandom, sizeof(seedRandom));

    sRadioEventQueue = xQueueCreate(10, sizeof(uintptr_t));

    sState = platformRadio_phyState_Disabled;
}

/**
 * Function documented in platform/radio.h
 */
otError otPlatRadioEnable(otInstance *aInstance)
{
    otError error = OT_ERROR_BUSY;
    RF_Params rfParams;
    (void)aInstance;

    if (sState == platformRadio_phyState_Sleep)
    {
        error = OT_ERROR_NONE;
    }
    else if (sState == platformRadio_phyState_Disabled)
    {
        RF_Params_init(&rfParams);

        rfParams.pErrCb         = rfCoreErrorCallback;

        sRfHandle = RF_open(&sRfObject, &RF_prop,
                (RF_RadioSetup *)&sRadioSetupCmd, &rfParams);

        otEXPECT_ACTION(sRfHandle != NULL, error = OT_ERROR_FAILED);
        sState = platformRadio_phyState_Sleep;

        error = OT_ERROR_NONE;
    }

exit:
    if (error == OT_ERROR_FAILED)
    {
        sState = platformRadio_phyState_Disabled;
    }

    return error;
}

/**
 * Function documented in platform/radio.h
 */
bool otPlatRadioIsEnabled(otInstance *aInstance)
{
    (void)aInstance;
    return (sState != platformRadio_phyState_Disabled);
}

/**
 * Function documented in platform/radio.h
 */
otError otPlatRadioDisable(otInstance *aInstance)
{
    otError error = OT_ERROR_BUSY;
    (void)aInstance;

    if (sState == platformRadio_phyState_Disabled)
    {
        error = OT_ERROR_NONE;
    }
    else if (sState == platformRadio_phyState_Sleep)
    {
        RF_close(sRfHandle);
        sState = platformRadio_phyState_Disabled;
        error = OT_ERROR_NONE;
    }

    return error;
}

/**
 * Function documented in platform/radio.h
 */
otError otPlatRadioGetCcaEnergyDetectThreshold(otInstance *aInstance, int8_t *aThreshold)
{
    OT_UNUSED_VARIABLE(aInstance);
    OT_UNUSED_VARIABLE(aThreshold);

    return OT_ERROR_NOT_IMPLEMENTED;
}

/**
 * Function documented in platform/radio.h
 */
otError otPlatRadioSetCcaEnergyDetectThreshold(otInstance *aInstance, int8_t aThreshold)
{
    OT_UNUSED_VARIABLE(aInstance);
    OT_UNUSED_VARIABLE(aThreshold);

    return OT_ERROR_NOT_IMPLEMENTED;
}

/**
 * Function documented in platform/radio.h
 */
otError otPlatRadioEnergyScan(otInstance *aInstance, uint8_t aScanChannel,
        uint16_t aScanDuration)
{
    otError error = OT_ERROR_NONE;
    (void)aInstance;

    switch (sState)
    {
        case platformRadio_phyState_Receive:
        {
            sState = platformRadio_phyState_EdScan;
            /* abort receive */
            rfCoreExecuteAbortCmd(sRfHandle, sReceiveCmdHandle);
            otEXPECT_ACTION((sReceiveCmd.status != PENDING
                             && sReceiveCmd.status != ACTIVE
                             && sReceiveCmd.status != IEEE_SUSPENDED),
                            error = OT_ERROR_FAILED);
            /* fall through */
        }

        case platformRadio_phyState_Sleep:
        {
            sState = platformRadio_phyState_EdScan;
            otEXPECT_ACTION(rfCoreSendEdScanCmd(sRfHandle, aScanChannel,
                                                aScanDuration) >= 0,
                            error = OT_ERROR_FAILED);
            break;
        }

        default:
        {
            error = OT_ERROR_BUSY;
            break;
        }
    }

exit:
    if (OT_ERROR_NONE != error)
    {
        sState = platformRadio_phyState_Sleep;
    }
    return error;
}

/**
 * Function documented in platform/radio.h
 */
otError otPlatRadioSetTransmitPower(otInstance *aInstance, int8_t aPower)
{
    (void)aInstance;
    /* update the tracking variable */
    sReqTxPower = aPower;
    /* set the power */
    return rfCoreSetTransmitPower(aPower);
}

/**
 * Function documented in platform/radio.h
 */
otError otPlatRadioGetTransmitPower(otInstance *aInstance, int8_t *aPower)
{
    otError error = OT_ERROR_NONE;
    (void)aInstance;

    otEXPECT_ACTION(aPower != NULL, error = OT_ERROR_INVALID_ARGS);
    *aPower = RF_TxPowerTable_findPowerLevel(txPowerTable, RF_getTxPower(sRfHandle));

exit:
    return error;
}

/**
 * Function documented in platform/radio.h
 */
otError otPlatRadioReceive(otInstance *aInstance, uint8_t aChannel)
{
    otError error = OT_ERROR_BUSY;
    (void)aInstance;

    if (sState == platformRadio_phyState_Sleep)
    {
        /* initialize the receive command
         *
         * no memset here because we assume init has been called and we may
         * have changed some values in the rx command
         */
        if (!sDisableChannel)
        {
            /* If the diag module has not locked out changing the channel */
            sReceiveCmd.channel = aChannel;
        }
        /* allow the transmit power helper function to manage the characterized
         * max power.
         */
        rfCoreSetTransmitPower(sReqTxPower);
        /* send the command to the radio */
        sReceiveCmdHandle = rfCoreSendReceiveCmd(sRfHandle);
        otEXPECT_ACTION(sReceiveCmdHandle >= 0, error = OT_ERROR_FAILED);
        /* update the tracking variables */
        sState = platformRadio_phyState_Receive;
        error = OT_ERROR_NONE;
    }
    else if (sState == platformRadio_phyState_Receive)
    {
        if (sReceiveCmd.channel == aChannel || sDisableChannel)
        {
            /* we are already running on the correct channel or the diag module
             * has disallowed switching channels.
             */
            error = OT_ERROR_NONE;
        }
        else
        {
            rfCoreExecuteAbortCmd(sRfHandle, sReceiveCmdHandle);
            otEXPECT_ACTION((sReceiveCmd.status != PENDING
                        && sReceiveCmd.status != ACTIVE
                        && sReceiveCmd.status != IEEE_SUSPENDED),
                     error = OT_ERROR_FAILED);

            sReceiveCmd.channel = aChannel;
            /* allow the transmit power helper function to manage the characterized
             * max power.
             */
            rfCoreSetTransmitPower(sReqTxPower);
            /* send the command to the radio */
            sReceiveCmdHandle = rfCoreSendReceiveCmd(sRfHandle);
            otEXPECT_ACTION(sReceiveCmdHandle >= 0, error = OT_ERROR_FAILED);
            error = OT_ERROR_NONE;
        }
    }

exit:
    return error;
}

/**
 * Function documented in platform/radio.h
 */
otError otPlatRadioSleep(otInstance *aInstance)
{
    otError error = OT_ERROR_BUSY;
    (void)aInstance;

    if (sState == platformRadio_phyState_Sleep)
    {
        error = OT_ERROR_NONE;
    }
    else if (sState == platformRadio_phyState_Receive)
    {
        rfCoreExecuteAbortCmd(sRfHandle, sReceiveCmdHandle);
        otEXPECT_ACTION((sReceiveCmd.status != PENDING
                             && sReceiveCmd.status != ACTIVE
                             && sReceiveCmd.status != IEEE_SUSPENDED),
                        error = OT_ERROR_FAILED);

        sState = platformRadio_phyState_Sleep;

        /* The upper layers like to thrash the interface from RX to sleep.
         * Aborting and restarting the commands wastes time and energy, but
         * can be done as often as requested; yielding the RF driver causes
         * the whole core to be shutdown. Delay yield until the rf processing
         * loop to make sure we actually want to sleep.
         */
        radioSignal(RF_EVENT_SLEEP_YIELD);
        error = OT_ERROR_NONE;
    }

exit:
    return error;
}

/**
 * Function documented in platform/radio.h
 */
otRadioFrame *otPlatRadioGetTransmitBuffer(otInstance *aInstance)
{
    (void)aInstance;
    return &sTransmitFrame;
}

/**
 * Function documented in platform/radio.h
 */
otError otPlatRadioTransmit(otInstance *aInstance, otRadioFrame *aFrame)
{
    otError error = OT_ERROR_BUSY;

    if (sState == platformRadio_phyState_Receive)
    {
        sState = platformRadio_phyState_Transmit;

        /* removing 2 bytes of CRC placeholder, generated in hardware */
        sTransmitCmdHandle = rfCoreSendTransmitCmd(aInstance, sRfHandle, aFrame->mPsdu,
                aFrame->mLength - 2);
        otEXPECT_ACTION(sTransmitCmdHandle >= 0, error = OT_ERROR_FAILED);
        error = OT_ERROR_NONE;
    }

exit:
    return error;
}

/**
 * Function documented in platform/radio.h
 */
int8_t otPlatRadioGetRssi(otInstance *aInstance)
{
    (void)aInstance;
    return sRfStats.maxRssi;
}

/**
 * Function documented in platform/radio.h
 */
#ifdef __TI_ARM__
/*
 * ti-cgt warns about using enums as bitfields because by default all enums
 * are packed to save space. We could pass the `--enum_type=int` switch to the
 * compiler, but this creates linking errors with our board support libraries.
 *
 * Instead we disable `#190-D enumerated type mixed with another type`.
 */
#pragma diag_push
#pragma diag_suppress 190
#endif
otRadioCaps otPlatRadioGetCaps(otInstance *aInstance)
{
    (void)aInstance;
    return OT_RADIO_CAPS_ACK_TIMEOUT | OT_RADIO_CAPS_ENERGY_SCAN
        | OT_RADIO_CAPS_TRANSMIT_RETRIES;
}
#ifdef __TI_ARM__
#pragma diag_pop
#endif

/**
 * Function documented in platform/radio.h
 */
void otPlatRadioEnableSrcMatch(otInstance *aInstance, bool aEnable)
{
    (void)aInstance;

    if (sReceiveCmd.status == ACTIVE || sReceiveCmd.status == IEEE_SUSPENDED)
    {
        /* we have a running or backgrounded rx command */
        otEXPECT(rfCoreModifyRxAutoPend(sRfHandle, aEnable)
                == RF_StatCmdDoneSuccess);
    }
    else
    {
        /* if we are promiscuous, then frame filtering should be disabled */
        sReceiveCmd.frameFiltOpt.autoPendEn = aEnable ? 1 : 0;
    }

exit:
    return;
}

/**
 * Function documented in platform/radio.h
 */
otError otPlatRadioAddSrcMatchShortEntry(otInstance *aInstance,
        const uint16_t aShortAddress)
{
    otError error = OT_ERROR_NONE;
    (void)aInstance;
    uint8_t idx = rfCoreFindShortSrcMatchIdx(aShortAddress);

    if (idx == PLATFORM_RADIO_SRC_MATCH_NONE)
    {
        /* the entry does not exist already, add it */
        otEXPECT_ACTION((idx = rfCoreFindEmptyShortSrcMatchIdx())
                != PLATFORM_RADIO_SRC_MATCH_NONE,
                error = OT_ERROR_NO_BUFS);
        sSrcMatchShortData.shortAddrEnt[idx].shortAddr = aShortAddress;
        sSrcMatchShortData.shortAddrEnt[idx].panId = sReceiveCmd.localPanID;
    }

    if (sReceiveCmd.status == ACTIVE || sReceiveCmd.status == IEEE_SUSPENDED)
    {
        /* we have a running or backgrounded rx command */
        otEXPECT_ACTION(rfCoreModifySourceMatchEntry(sRfHandle, idx,
                    platformRadio_address_short, true) == RF_StatCmdDoneSuccess,
                error = OT_ERROR_FAILED);
    }
    else
    {
        /* we are not running, so we must update the values ourselves */
        sSrcMatchShortData.srcPendEn[idx / 32] |= (1 << (idx % 32));
        sSrcMatchShortData.srcMatchEn[idx / 32] |= (1 << (idx % 32));
    }

exit:
    return error;
}

/**
 * Function documented in platform/radio.h
 */
otError otPlatRadioClearSrcMatchShortEntry(otInstance *aInstance,
        const uint16_t aShortAddress)
{
    otError error = OT_ERROR_NONE;
    (void)aInstance;
    uint8_t idx;
    otEXPECT_ACTION((idx = rfCoreFindShortSrcMatchIdx(aShortAddress))
            != PLATFORM_RADIO_SRC_MATCH_NONE,
            error = OT_ERROR_NO_ADDRESS);

    if (sReceiveCmd.status == ACTIVE || sReceiveCmd.status == IEEE_SUSPENDED)
    {
        /* we have a running or backgrounded rx command */
        otEXPECT_ACTION(rfCoreModifySourceMatchEntry(sRfHandle, idx,
                    platformRadio_address_short, false) == RF_StatCmdDoneSuccess,
                error = OT_ERROR_FAILED);
    }
    else
    {
        /* we are not running, so we must update the values ourselves */
        sSrcMatchShortData.srcPendEn[idx / 32] &= ~(1 << (idx % 32));
        sSrcMatchShortData.srcMatchEn[idx / 32] &= ~(1 << (idx % 32));
    }

exit:
    return error;
}

/**
 * Function documented in platform/radio.h
 */
otError otPlatRadioAddSrcMatchExtEntry(otInstance *aInstance,
        const otExtAddress *aExtAddress)
{
    otError error = OT_ERROR_NONE;
    (void)aInstance;
    uint8_t idx;
    uint64_t extAddr;
    memcpy(&extAddr, aExtAddress->m8, sizeof(uint64_t));
    idx = rfCoreFindExtSrcMatchIdx(extAddr);

    if (idx == PLATFORM_RADIO_SRC_MATCH_NONE)
    {
        /* the entry does not exist already, add it */
        otEXPECT_ACTION((idx = rfCoreFindEmptyExtSrcMatchIdx())
                != PLATFORM_RADIO_SRC_MATCH_NONE,
                error = OT_ERROR_NO_BUFS);
        sSrcMatchExtData.extAddrEnt[idx] = extAddr;
    }

    if (sReceiveCmd.status == ACTIVE || sReceiveCmd.status == IEEE_SUSPENDED)
    {
        /* we have a running or backgrounded rx command */
        otEXPECT_ACTION(rfCoreModifySourceMatchEntry(sRfHandle, idx,
                    platformRadio_address_ext, true) == RF_StatCmdDoneSuccess,
                error = OT_ERROR_FAILED);
    }
    else
    {
        /* we are not running, so we must update the values ourselves */
        sSrcMatchExtData.srcPendEn[idx / 32] |= (1 << (idx % 32));
        sSrcMatchExtData.srcMatchEn[idx / 32] |= (1 << (idx % 32));
    }

exit:
    return error;
}

/**
 * Function documented in platform/radio.h
 */
otError otPlatRadioClearSrcMatchExtEntry(otInstance *aInstance,
        const otExtAddress *aExtAddress)
{
    otError error = OT_ERROR_NONE;
    (void)aInstance;
    uint8_t idx;
    uint64_t extAddr;
    memcpy(&extAddr, aExtAddress->m8, sizeof(uint64_t));
    otEXPECT_ACTION((idx = rfCoreFindExtSrcMatchIdx(extAddr))
            != PLATFORM_RADIO_SRC_MATCH_NONE, error = OT_ERROR_NO_ADDRESS);

    if (sReceiveCmd.status == ACTIVE || sReceiveCmd.status == IEEE_SUSPENDED)
    {
        /* we have a running or backgrounded rx command */
        otEXPECT_ACTION(rfCoreModifySourceMatchEntry(sRfHandle, idx,
                    platformRadio_address_ext, false) == RF_StatCmdDoneSuccess,
                error = OT_ERROR_FAILED);
    }
    else
    {
        /* we are not running, so we must update the values ourselves */
        sSrcMatchExtData.srcPendEn[idx / 32] &= ~(1 << (idx % 32));
        sSrcMatchExtData.srcMatchEn[idx / 32] &= ~(1 << (idx % 32));
    }

exit:
    return error;
}

/**
* Function documented in platform/radio.h
*/
void otPlatRadioClearSrcMatchShortEntries(otInstance *aInstance)
{
    (void)aInstance;

    if (sState == platformRadio_phyState_Receive
            || sState == platformRadio_phyState_Transmit)
    {
        unsigned int i;
        for (i = 0; i < PLATFORM_RADIO_SHORTADD_SRC_MATCH_NUM; i++)
        {
            /* we have a running or backgrounded rx command */
            otEXPECT(rfCoreModifySourceMatchEntry(sRfHandle, i,
                        platformRadio_address_short, false) == CMDSTA_Done);
        }
    }
    else
    {
        /* we are not running, so we can erase them ourselves */
        memset((void *)&sSrcMatchShortData, 0, sizeof(sSrcMatchShortData));
    }
exit:
    return;
}

/**
* Function documented in platform/radio.h
*/
void otPlatRadioClearSrcMatchExtEntries(otInstance *aInstance)
{
    (void)aInstance;

    if (sState == platformRadio_phyState_Receive
            || sState == platformRadio_phyState_Transmit)
    {
        unsigned int i;
        for (i = 0; i < PLATFORM_RADIO_EXTADD_SRC_MATCH_NUM; i++)
        {
            /* we have a running or backgrounded rx command */
            otEXPECT(rfCoreModifySourceMatchEntry(sRfHandle, i,
                        platformRadio_address_ext, false) == CMDSTA_Done);
        }
    }
    else
    {
        /* we are not running, so we can erase them ourselves */
        memset((void *)&sSrcMatchExtData, 0, sizeof(sSrcMatchExtData));
    }
exit:
    return;
}

/**
 * Function documented in platform/radio.h
 */
bool otPlatRadioGetPromiscuous(otInstance *aInstance)
{
    (void)aInstance;
    /* we are promiscuous if we are not filtering */
    return sReceiveCmd.frameFiltOpt.frameFiltEn == 0;
}

/**
 * Function documented in platform/radio.h
 */
void otPlatRadioSetPromiscuous(otInstance *aInstance, bool aEnable)
{
    (void)aInstance;

    if (sReceiveCmd.status == ACTIVE || sReceiveCmd.status == IEEE_SUSPENDED)
    {
        /* we have a running or backgrounded rx command */
        /* if we are promiscuous, then frame filtering should be disabled */
        otEXPECT(rfCoreModifyRxFrameFilter(sRfHandle, !aEnable)
                == RF_StatCmdDoneSuccess);
        /* XXX should we dump any queued messages ? */
    }
    else
    {
        /* if we are promiscuous, then frame filtering should be disabled */
        sReceiveCmd.frameFiltOpt.frameFiltEn = aEnable ? 0 : 1;
    }

exit:
    return;
}

/**
 * Function documented in platform/radio.h
 */
void otPlatRadioGetIeeeEui64(otInstance *aInstance, uint8_t *aIeeeEui64)
{
    uint8_t *eui64;
    unsigned int i;
    (void)aInstance;

    /* The IEEE MAC address can be stored two places. We check the Customer
     * Configuration was not set before defaulting to the Factory
     * Configuration.
     */
    eui64 = (uint8_t *)(CCFG_BASE + CCFG_O_IEEE_MAC_0);

    for (i = 0; i < OT_EXT_ADDRESS_SIZE; i++)
    {
        /* If the EUI is all ones, then the EUI is invalid, or wasn't set. */
        if (eui64[i] != 0xFF)
        {
            break;
        }
    }

    if (i >= OT_EXT_ADDRESS_SIZE)
    {
        /* The ccfg address was all 0xFF, switch to the fcfg */
        eui64 = (uint8_t *)(FCFG1_BASE + FCFG1_O_MAC_15_4_0);
    }

    /* The IEEE MAC address is stored in network byte order (big endian).
     * The caller seems to want the address stored in little endian format,
     * which is backwards of the conventions setup by @ref
     * otPlatRadioSetExtendedAddress. otPlatRadioSetExtendedAddress assumes
     * that the address being passed to it is in network byte order (big
     * endian), so the caller of otPlatRadioSetExtendedAddress must swap the
     * endianness before calling.
     *
     * It may be easier to have the caller of this function store the IEEE
     * address in network byte order (big endian).
     */
    for (i = 0; i < OT_EXT_ADDRESS_SIZE; i++)
    {
        aIeeeEui64[i] = eui64[(OT_EXT_ADDRESS_SIZE - 1) - i];
    }
}

/**
 * Function documented in platform/radio.h
 *
 * @note it is entirely possible for this function to fail, but there is no
 * valid way to return that error since the funciton prototype was changed.
 */
void otPlatRadioSetPanId(otInstance *aInstance, uint16_t aPanid)
{
    (void)aInstance;

    /* XXX: if the pan id is the broadcast pan id (0xFFFF) the auto ack will
     * not work. This is due to the design of the CM0 and follows IEEE 802.15.4
     */
    if (sState == platformRadio_phyState_Receive)
    {
        /* stop the running receive operation */
        rfCoreExecuteAbortCmd(sRfHandle, sReceiveCmdHandle);
        otEXPECT(sReceiveCmd.status != PENDING
                    && sReceiveCmd.status != ACTIVE
                    && sReceiveCmd.status != IEEE_SUSPENDED);

        sReceiveCmd.localPanID = aPanid;
        sReceiveCmdHandle = rfCoreSendReceiveCmd(sRfHandle);
    }
    else if (sState != platformRadio_phyState_Transmit)
    {
        sReceiveCmd.localPanID = aPanid;
    }

exit:
    return;
}

/**
 * Function documented in platform/radio.h
 *
 * @note it is entirely possible for this function to fail, but there is no
 * valid way to return that error since the function prototype was changed.
 */
void otPlatRadioSetExtendedAddress(otInstance *aInstance, const otExtAddress *aAddress)
{
    (void)aInstance;

    if (sState == platformRadio_phyState_Receive)
    {
        /* stop the running receive operation */
        rfCoreExecuteAbortCmd(sRfHandle, sReceiveCmdHandle);
        otEXPECT(sReceiveCmd.status != PENDING
                    && sReceiveCmd.status != ACTIVE
                    && sReceiveCmd.status != IEEE_SUSPENDED);

        sReceiveCmd.localExtAddr = *((uint64_t *)(aAddress));
        sReceiveCmdHandle = rfCoreSendReceiveCmd(sRfHandle);
    }
    else if (sState != platformRadio_phyState_Transmit)
    {
        sReceiveCmd.localExtAddr = *((uint64_t *)(aAddress));
    }

exit:
    return;
}

/**
 * Function documented in platform/radio.h
 *
 * @note it is entirely possible for this function to fail, but there is no
 * valid way to return that error since the funciton prototype was changed.
 */
void otPlatRadioSetShortAddress(otInstance *aInstance, uint16_t aAddress)
{
    (void)aInstance;

    if (sState == platformRadio_phyState_Receive)
    {
        /* stop the running receive operation */
        rfCoreExecuteAbortCmd(sRfHandle, sReceiveCmdHandle);
        otEXPECT(sReceiveCmd.status != PENDING
                    && sReceiveCmd.status != ACTIVE
                    && sReceiveCmd.status != IEEE_SUSPENDED);

        sReceiveCmd.localShortAddr = aAddress;
        sReceiveCmdHandle = rfCoreSendReceiveCmd(sRfHandle);
    }
    else if (sState != platformRadio_phyState_Transmit)
    {
        sReceiveCmd.localShortAddr = aAddress;
    }
    else
    {
        // in transmit state, optomistically hope the running RX won't die
        sReceiveCmd.localShortAddr = aAddress;
    }

exit:
    return;
}

/**
 * Function documented in platform/radio.h
 */
otError otPlatDiagRadioToneStart(otInstance *aInstance, bool aModulated)
{
    (void)aInstance;
    otError retval = OT_ERROR_NONE;

    otEXPECT_ACTION(platformRadio_phyState_Receive == sState
                        || platformRadio_phyState_Sleep == sState,
                    retval = OT_ERROR_INVALID_STATE);

    if (platformRadio_phyState_Receive == sState)
    {
        /* stop the running receive operation */
        rfCoreExecuteAbortCmd(sRfHandle, sReceiveCmdHandle);
    }

    sTxTestCmdHandle = rfCoreSendTxTestCmd(sRfHandle, aModulated);

    otEXPECT_ACTION(RF_ALLOC_ERROR != sTxTestCmdHandle, retval = OT_ERROR_FAILED);

exit:
    return retval;
}

/**
 * Function documented in platform/radio.h
 */
otError otPlatDiagRadioToneStop(otInstance *aInstance)
{
    (void)aInstance;
    otError retval = OT_ERROR_NONE;

    otEXPECT_ACTION(platformRadio_phyState_Receive == sState
                        || platformRadio_phyState_Sleep == sState,
                    retval = OT_ERROR_INVALID_STATE);

    /* stop the running receive operation */
    rfCoreExecuteAbortCmd(sRfHandle, sTxTestCmdHandle);

    if (platformRadio_phyState_Receive == sState)
    {
        /* re-issue the receive operation */
        sReceiveCmdHandle = rfCoreSendReceiveCmd(sRfHandle);
    }

exit:
    return retval;
}

static void platformRadioProcessTransmitDone(otInstance *aInstance,
                                             otRadioFrame *aTransmitFrame,
                                             otRadioFrame *aAckFrame,
                                             otError aTransmitError)
{
    /* clear the pseudo-transmit-active flag */
    sTransmitCmd.pPayload = NULL;

#if OPENTHREAD_CONFIG_DIAG_ENABLE
    if (otPlatDiagModeGet())
    {
        otPlatDiagRadioTransmitDone(aInstance, aTransmitFrame, aTransmitError);
    }
    else
#endif /* OPENTHREAD_CONFIG_DIAG_ENABLE */
    {
        otPlatRadioTxDone(aInstance, aTransmitFrame, aAckFrame, aTransmitError);
    }
}

static void platformRadioProcessReceiveDone(otInstance *aInstance,
                                            otRadioFrame *aReceiveFrame,
                                            otError aReceiveError)
{
#if OPENTHREAD_CONFIG_DIAG_ENABLE
    if (otPlatDiagModeGet())
    {
        otPlatDiagRadioReceiveDone(aInstance, aReceiveFrame, aReceiveError);
    }
    else
#endif /* OPENTHREAD_CONFIG_DIAG_ENABLE */
    {
        otPlatRadioReceiveDone(aInstance, aReceiveFrame, aReceiveError);
    }
}


/**
 * Release an entry in the RX buffer list
 */
static void releaseQueueEntry(struct rx_queue_info *p)
{
    p->curEntry->status = DATA_ENTRY_PENDING;
}

/**
 * Given a queue entry, give the next queue entry.
 */
static void nextQueueEntry(struct rx_queue_info *p)
{
    /* is this the first time? */

    if (p->startEntry == NULL)
    {
        /* yes */
        p->startEntry = p->curEntry;
        /* next */
        p->curEntry = (rfc_dataEntryGeneral_t *)(p->curEntry->pNextEntry);
        return;
    }

    /* are we done? */
    if (p->curEntry == p->startEntry)
    {
        /* YES  */
        p->curEntry = NULL;
    }
    else
    {
        /* Next */
        p->curEntry = (rfc_dataEntryGeneral_t *)(p->curEntry->pNextEntry);
    }
}

/**
 * Release the current entry and move to the next entry in the rx queue.
 */
static void releaseAndNext(struct rx_queue_info *p)
{
    releaseQueueEntry(p);
    nextQueueEntry(p);
}

static otError populateReceiveFrame(struct rx_queue_info *p)
{
    int                  len;
    int                  infoIdx;
    rfc_ieeeRxCorrCrc_t *crcCorr;
    uint8_t             *payload;
#if OPENTHREAD_CONFIG_LINK_RAW_ENABLE
    uint32_t             currTs;
    uint32_t             rxPktTs;
    uint32_t             deltaTimeUs;
    uint64_t             rtcTimestampUs;
#endif

    struct rfPktAdditionalInfo {
        uint8_t rssi;
        uint8_t crcCorr;
        uint8_t sourceIndex;
        uint32_t timestamp;
    } __attribute__ ((__packed__));

    struct rfPktAdditionalInfo *aInfo;

    /* We have received a packet
     *
     * the common case is: Success.
     *
     * Get the information appended to the end of the frame.  This
     * array access looks like it is a fencepost error, but the first
     * byte is the number of bytes that follow.
     */

    /* extract things from the payload */
    payload = &(p->curEntry->data);
    len     = payload[0] & 0x0ff;
    infoIdx = len + 1 - sizeof(struct rfPktAdditionalInfo);
    aInfo = (struct rfPktAdditionalInfo*)&(payload[infoIdx]);

    crcCorr = (rfc_ieeeRxCorrCrc_t *)&aInfo->crcCorr;

    /* construct the common case "Success" */

#if OPENTHREAD_CONFIG_LINK_RAW_ENABLE
    /*get SFD ts from the packet */

    rxPktTs = aInfo->timestamp;

    /* get current RAT time */
    currTs    = RF_getCurrentTime();
    /* packet duration in us */
    deltaTimeUs = (currTs - rxPktTs) / RF_NUM_RAT_TICKS_IN_1_US;
    /* Get current RTC ts  in us*/
    rtcTimestampUs = AONRTCCurrent64BitValueGet() >> 12;

    /*subtract packet duration to get RTC ts of the start of the packet */
    rtcTimestampUs = rtcTimestampUs - (uint64_t)deltaTimeUs;
    p->receiveFrame.mInfo.mRxInfo.mTimestamp = rtcTimestampUs;
#endif

    /*remove additional info len from pkt size */
    len -= 5;

    p->receiveFrame.mLength             = len;
    p->receiveFrame.mPsdu               = &(payload[1]);
    p->receiveFrame.mChannel            = sReceiveCmd.channel;
    p->receiveFrame.mInfo.mRxInfo.mRssi = aInfo->rssi;
    p->receiveFrame.mInfo.mRxInfo.mLqi  = crcCorr->status.corr;


    bool pend = false;

    if (aInfo->sourceIndex != 0xff)
    {
        if ((p->receiveFrame.mPsdu[1] & IEEE802154_FRAME_PENDING_MASK) >> 6)
        {
            pend = ((sSrcMatchExtData.srcPendEn[aInfo->sourceIndex / 32] & (1 << (aInfo->sourceIndex % 32))) != 0);
        }
        else
        {
            pend = ((sSrcMatchShortData.srcPendEn[aInfo->sourceIndex / 32] & (1 << (aInfo->sourceIndex % 32))) != 0);
        }
    }
    p->receiveFrame.mInfo.mRxInfo.mAckedWithFramePending = pend;

    /* if a CRC error, or invalid length occured */
    if (crcCorr->status.bCrcErr  || (len >= (OT_RADIO_FRAME_MAX_SIZE + 2)))
    {
        /* toss this packet */
        memset(&(p->receiveFrame), 0x0, sizeof(p->receiveFrame));
        return OT_ERROR_FCS;
    }

    return OT_ERROR_NONE;
}

/**
 * An RX queue entry is in the finished state, process it.
 */
static void handleRxDataFinish(struct rx_queue_info *p)
{
    otError error;
    bool    need_ack;
    bool    tx_ack_done;

    error = populateReceiveFrame(p);
    if (OT_ERROR_NONE != error)
    {
        /* Indicate a receive error to the upper layers */
        platformRadioProcessReceiveDone(p->aInstance, &(p->receiveFrame),
                error);

        releaseAndNext(p);
        return;
    }

    /* Is this an ACK frame? */
    if ((p->receiveFrame.mPsdu[0] & IEEE802154_FRAME_TYPE_MASK) == IEEE802154_FRAME_TYPE_ACK)
    {
        if (platformRadio_phyState_Transmit == sState)
        {
            /* Check if this ACK for our transmitted frame DSN */
            if (p->receiveFrame.mPsdu[IEEE802154_DSN_OFFSET] == sTransmitFrame.mPsdu[IEEE802154_DSN_OFFSET])
            {
                /* Our ACK, keep this ACK frame for later processing */
                nextQueueEntry(p);
            }
            else
            {
                /* Not our ACK, release the buffer */
                releaseAndNext(p);
            }
        }
        else
        {
            /* Any ACKs present outside TX state are either not for us, or did not get
             * processed correctly during TX state. Cleanup these ACKS */
            releaseAndNext(p);
        }
        return;
    }

    /* Does the packet require an ACK? */
    need_ack = (0 != (p->receiveFrame.mPsdu[0] & IEEE802154_ACK_REQUEST));

    /* Assuming the ACK was required, has the ack been transmitted? */
    tx_ack_done = (0 != (p->events & RF_EVENT_RX_ACK_DONE));

    if ((!need_ack) || (need_ack && tx_ack_done))
    {
        /* We do not need the ACK frame because the RX frame did not request an
         * ACK, or the RX frame requested an ACK and we have transmitted the
         * ACK. Indicate the transmission was complete to the upper layers.
         */
        platformRadioProcessReceiveDone(p->aInstance, &(p->receiveFrame),
                OT_ERROR_NONE);

        releaseAndNext(p);
    }
    else
    {
        /* The RX frame requested an ACK and we have not sent it yet. Leave the
         * frame in the queue until we have indication that we transmitted the
         * ACK.
         */
        nextQueueEntry(p);
    }
}

/**
 * An RX queue entry is in the finished state, process it.
 */
static otRadioFrame* handleRxAckFinish(struct rx_queue_info *p, uint8_t ackDsn)
{
    otRadioFrame *ackFrame = NULL;
    otError       error    = OT_ERROR_NONE;

    /* We only care about ACKs in TX state */
    if (platformRadio_phyState_Transmit != sState)
    {
        nextQueueEntry(p);
        return NULL;
    }

    error = populateReceiveFrame(p);
    if (OT_ERROR_NONE != error)
    {
        /* we only care about OK entries */
        nextQueueEntry(p);
        return NULL;
    }

    /* Is this not an ACK frame? */
    if ((p->receiveFrame.mPsdu[0] & IEEE802154_FRAME_TYPE_MASK) != IEEE802154_FRAME_TYPE_ACK)
    {
        /* we only care about ACK frames */
        nextQueueEntry(p);
        return NULL;
    }

    /*
     * Also note: Due to CSMA backoff other devices may be
     * transmitting and acking, most commonly with differing DSN numbers
     *
     * Check is this ACK for our DSN?
     */
    if (p->receiveFrame.mPsdu[IEEE802154_DSN_OFFSET] != ackDsn)
    {
        /* not our ACK, we don't want it */
        nextQueueEntry(p);
        return NULL;
    }

    /* SUCCESS */

    /* this is the ACK frame you are looking for */
    ackFrame = &(p->receiveFrame);

    return ackFrame;
}

/**
 * Empties the rx queue, regardless of the current state of the entries.
 */
static void clearRxQueue(void)
{
    rfc_dataEntryGeneral_t *curEntry   = (rfc_dataEntryGeneral_t *)sRxDataQueue.pCurrEntry;
    rfc_dataEntryGeneral_t *startEntry = curEntry;

    /* loop through receive queue */
    do
    {
        curEntry->status = DATA_ENTRY_PENDING;
        curEntry         = (rfc_dataEntryGeneral_t *)curEntry->pNextEntry;
    }
    while(curEntry != startEntry);
}

/**
 * Scan through the RX queue, looking for completed entries.
 */
static void processRxQueue(otInstance *aInstance, unsigned int events)
{
    struct rx_queue_info rqi;

    rqi.aInstance  = aInstance;
    rqi.events     = events;
    rqi.startEntry = NULL;
    rqi.curEntry   = (rfc_dataEntryGeneral_t *)sRxDataQueue.pCurrEntry;

    /* loop through receive queue */
    while (rqi.curEntry != NULL)
    {
        switch (rqi.curEntry->status)
        {
            case DATA_ENTRY_UNFINISHED:
            {
                /* the command was aborted, cleanup the entry
                 * Release this entry and move to the next entry
                 */
                releaseAndNext(&rqi);
                break;
            }

            case DATA_ENTRY_FINISHED:
            {
                /* Something is in this queue entry, process what we find */
                handleRxDataFinish(&rqi);
                break;
            }

            default:
            {
                /* Else - busy, or unused, just move to the next entry */
                nextQueueEntry(&rqi);
                break;
            }
        }
    }
}

static otRadioFrame* ackSearchRxQueue(struct rx_queue_info *rqi, unsigned int events, uint8_t ackDsn)
{
    otRadioFrame         *ackFrame = NULL;

    /* loop through receive queue */
    while (NULL != rqi->curEntry)
    {
        if (DATA_ENTRY_FINISHED == rqi->curEntry->status)
        {
            /* Something is in this queue entry, see if it is the ACK */
            ackFrame = handleRxAckFinish(rqi, ackDsn);
            if (NULL != ackFrame)
            {
                break;
            }
        }
        else
        {
            /* busy, or unused, just move to the next entry */
            nextQueueEntry(rqi);
        }
    }

    return ackFrame;
}

/**
 * Handle events in the TX state.
 */
static void handleTxState(otInstance *aInstance, unsigned int events)
{
    uint8_t               ackDsn;
    otRadioFrame         *ackFrame = NULL;
    otError               error    = OT_ERROR_NONE;
    struct rx_queue_info  rqi;
    otRadioFrame         *txFrame  = &sTransmitFrame;

    /* Save error on the stack and clear global variable */
    error          = sTransmitError;
    sTransmitError = OT_ERROR_NONE;

    /* Did we have an error of some sort? (ie: Retry, Timeout, etc) */
    otEXPECT(OT_ERROR_NONE == error);

    /* Does the packet require an ACK? If not, no need to search. */
    otEXPECT(0 != (sTransmitCmd.pPayload[0] & IEEE802154_ACK_REQUEST));

    /* need to find the ACK frame in the RX queue */
    ackDsn = sTransmitFrame.mPsdu[IEEE802154_DSN_OFFSET];

    rqi.aInstance  = aInstance;
    rqi.events     = events;
    rqi.startEntry = NULL;
    rqi.curEntry   = (rfc_dataEntryGeneral_t *)sRxDataQueue.pCurrEntry;

    ackFrame = ackSearchRxQueue(&rqi, events, ackDsn);
    otEXPECT_ACTION(NULL != ackFrame, error = OT_ERROR_NO_ACK);

exit:
    /* return to receive state */
    sState = platformRadio_phyState_Receive;

    /* Notify upper layers that transmission has finished transmission */
    platformRadioProcessTransmitDone(aInstance, txFrame, ackFrame, error);

    if (NULL != ackFrame)
    {
        /* release this queue entry and move to the next queue entry */
        releaseAndNext(&rqi);
    }
}


/**
 * Function documented in system.h
 * This is called from the main process loop.
 */
void platformRadioProcess(otInstance *aInstance)
{
    uintptr_t arg;
    /*
     * XXX: Temporary until we reconcile how the TI-OpenThread examples use
     * the Thread Task for scheduling.
     */
    if (pdFALSE == xQueueReceive(sRadioEventQueue, &arg, 0U))
    {
        // nothing to process
        return;
    }
    else
    {
        // just claim we need more processing, hacky
        platformRadioSignal();
    }

    /* Handle the events based on the radio state */
    switch (sState)
    {
        case platformRadio_phyState_Sleep:
        {
            if (arg & RF_EVENT_SLEEP_YIELD)
            {
                /* we have not been thrashed back into receive state, actually
                 * release the RFC and clear the rx queue.
                 */
                clearRxQueue();
                RF_yield(sRfHandle);
            }
            if (arg & (RF_EVENT_RX_DONE | RF_EVENT_RX_ACK_DONE))
            {
                /* Unfortunately, the frame must be discarded or we risk
                 * asserting the MAC.
                 */
                clearRxQueue();
            }
            break;
        }

        case platformRadio_phyState_Transmit:
        {
            if (arg & (RF_EVENT_TX_CMD_PREEMPTED | RF_EVENT_RX_CMD_PREEMPTED))
            {
                /* The RF driver has preempted the RX command. Check if the RX
                 * is running and re-submit the command.
                 */
                if (sReceiveCmd.status != PENDING
                        && sReceiveCmd.status != ACTIVE
                        && sReceiveCmd.status != IEEE_SUSPENDED)
                {
                    sReceiveCmdHandle = rfCoreSendReceiveCmd(sRfHandle);
                }
                /* If any of the transmit command string finished without success.
                 * XXX: order is important because these are updated by a coprocessor.
                 */
                if ((sCsmaBackoffCmd.status != ACTIVE                           // csma is running
                        && sCsmaBackoffCmd.status != IEEE_DONE_OK)              // csma was successful
                    || (
                        sTransmitCmd.status != IDLE                             // tx is waiting
                        && sTransmitCmd.status != ACTIVE                        // tx is running
                        && sTransmitCmd.status != IEEE_DONE_OK)                 // tx was successful
                    || (
                        (sTransmitCmd.pPayload[0] & IEEE802154_ACK_REQUEST)     // if an ACK was requested
                        && sTransmitRxAckCmd.status != IDLE                     // rx ACK is waiting
                        && sTransmitRxAckCmd.status != ACTIVE                   // rx ACK is running
                        && sTransmitRxAckCmd.status != IEEE_DONE_ACK            // success ACK frame
                        && sTransmitRxAckCmd.status != IEEE_DONE_ACKPEND        // success ACK frame with pend bit
                        && sTransmitRxAckCmd.status != IEEE_DONE_TIMEOUT)       // "success" NO ACK, cmd successful
                    )
                {
                    /* The RF driver has preempted the TX command string. This
                     * is likely due to a temperature change preemption.
                     * Notify of a TX failure and let retries restart the
                     * command. It is very unlikely this will happen due to the
                     * delay in scheduling of a command.
                     */
                    sTransmitError = OT_ERROR_ABORT;
                    handleTxState(aInstance, RF_EVENT_TX_DONE);
                }
            }
            else
            {
                /* The TX command string has finished */
                if (arg & RF_EVENT_TX_DONE)
                {
                    handleTxState(aInstance, arg);
                }

                /* Handle new received frame */
                if (arg & (RF_EVENT_RX_DONE | RF_EVENT_RX_ACK_DONE))
                {
                    processRxQueue(aInstance, arg);
                }

                /* Clear the receive buffer if the radio can't find space to put RX frames */
                if (arg & RF_EVENT_BUF_FULL)
                {
                    clearRxQueue();
                }
            }
            break;
        }

        case platformRadio_phyState_EdScan:
        {
            if (arg & RF_EVENT_ED_SCAN_DONE)
            {
                sState = platformRadio_phyState_Receive;

                /* restart receive command if necessary */
                if (sReceiveCmd.status != PENDING
                        && sReceiveCmd.status != ACTIVE
                        && sReceiveCmd.status != IEEE_SUSPENDED)
                {
                    sReceiveCmdHandle = rfCoreSendReceiveCmd(sRfHandle);
                }

                if (sEdScanCmd.status == IEEE_DONE_OK)
                {
                    otPlatRadioEnergyScanDone(aInstance, sEdScanCmd.maxRssi);
                }
                else
                {
                    otPlatRadioEnergyScanDone(aInstance, PLATFORM_RADIO_INVALID_RSSI);
                }
            }
            /* fall through */
        }

        case platformRadio_phyState_Receive:
        {
            if (arg & RF_EVENT_RX_CMD_PREEMPTED)
            {
                /* The RF driver has preempted the RX command. Check if the RX
                 * is running and re-submit the command.
                 */
                if (sReceiveCmd.status != PENDING
                        && sReceiveCmd.status != ACTIVE
                        && sReceiveCmd.status != IEEE_SUSPENDED)
                {
                    sReceiveCmdHandle = rfCoreSendReceiveCmd(sRfHandle);
                }
            }

            /* Handle new received frame */
            if (arg & (RF_EVENT_RX_DONE | RF_EVENT_RX_ACK_DONE))
            {
                processRxQueue(aInstance, arg);
            }

            /* Clear the receive buffer if the radio can't find space to put RX frames */
            if (arg & RF_EVENT_BUF_FULL)
            {
                clearRxQueue();
            }
            
            /* Re-start the RX command since we are still in the state. */
            if (arg & RF_EVENT_RX_CMD_STOP)
            {
                if (sReceiveCmd.status != PENDING
                        && sReceiveCmd.status != ACTIVE
                        && sReceiveCmd.status != IEEE_SUSPENDED)
                {
                    sReceiveCmdHandle = rfCoreSendReceiveCmd(sRfHandle);
                }
            }
            break;
        }

        case platformRadio_phyState_Disabled:
        default:
            break;
    }
}

