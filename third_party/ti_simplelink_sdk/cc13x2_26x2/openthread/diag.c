/******************************************************************************

 @file diag.c

 @brief platform specific diag functions.

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

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <openthread/platform/alarm-milli.h>
#include <openthread/platform/diag.h>
#include <openthread/platform/radio.h>

#include <utils/code_utils.h>

#include "radio.h"

/**
 * Window for rx frame to be counted as lost instead of RX nOK.
 *
 * Workaround for issue of receiving valid 802.15.4 frames counts ad very large
 * sequence numbers.
 */
#define PLAT_DIAG_RX_SEQNO_WINDOW 100

/**
 * Default interframe spacing used for transmission command.
 */
#define PLAT_DIAG_TX_INTERFRAME 100 // ms

/**
 * Default packet size used for transmission command.
 */
#define PLAT_DIAG_TX_PACKETSIZE 30

/**
 * Diagnostics mode variables.
 */
static bool PlatDiag_diagEnabled = false;
static bool PlatDiag_rxEnabled   = false;
static bool PlatDiag_txEnabled   = false;

/**
 * `diag transmit` variables.
 */
static uint32_t PlatDiag_txPeriod;
static uint16_t PlatDiag_txSentCount;
static uint16_t PlatDiag_txFrameSize;
static uint16_t PlatDiag_txFrameCount;

/**
 * `diag receive` variables.
 */
static uint16_t PlatDiag_rxExpectedSeqNo;
static uint16_t PlatDiag_rxFrameCount;
static uint16_t PlatDiag_rxLostFrames;
static uint16_t PlatDiag_rxNokCount;
static uint16_t PlatDiag_rxReceivedCount;

/**
 * Helper function to parse strings into long variables and mark errors.
 *
 * @param[in]  aArgVector   C string to parse.
 * @param[out] aValue       address of long container.
 *
 * @retval OT_ERROR_NONE    The string was parsed correctly.
 * @retval OT_ERROR_PARSE   The string was not formatted correctly.
 */
otError PlatDiag_parseLong(char * aArgVector, long * aValue)
{
    char * endptr;
    *aValue = strtol(aArgVector, &endptr, 0);
    return (*endptr == '\0') ? OT_ERROR_NONE : OT_ERROR_PARSE;
}

/**
 * Helper function to calculate the Packet Error Rate.
 *
 * PER value based on @ref PlatDiag_rxFrameCount @ref PlatDiag_rxReceivedCount
 * @ref PlatDiag_rxNokCount and @ref PlatDiag_rxLostFrames.
 *
 * @return Packet Error Rate.
 */
unsigned int PlatDiag_calculatePER(void)
{
    unsigned int packetErrorRate;

    if (PlatDiag_rxFrameCount != 0)
    {
        packetErrorRate = (((long) PlatDiag_rxFrameCount - (long) PlatDiag_rxReceivedCount) * 100 / (long) PlatDiag_rxFrameCount);
    }
    else
    {
        if (PlatDiag_rxReceivedCount == 0)
            packetErrorRate = 100;
        else
        {
            packetErrorRate = (((long) PlatDiag_rxNokCount + (long) PlatDiag_rxLostFrames) * 100 /
                               ((long) PlatDiag_rxReceivedCount + (long) PlatDiag_rxNokCount + (long) PlatDiag_rxLostFrames));
        }
    }

    return packetErrorRate;
}

/**
 * Process the `diag receive` command.
 *
 * @param[in]  aInstance        OpenThread instance structure.
 * @param[in]  argc             Count of command arguments.
 * @param[in]  argv             Array of command arguments.
 * @param[out] aOutput          Output buffer used for user interaction.
 * @param[in]  aOutputMaxLen    Size of the Output buffer.
 *
 * @return Error value from parsing or executing the command.
 */
