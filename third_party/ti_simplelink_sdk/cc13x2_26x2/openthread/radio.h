/******************************************************************************

 @file radio.h

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

// name collision with openthread/radio.h
#ifndef PLATFORM_RADIO_H_
#define PLATFORM_RADIO_H_

// clang-format off
// `/` is a path delimiter, not the division operation

#include <ti/devices/DeviceFamily.h>
#include DeviceFamily_constructPath(driverlib/rf_ieee_cmd.h)

// clang-format on

#include <ti/drivers/rf/RF.h>

#include <openthread/instance.h>

/**
 * Size of the receive buffers in the receive queue.
 */
#define RX_BUF_SIZE 148

/**
 * Value to pass to `RF_cancelCmd` to signify aborting the command.
 *
 * documented in `source/ti/drivers/rf/RF.h`
 */
#define RF_DRIVER_ABORT 0

/**
 * Return value used when searching the source match array.
 *
 * Returned if an address could not be found or if an empty element could not
 * be found.
 */
#define PLATFORM_RADIO_SRC_MATCH_NONE 0xFF

/**
 * Number of extended addresses in @ref ext_src_match_data_t.
 */
#define PLATFORM_RADIO_EXTADD_SRC_MATCH_NUM 10

/**
 * Number of short addresses in @ref short_src_match_data_t.
 */
#define PLATFORM_RADIO_SHORTADD_SRC_MATCH_NUM 10

/**
 * size of length field in receive struct.
 *
 * defined in Table 23-10 of the cc13xx and cc26xx TRM.
 */
#define DATA_ENTRY_LENSZ_BYTE 1

/**
 * Event flags for the radio process function
 */
#define RF_EVENT_TX_DONE (1U << 0)
#define RF_EVENT_ED_SCAN_DONE (1U << 1)
#define RF_EVENT_RX_DONE (1U << 2)
#define RF_EVENT_RX_ACK_DONE (1U << 3)
#define RF_EVENT_SLEEP_YIELD (1U << 4)
#define RF_EVENT_BUF_FULL (1U << 5)
#define RF_EVENT_RX_CMD_STOP (1U << 6)
#define RF_EVENT_RX_CMD_PREEMPTED (1U << 7)
#define RF_EVENT_TX_CMD_PREEMPTED (1U << 8)

/**
 * (IEEE 802.15.4-2006) PSDU.FCF.frameType.
 */
#define IEEE802154_FRAME_TYPE_MASK (0x7)

/**
 * (IEEE 802.15.4-2006) PSDU.FCF.framePending.
 */
#define IEEE802154_FRAME_PENDING_MASK (0x40)

/**
 * (IEEE 802.15.4-2006) frame type: ACK.
 */
#define IEEE802154_FRAME_TYPE_ACK (0x2)

/**
 * (IEEE 802.15.4-2006) PSDU.FCF.bFramePending.
 */
#define IEEE802154_FRAME_PENDING (1 << 4)

/**
 * (IEEE 802.15.4-2006) Length of an ack frame.
 */
#define IEEE802154_ACK_LENGTH (5)

/**
 * (IEEE 802.15.4-2006) PSDU.FCF.bAR.
 */
#define IEEE802154_ACK_REQUEST (1 << 5)

/**
 * (IEEE 802.15.4-2006) PSDU.sequenceNumber.
 */
#define IEEE802154_DSN_OFFSET (2)

/**
 * (IEEE 802.15.4-2006) macMinBE.
 */
#define IEEE802154_MAC_MIN_BE (3)

/**
 * (IEEE 802.15.4-2006) macMaxBE.
 */
#define IEEE802154_MAC_MAX_BE (5)

/**
 * (IEEE 802.15.4-2006) macMaxCSMABackoffs.
 */
#define IEEE802154_MAC_MAX_CSMA_BACKOFFS (4)

/**
 * (IEEE 802.15.4-2006) macMaxFrameRetries.
 */
#define IEEE802154_MAC_MAX_FRAMES_RETRIES (3)

/**
 * (IEEE 802.15.4-2006 7.4.1) MAC constants.
 */
#define IEEE802154_A_UINT_BACKOFF_PERIOD (20)

/**
 * (IEEE 802.15.4-2006 6.4.1) PHY constants.
 */
#define IEEE802154_A_TURNAROUND_TIME (12)

/**
 * (IEEE 802.15.4-2006 6.4.2) PHY PIB attribute, specifically the O-QPSK PHY.
 */
#define IEEE802154_PHY_SHR_DURATION (10)

/**
 * (IEEE 802.15.4-2006 6.4.2) PHY PIB attribute, specifically the O-QPSK PHY.
 */
#define IEEE802154_PHY_SYMBOLS_PER_OCTET (2)

/**
 * (IEEE 802.15.4-2006 7.4.2) macAckWaitDuration PIB attribute.
 */
#define IEEE802154_MAC_ACK_WAIT_DURATION                                                                                           \
    (IEEE802154_A_UINT_BACKOFF_PERIOD + IEEE802154_A_TURNAROUND_TIME + IEEE802154_PHY_SHR_DURATION +                               \
     (6 * IEEE802154_PHY_SYMBOLS_PER_OCTET))

/**
 * (IEEE 802.15.4-2006 6.5.3.2) O-QPSK symbol rate.
 */
#define IEEE802154_SYMBOLS_PER_SEC (62500)

/**
 * Frequency of the Radio Timer module.
 *
 * 4MHz clock.
 */