otError PlatDiag_processReceive(otInstance * aInstance, int argc, char * argv[], char * aOutput, size_t aOutputMaxLen)
{
    otError retval = OT_ERROR_INVALID_ARGS;

    if (argc >= 1)
    {
        if (strcmp(argv[0], "start") == 0)
        {
            long expectedCount;

            otEXPECT_ACTION(true == PlatDiag_diagEnabled && false == PlatDiag_rxEnabled && false == PlatDiag_txEnabled,
                            retval = OT_ERROR_INVALID_STATE);

            if (argc >= 2)
            {
                retval = PlatDiag_parseLong(argv[1], &expectedCount);
                otEXPECT(OT_ERROR_NONE == retval);
            }
            else
            {
                expectedCount = 0;
            }

            PlatDiag_rxEnabled       = true;
            PlatDiag_rxExpectedSeqNo = 0;
            PlatDiag_rxFrameCount    = expectedCount;
            PlatDiag_rxReceivedCount = 0;
            PlatDiag_rxNokCount      = 0;
            PlatDiag_rxLostFrames    = 0;

            retval = OT_ERROR_NONE;
            snprintf(aOutput, aOutputMaxLen,
                     "packet reception is started\r\n"
                     "status 0x%02x\r\n",
                     retval);
        }
        else if (strcmp(argv[0], "stop") == 0)
        {
            unsigned int packetErrorRate;

            otEXPECT(true == PlatDiag_diagEnabled);

            packetErrorRate = PlatDiag_calculatePER();
            retval          = OT_ERROR_NONE;

            PlatDiag_rxEnabled = false;
            snprintf(aOutput, aOutputMaxLen,
                     "packet reception is stopped\r\n"
                     "received frame: 0x%04x\r\n"
                     "received OK: 0x%04x\r\n"
                     "received nOK: 0x%04x\r\n"
                     "lost: 0x%04x\r\n"
                     "Packet Error Rate: %d%%\r\n"
                     "status 0x%02x\r\n",
                     PlatDiag_rxFrameCount, PlatDiag_rxReceivedCount, PlatDiag_rxNokCount, PlatDiag_rxLostFrames, packetErrorRate,
                     retval);
        }
    }

exit:
    return retval;
}

/**
 * Process the `diag transmit` command.
 *
 * @param[in]  aInstance        OpenThread instance structure.
 * @param[in]  argc             Count of command arguments.
 * @param[in]  argv             Array of command arguments.
 * @param[out] aOutput          Output buffer used for user interaction.
 * @param[in]  aOutputMaxLen    Size of the Output buffer.
 *
 * @return Error value from parsing or executing the command.
 */
otError PlatDiag_processTransmit(otInstance * aInstance, int argc, char * argv[], char * aOutput, size_t aOutputMaxLen)
{
    otError retval = OT_ERROR_INVALID_ARGS;

    if (argc >= 1)
    {
        if (strcmp(argv[0], "start") == 0)
        {
            long packetSize;
            long interframeSpace;
            long transmitCount;

            otEXPECT_ACTION(true == PlatDiag_diagEnabled && false == PlatDiag_txEnabled && false == PlatDiag_rxEnabled,
                            retval = OT_ERROR_INVALID_STATE);

            if (argc >= 2)
            {
                retval = PlatDiag_parseLong(argv[1], &packetSize);
                otEXPECT(OT_ERROR_NONE == retval);
            }
            else
            {
                packetSize = PLAT_DIAG_TX_PACKETSIZE;
            }

            if (argc >= 3)
            {
                retval = PlatDiag_parseLong(argv[2], &interframeSpace);
                otEXPECT(OT_ERROR_NONE == retval);
            }
            else
            {
                interframeSpace = PLAT_DIAG_TX_INTERFRAME;
            }

            if (argc >= 4)
            {
                retval = PlatDiag_parseLong(argv[3], &transmitCount);
                otEXPECT(OT_ERROR_NONE == retval);
            }
            else
            {
                transmitCount = 0; /* XXX: `0` used as continuous transmission */
            }

            PlatDiag_txEnabled    = true;
            PlatDiag_txFrameSize  = packetSize;
            PlatDiag_txPeriod     = interframeSpace;
            PlatDiag_txFrameCount = transmitCount;
            PlatDiag_txSentCount  = 0;

            otPlatAlarmMilliStartAt(aInstance, otPlatAlarmMilliGetNow(), PlatDiag_txPeriod);

            retval = OT_ERROR_NONE;
            snprintf(aOutput, aOutputMaxLen,
                     "packet transmission is started\r\n"
                     "status 0x%02x\r\n",
                     retval);
        }
        else if (strcmp(argv[0], "stop") == 0)
        {
            otEXPECT_ACTION(true == PlatDiag_diagEnabled && true == PlatDiag_txEnabled, retval = OT_ERROR_INVALID_STATE);

            PlatDiag_txEnabled = false;
            otPlatAlarmMilliStop(aInstance);
            retval = OT_ERROR_NONE;

            snprintf(aOutput, aOutputMaxLen,
                     "packet transmission is stopped\r\n"
                     "transmitted: 0x%04x\r\n"
                     "status 0x%02x\r\n",
                     PlatDiag_txSentCount, retval);
        }
    }

exit:
    return retval;
}

/**
 * Process the `diag tone` command.
 *
 * @param[in]  aInstance        OpenThread instance structure.
 * @param[in]  argc             Count of command arguments.
 * @param[in]  argv             Array of command arguments.
 * @param[out] aOutput          Output buffer used for user interaction.
 * @param[in]  aOutputMaxLen    Size of the Output buffer.
 *
 * @return Error value from parsing or executing the command.
 */
otError PlatDiag_processTone(otInstance * aInstance, int argc, char * argv[], char * aOutput, size_t aOutputMaxLen)
{
    otError retval = OT_ERROR_INVALID_ARGS;

    otEXPECT(true == PlatDiag_diagEnabled);
    if (argc >= 1)
    {
        if (strcmp(argv[0], "start") == 0)
        {
            bool modulated = false;

            if (argc == 2)
            {
                modulated = (strcmp(argv[1], "mod") == 0);
            }
            retval = otPlatDiagRadioToneStart(aInstance, modulated);
            otEXPECT(OT_ERROR_NONE == retval);

            snprintf(aOutput, aOutputMaxLen, "continuous %s tone started\r\n", modulated ? "modulated" : "unmodulated");
        }
        else if (strcmp(argv[0], "stop") == 0)
        {
            retval = otPlatDiagRadioToneStop(aInstance);
            otEXPECT(OT_ERROR_NONE == retval);

            snprintf(aOutput, aOutputMaxLen, "continuous tone stopped\r\n");
        }
    }

exit:
    return retval;
}

/**
 * Process the `diag shield` command.
 *
 * @param[in]  aInstance        OpenThread instance structure.
 * @param[in]  argc             Count of command arguments.
 * @param[in]  argv             Array of command arguments.
 * @param[out] aOutput          Output buffer used for user interaction.
 * @param[in]  aOutputMaxLen    Size of the Output buffer.
 *
 * @return Error value from parsing or executing the command.
 */
otError PlatDiag_processShield(otInstance * aInstance, int argc, char * argv[], char * aOutput, size_t aOutputMaxLen)
{
    otError retval = OT_ERROR_INVALID_ARGS;
    if (argc == 2)
    {
        long channel;

        retval = PlatDiag_parseLong(argv[1], &channel);
        otEXPECT(OT_ERROR_NONE == retval);

        if (strcmp(argv[0], "start") == 0)
        {
            rfCoreDiagChannelDisable(channel);
            snprintf(aOutput, aOutputMaxLen, "Shield started\r\n");
        }
        else if (strcmp(argv[0], "stop") == 0)
        {
            rfCoreDiagChannelEnable(channel);
            snprintf(aOutput, aOutputMaxLen, "Shield stopped\r\n");
        }
        else
        {
            retval = OT_ERROR_INVALID_ARGS;
        }
    }

exit:
    return retval;
}

/**
 * Documented in <openthread/platform/diag.h>
 */
otError otPlatDiagProcess(otInstance * aInstance, uint8_t argc, char * argv[], char * aOutput, size_t aOutputMaxLen)
{
    otError retval = OT_ERROR_NONE;

    if (argc >= 1)
    {
        if (strcmp(argv[0], "tone") == 0)
        {
            retval = PlatDiag_processTone(aInstance, argc - 1, (argc > 1) ? &argv[1] : NULL, aOutput, aOutputMaxLen);
        }
        else if (strcmp(argv[0], "receive") == 0)
        {
            retval = PlatDiag_processReceive(aInstance, argc - 1, (argc > 1) ? &argv[1] : NULL, aOutput, aOutputMaxLen);
        }
        else if (strcmp(argv[0], "transmit") == 0)
        {
            retval = PlatDiag_processTransmit(aInstance, argc - 1, (argc > 1) ? &argv[1] : NULL, aOutput, aOutputMaxLen);
        }
        else if (strcmp(argv[0], "shield") == 0)
        {
            retval = PlatDiag_processShield(aInstance, argc - 1, (argc > 1) ? &argv[1] : NULL, aOutput, aOutputMaxLen);
        }
        else
        {
            snprintf(aOutput, aOutputMaxLen, "diag feature '%s' is not supported\r\n", argv[0]);
        }
    }

    if (retval != OT_ERROR_NONE)
    {
        snprintf(aOutput, aOutputMaxLen, "failed\r\nstatus %#x\r\n", retval);
    }

    return retval;
}