#define PLATFORM_RADIO_RAT_TICKS_PER_SEC (4000000)

/**
 * Invalid RSSI value returned from an ED scan.
 */
#define PLATFORM_RADIO_INVALID_RSSI (127)

/**
 * Structure for source matching extended addresses.
 *
 * Defined in Table 23-73 of the cc13xx and cc26xx TRM.
 */
typedef struct __RFC_STRUCT ext_src_match_data_s ext_src_match_data_t;
struct __RFC_STRUCT ext_src_match_data_s
{
    uint32_t srcMatchEn[((PLATFORM_RADIO_EXTADD_SRC_MATCH_NUM + 31) / 32)];
    uint32_t srcPendEn[((PLATFORM_RADIO_EXTADD_SRC_MATCH_NUM + 31) / 32)];
    uint64_t extAddrEnt[PLATFORM_RADIO_EXTADD_SRC_MATCH_NUM];
} __RFC_STRUCT_ATTR;

/**
 * Structure for source matching short addresses.
 *
 * Defined in Table 23-74 of the cc13xx and cc26xx TRM.
 */
typedef struct __RFC_STRUCT short_src_match_data_s short_src_match_data_t;
struct __RFC_STRUCT short_src_match_data_s
{
    uint32_t srcMatchEn[((PLATFORM_RADIO_SHORTADD_SRC_MATCH_NUM + 31) / 32)];
    uint32_t srcPendEn[((PLATFORM_RADIO_SHORTADD_SRC_MATCH_NUM + 31) / 32)];
    rfc_shortAddrEntry_t shortAddrEnt[PLATFORM_RADIO_SHORTADD_SRC_MATCH_NUM];
} __RFC_STRUCT_ATTR;

/**
 * Enum for specifying short/ext address type
 */
typedef enum platformRadio_address
{
    platformRadio_address_short = 0,
    platformRadio_address_ext   = 1,
} platformRadio_address;

/**
 * This enum represents the state of a radio.
 *
 * Initially, a radio is in the Disabled state.
 *
 * The following are valid radio state transitions for the platform
 *
 * ```
 *  +----------+  Enable()  +-------+  Receive()   +---------+   Transmit()  +----------+
 *  |          |----------->|       |------------->|         |-------------->|          |
 *  | Disabled |            | Sleep |              | Receive |               | Transmit |
 *  |          |<-----------|       |<-------------|         |<--------------|          |
 *  +----------+  Disable() |       |   Sleep()    +---------+ Receive() or  +----------+
 *    ^                     |       |                    | ^   Transmit complete
 *    |                     |       |       EnergyScan() | |
 *    |                     |       |                    V | Scan Complete
 *    |                     |       | EnergyScan()  +--------+
 *  Init()                  |       |-------------->| EdScan |
 *                          +-------+               +--------+
 * ```
 *
 * These states slightly differ from the states in include/openthread/platform/radio.h
 * from OpenThread. The additional states the phy can be in are due to the asynchronous
 * nature of the CM0 radio core.
 *
 * | state            | description                                        |
 * |------------------|----------------------------------------------------|
 * | Disabled         | The rfcore powerdomain is off and the RFCPE is off |
 * | Sleep            | The RFCORE PD is on, and the RFCPE is in IEEE mode |
 * | Receive          | The RFCPE is running a CMD_IEEE_RX                 |
 * | EdScan           | The RFCPE is running a CMD_IEEE_ED_SCAN            |
 * | Transmit         | The RFCPE is running a transmit command string     |
 *
 */
typedef enum platformRadio_phyState
{
    platformRadio_phyState_Disabled = 0,
    platformRadio_phyState_Sleep,
    platformRadio_phyState_Receive,
    platformRadio_phyState_EdScan,
    platformRadio_phyState_Transmit,
} platformRadio_phyState;

/**
 * The diagnostic module calls this function to begin transmitting a continuous tone. The tone will be transmitted on
 * the current receive channel.
 *
 * @param[in]  aInstance      The OpenThread instance structure.
 * @param[in]  aModulated     Indicates whether or not the tone was modulated or not.
 *
 * @retval OT_ERROR_NONE             Successfully started sending the RF tone.
 * @retval OT_ERROR_NOT_IMPLEMENTED  The radio doesn't support sending a test tone.
 */
otError otPlatDiagRadioToneStart(otInstance * aInstance, bool aModulated);

/**
 * The diagnostic module calls this to stop transmitting a continuous tone.
 *
 * @param[in]  aInstance      The OpenThread instance structure.
 *
 * @retval OT_ERROR_NONE             Successfully stopped sending the RF tone.
 * @retval OT_ERROR_NOT_IMPLEMENTED  The radio doesn't support sending a test tone.
 */
otError otPlatDiagRadioToneStop(otInstance * aInstance);

/**
 * The diagnostic module calls this to set the channel and disable channel switching.
 *
 * This function was added to support automated Thread Test Harness execution.
 * It is intended to simulate the device being placed within a shield box by
 * switching the radio to an unused channel and disallowing the stack switching
 * back to the set channel.
 *
 * @param[in]  aChannel The new (empty) channel.
 *
 */
void rfCoreDiagChannelDisable(uint8_t aChannel);

/**
 * The diagnostic module calls this to reset the channel and re-enable channel switching.
 *
 * @sa rfCoreDiagChannelDisable
 *
 * @param[in]  aChannel The original channel.
 *
 */
void rfCoreDiagChannelEnable(uint8_t aChannel);

#endif /* PLATFORM_RADIO_H_ */