/**
 * Documented in <openthread/platform/diag.h>
 */
void otPlatDiagModeSet(bool aMode)
{
    PlatDiag_diagEnabled = aMode;
}

/**
 * Documented in <openthread/platform/diag.h>
 */
bool otPlatDiagModeGet()
{
    return PlatDiag_diagEnabled;
}

/**
 * Documented in <openthread/platform/diag.h>
 */
void otPlatDiagChannelSet(uint8_t aChannel)
{
    /* factory diag handles the necessary calls */
}

/**
 * Documented in <openthread/platform/diag.h>
 */
void otPlatDiagTxPowerSet(int8_t aTxPower)
{
    /* factory diag handles the necessary calls */
}

/**
 * Documented in <openthread/platform/diag.h>
 */
void otPlatDiagRadioReceived(otInstance * aInstance, otRadioFrame * aFrame, otError aError)
{
    (void) aInstance;

    if (PlatDiag_rxEnabled)
    {
        if (OT_ERROR_NONE == aError)
        {
            uint16_t seqNo = (aFrame->mPsdu[0] << 8 & 0xFF00) | (aFrame->mPsdu[1] & 0x00FF);

            if (PlatDiag_rxFrameCount != 0 && PlatDiag_rxReceivedCount >= PlatDiag_rxFrameCount)
            {
                /* Recieved enough frames, stop */
                PlatDiag_rxEnabled = false;
            }

            if (seqNo == PlatDiag_rxExpectedSeqNo)
            {
                PlatDiag_rxReceivedCount++;
                PlatDiag_rxExpectedSeqNo = seqNo + 1;
            }
            else
            {
                uint16_t seqNoWindow = (0 == PlatDiag_rxFrameCount ? PLAT_DIAG_RX_SEQNO_WINDOW : PlatDiag_rxFrameCount);
                /* Both are unsigned, we are looking for positive difference
                 * only.
                 */
                uint16_t missedFrames = seqNo - PlatDiag_rxExpectedSeqNo;

                if (missedFrames < seqNoWindow)
                {
                    /* Sequnce number is just off enough to make us think we
                     * actually lost these frames.
                     */
                    PlatDiag_rxLostFrames += missedFrames;
                    PlatDiag_rxReceivedCount++;
                    PlatDiag_rxExpectedSeqNo = seqNo + 1;
                }
                else
                {
                    PlatDiag_rxNokCount++;
                }
            }
        }
        else
        {
            PlatDiag_rxNokCount++;
        }
    }
}

/**
 * Documented in <openthread/platform/diag.h>
 */
void otPlatDiagAlarmCallback(otInstance * aInstance)
{
    if (PlatDiag_txEnabled)
    {
        if (PlatDiag_txFrameCount == 0 || PlatDiag_txSentCount < PlatDiag_txFrameCount)
        {
            unsigned int i;
            otRadioFrame * packet = otPlatRadioGetTransmitBuffer(aInstance);

            packet->mLength = PlatDiag_txFrameSize;

            for (i = 0; i < packet->mLength; i++)
            {
                packet->mPsdu[i] = i;
            }

            packet->mPsdu[0] = (PlatDiag_txSentCount >> 8) & 0xFF;
            packet->mPsdu[1] = PlatDiag_txSentCount & 0xFF;

            otPlatRadioTransmit(aInstance, packet);
            PlatDiag_txSentCount++;
            otPlatAlarmMilliStartAt(aInstance, otPlatAlarmMilliGetNow(), PlatDiag_txPeriod);
        }
        else
        {
        }
    }
}

#if !OPENTHREAD_CONFIG_DIAG_ENABLE
/**
 * Documented in <openthread/platform/diag.h>
 */
bool otDiagIsEnabled(otInstance * aInstance)
{

    return false;
}
#endif /* OPENTHREAD_CONFIG_DIAG_ENABLE */
