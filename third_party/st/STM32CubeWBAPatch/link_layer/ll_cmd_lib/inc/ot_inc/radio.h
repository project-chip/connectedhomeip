/*$Id: //dwh/bluetooth/DWC_ble154combo/firmware/rel/2.00a-lca01/firmware/public_inc/ot_inc/radio.h#1 $*/
/*
 *  Copyright (c) 2016, The OpenThread Authors.
 *  All rights reserved.
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions are met:
 *  1. Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *  2. Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *  3. Neither the name of the copyright holder nor the
 *     names of its contributors may be used to endorse or promote products
 *     derived from this software without specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 *  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 *  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 *  ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 *  LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 *  CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 *  SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 *  INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 *  CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 *  ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 *  POSSIBILITY OF SUCH DAMAGE.
 */

/**
 * @file
 * @brief
 *   This file defines the radio interface for OpenThread.
 *
 */

#ifndef OPENTHREAD_PLATFORM_RADIO_H_
#define OPENTHREAD_PLATFORM_RADIO_H_

#include <stdint.h>

#include "error.h"
#include "instance.h"
#include "crypto.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @addtogroup plat-radio
 *
 * @brief
 *   This module includes the platform abstraction for radio communication.
 *
 * @{
 *
 */

/**
 * @defgroup radio-types Radio Types
 *
 * @brief
 *   This module includes the platform abstraction for a radio frame.
 *
 * @{
 *
 */

enum
{
    OT_RADIO_FRAME_MAX_SIZE = 127, ///< aMaxPHYPacketSize (IEEE 802.15.4-2006)
    OT_RADIO_FRAME_MIN_SIZE = 3,   ///< Minimal size of frame FCS + CONTROL

    OT_RADIO_SYMBOLS_PER_OCTET = 2,      ///< 2.4 GHz IEEE 802.15.4-2006
    OT_RADIO_BIT_RATE          = 250000, ///< 2.4 GHz IEEE 802.15.4 (bits per second)
    OT_RADIO_BITS_PER_OCTET    = 8,      ///< Number of bits per octet

    // Per IEEE 802.15.4-2015, 12.3.3 Symbol rate:
    // The O-QPSK PHY symbol rate shall be 25 ksymbol/s when operating in the 868 MHz band and 62.5 ksymbol/s when
    // operating in the 780 MHz, 915 MHz, 2380 MHz, or 2450 MHz band
    OT_RADIO_SYMBOL_RATE = 62500, ///< The O-QPSK PHY symbol rate when operating in the 780MHz, 915MHz, 2380MHz, 2450MHz
    OT_RADIO_SYMBOL_TIME = 1000000 * 1 / OT_RADIO_SYMBOL_RATE, ///< Symbol duration time in unit of microseconds
    OT_RADIO_TEN_SYMBOLS_TIME = 10 * OT_RADIO_SYMBOL_TIME,     ///< Time for 10 symbols in unit of microseconds

    OT_RADIO_LQI_NONE      = 0,   ///< LQI measurement not supported
    OT_RADIO_RSSI_INVALID  = 127, ///< Invalid or unknown RSSI value
    OT_RADIO_POWER_INVALID = 127, ///< Invalid or unknown power value
};

/**
 * Defines the channel page.
 *
 */
enum
{
    OT_RADIO_CHANNEL_PAGE_0      = 0,                               ///< 2.4 GHz IEEE 802.15.4-2006
    OT_RADIO_CHANNEL_PAGE_0_MASK = (1U << OT_RADIO_CHANNEL_PAGE_0), ///< 2.4 GHz IEEE 802.15.4-2006
    OT_RADIO_CHANNEL_PAGE_2      = 2,                               ///< 915 MHz IEEE 802.15.4-2006
    OT_RADIO_CHANNEL_PAGE_2_MASK = (1U << OT_RADIO_CHANNEL_PAGE_2), ///< 915 MHz IEEE 802.15.4-2006
};

/**
 * Defines the frequency band channel range.
 *
 */
enum
{
    OT_RADIO_915MHZ_OQPSK_CHANNEL_MIN  = 1,                                           ///< 915 MHz IEEE 802.15.4-2006
    OT_RADIO_915MHZ_OQPSK_CHANNEL_MAX  = 10,                                          ///< 915 MHz IEEE 802.15.4-2006
    OT_RADIO_915MHZ_OQPSK_CHANNEL_MASK = 0x3ff << OT_RADIO_915MHZ_OQPSK_CHANNEL_MIN,  ///< 915 MHz IEEE 802.15.4-2006
    OT_RADIO_2P4GHZ_OQPSK_CHANNEL_MIN  = 11,                                          ///< 2.4 GHz IEEE 802.15.4-2006
    OT_RADIO_2P4GHZ_OQPSK_CHANNEL_MAX  = 26,                                          ///< 2.4 GHz IEEE 802.15.4-2006
    OT_RADIO_2P4GHZ_OQPSK_CHANNEL_MASK = 0xffff << OT_RADIO_2P4GHZ_OQPSK_CHANNEL_MIN, ///< 2.4 GHz IEEE 802.15.4-2006
};

/**
 * Represents radio capabilities.
 *
 * The value is a bit-field indicating the capabilities supported by the radio. See `OT_RADIO_CAPS_*` definitions.
 *
 */
typedef uint16_t otRadioCaps;

/**
 * Defines constants that are used to indicate different radio capabilities. See `otRadioCaps`.
 *
 */
enum
{
    OT_RADIO_CAPS_NONE             = 0,      ///< Radio supports no capability.
    OT_RADIO_CAPS_ACK_TIMEOUT      = 1 << 0, ///< Radio supports AckTime event.
    OT_RADIO_CAPS_ENERGY_SCAN      = 1 << 1, ///< Radio supports Energy Scans.
    OT_RADIO_CAPS_TRANSMIT_RETRIES = 1 << 2, ///< Radio supports tx retry logic with collision avoidance (CSMA).
    OT_RADIO_CAPS_CSMA_BACKOFF     = 1 << 3, ///< Radio supports CSMA backoff for frame transmission (but no retry).
    OT_RADIO_CAPS_SLEEP_TO_TX      = 1 << 4, ///< Radio supports direct transition from sleep to TX with CSMA.
    OT_RADIO_CAPS_TRANSMIT_SEC     = 1 << 5, ///< Radio supports tx security.
    OT_RADIO_CAPS_TRANSMIT_TIMING  = 1 << 6, ///< Radio supports tx at specific time.
    OT_RADIO_CAPS_RECEIVE_TIMING   = 1 << 7, ///< Radio supports rx at specific time.
    OT_RADIO_CAPS_RX_ON_WHEN_IDLE  = 1 << 8, ///< Radio supports RxOnWhenIdle handling.
};

#define OT_PANID_BROADCAST 0xffff ///< IEEE 802.15.4 Broadcast PAN ID

/**
 * Represents the IEEE 802.15.4 PAN ID.
 *
 */
typedef uint16_t otPanId;

/**
 * Represents the IEEE 802.15.4 Short Address.
 *
 */
typedef uint16_t otShortAddress;

#define OT_EXT_ADDRESS_SIZE 8 ///< Size of an IEEE 802.15.4 Extended Address (bytes)

/**
 * Defines constants about size of header IE in ACK.
 *
 */
enum
{
    OT_IE_HEADER_SIZE               = 2,  ///< Size of IE header in bytes.
    OT_CSL_IE_SIZE                  = 4,  ///< Size of CSL IE content in bytes.
    OT_ACK_IE_MAX_SIZE              = 16, ///< Max length for header IE in ACK.
    OT_ENH_PROBING_IE_DATA_MAX_SIZE = 2,  ///< Max length of Link Metrics data in Vendor-Specific IE.
};

#define CSL_IE_HEADER_BYTES_LO 0x04 ///< Fixed CSL IE header first byte
#define CSL_IE_HEADER_BYTES_HI 0x0d ///< Fixed CSL IE header second byte

/**
 * @struct otExtAddress
 *
 * Represents the IEEE 802.15.4 Extended Address.
 *
 */
OT_TOOL_PACKED_BEGIN
struct otExtAddress
{
    uint8_t m8[OT_EXT_ADDRESS_SIZE]; ///< IEEE 802.15.4 Extended Address bytes
} OT_TOOL_PACKED_END;

/**
 * Represents the IEEE 802.15.4 Extended Address.
 *
 */
typedef struct otExtAddress otExtAddress;

#define OT_MAC_KEY_SIZE 16 ///< Size of the MAC Key in bytes.

/**
 * @struct otMacKey
 *
 * Represents a MAC Key.
 *
 */
OT_TOOL_PACKED_BEGIN
struct otMacKey
{
    uint8_t m8[OT_MAC_KEY_SIZE]; ///< MAC Key bytes.
} OT_TOOL_PACKED_END;

/**
 * Represents a MAC Key.
 *
 */
typedef struct otMacKey otMacKey;

/**
 * Represents a MAC Key Ref used by PSA.
 *
 */
typedef otCryptoKeyRef otMacKeyRef;

/**
 * @struct otMacKeyMaterial
 *
 * Represents a MAC Key.
 *
 */
typedef struct otMacKeyMaterial
{
    union
    {
        otMacKeyRef mKeyRef; ///< Reference to the key stored.
        otMacKey    mKey;    ///< Key stored as literal.
    } mKeyMaterial;
} otMacKeyMaterial;

/**
 * Defines constants about key types.
 *
 */
typedef enum
{
    OT_KEY_TYPE_LITERAL_KEY = 0, ///< Use Literal Keys.
    OT_KEY_TYPE_KEY_REF     = 1, ///< Use Reference to Key.
} otRadioKeyType;

/**
 * Represents the IEEE 802.15.4 Header IE (Information Element) related information of a radio frame.
 */
typedef struct otRadioIeInfo
{
    int64_t mNetworkTimeOffset; ///< The time offset to the Thread network time.
    uint8_t mTimeIeOffset;      ///< The Time IE offset from the start of PSDU.
    uint8_t mTimeSyncSeq;       ///< The Time sync sequence.
} otRadioIeInfo;

/**
 * Represents an IEEE 802.15.4 radio frame.
 */
typedef struct otRadioFrame
{
    uint8_t *mPsdu; ///< The PSDU.

    uint16_t mLength;  ///< Length of the PSDU.
    uint8_t  mChannel; ///< Channel used to transmit/receive the frame.

    uint8_t mRadioType; ///< Radio link type - should be ignored by radio driver.

    /**
     * The union of transmit and receive information for a radio frame.
     */
    union
    {
        /**
         * Structure representing radio frame transmit information.
         */
        struct
        {
            const otMacKeyMaterial *mAesKey; ///< The key material used for AES-CCM frame security.
            otRadioIeInfo          *mIeInfo; ///< The pointer to the Header IE(s) related information.

            /**
             * The base time in microseconds for scheduled transmissions
             * relative to the local radio clock, see `otPlatRadioGetNow` and
             * `mTxDelay`.
             */
            uint32_t mTxDelayBaseTime;

            /**
             * The delay time in microseconds for this transmission referenced
             * to `mTxDelayBaseTime`.
             *
             * Note: `mTxDelayBaseTime` + `mTxDelay` SHALL point to the point in
             * time when the end of the SFD will be present at the local
             * antenna, relative to the local radio clock.
             */
            uint32_t mTxDelay;

            uint8_t mMaxCsmaBackoffs; ///< Maximum number of backoffs attempts before declaring CCA failure.
            uint8_t mMaxFrameRetries; ///< Maximum number of retries allowed after a transmission failure.

            /**
             * The RX channel after frame TX is done (after all frame retries - ack received, or timeout, or abort).
             *
             * Radio platforms can choose to fully ignore this. OT stack will make sure to call `otPlatRadioReceive()`
             * with the desired RX channel after a frame TX is done and signaled in `otPlatRadioTxDone()` callback.
             * Radio platforms that don't provide `OT_RADIO_CAPS_TRANSMIT_RETRIES` must always ignore this.
             *
             * This is intended for situations where there may be delay in interactions between OT stack and radio, as
             * an example this is used in RCP/host architecture to make sure RCP switches to PAN channel more quickly.
             * In particular, this can help with CSL tx to a sleepy child, where the child may use a different channel
             * for CSL than the PAN channel. After frame tx, we want the radio/RCP to go back to the PAN channel
             * quickly to ensure that parent does not miss tx from child afterwards, e.g., child responding to the
             * earlier CSL transmitted frame from parent using PAN channel while radio still staying on CSL channel.
             *
             * The switch to the RX channel MUST happen after the frame TX is fully done, i.e., after all retries and
             * when ack is received (when "Ack Request" flag is set on the TX frame) or ack timeout. Note that ack is
             * expected on the same channel that frame is sent on.
             *
             */
            uint8_t mRxChannelAfterTxDone;

            /**
             * Indicates whether frame counter and CSL IEs are properly updated in the header.
             *
             * If the platform layer does not provide `OT_RADIO_CAPS_TRANSMIT_SEC` capability, it can ignore this flag.
             *
             * If the platform provides `OT_RADIO_CAPS_TRANSMIT_SEC` capability, then platform is expected to handle tx
             * security processing and assignment of frame counter. In this case the following behavior is expected:
             *
             * When `mIsHeaderUpdated` is set, it indicates that OpenThread core has already set the frame counter and
             * CSL IEs (if security is enabled) in the prepared frame. The counter is ensured to match the counter value
             * from the previous attempts of the same frame. The platform should not assign or change the frame counter
             * (but may still need to perform security processing depending on `mIsSecurityProcessed` flag).
             *
             * If `mIsHeaderUpdated` is not set, then the frame counter and key CSL IE not set in the frame by
             * OpenThread core and it is the responsibility of the radio platform to assign them. The platform
             * must update the frame header (assign counter and CSL IE values) before sending the frame over the air,
             * however if the the transmission gets aborted and the frame is never sent over the air (e.g., channel
             * access error) the platform may choose to not update the header. If the platform updates the header,
             * it must also set this flag before passing the frame back from the `otPlatRadioTxDone()` callback.
             *
             */
            bool mIsHeaderUpdated : 1;
            bool mIsARetx : 1;             ///< Indicates whether the frame is a retransmission or not.
            bool mCsmaCaEnabled : 1;       ///< Set to true to enable CSMA-CA for this packet, false otherwise.
            bool mCslPresent : 1;          ///< Set to true if CSL header IE is present.
            bool mIsSecurityProcessed : 1; ///< True if SubMac should skip the AES processing of this frame.
        } mTxInfo;

        /**
         * Structure representing radio frame receive information.
         */
        struct
        {
            /**
             * The time of the local radio clock in microseconds when the end of
             * the SFD was present at the local antenna.
             */
            uint64_t mTimestamp;

            uint32_t mAckFrameCounter; ///< ACK security frame counter (applicable when `mAckedWithSecEnhAck` is set).
            uint8_t  mAckKeyId;        ///< ACK security key index (applicable when `mAckedWithSecEnhAck` is set).
            int8_t   mRssi;            ///< Received signal strength indicator in dBm for received frames.
            uint8_t  mLqi;             ///< Link Quality Indicator for received frames.

            // Flags
            bool mAckedWithFramePending : 1; ///< This indicates if this frame was acknowledged with frame pending set.
            bool mAckedWithSecEnhAck : 1; ///< This indicates if this frame was acknowledged with secured enhance ACK.
        } mRxInfo;
    } mInfo;
} otRadioFrame;

/**
 * Represents the state of a radio.
 * Initially, a radio is in the Disabled state.
 */
typedef enum otRadioState
{
    OT_RADIO_STATE_DISABLED = 0,
    OT_RADIO_STATE_SLEEP    = 1,
    OT_RADIO_STATE_RECEIVE  = 2,
    OT_RADIO_STATE_TRANSMIT = 3,
    OT_RADIO_STATE_INVALID  = 255,
} otRadioState;

/**
 * The following are valid radio state transitions:
 *
 *                                    (Radio ON)
 *  +----------+  Enable()  +-------+  Receive() +---------+   Transmit()  +----------+
 *  |          |----------->|       |----------->|         |-------------->|          |
 *  | Disabled |            | Sleep |            | Receive |               | Transmit |
 *  |          |<-----------|       |<-----------|         |<--------------|          |
 *  +----------+  Disable() +-------+   Sleep()  +---------+   Receive()   +----------+
 *                                    (Radio OFF)                 or
 *                                                        signal TransmitDone
 *
 * During the IEEE 802.15.4 data request command the transition Sleep->Receive->Transmit
 * can be shortened to direct transition from Sleep to Transmit if the platform supports
 * the OT_RADIO_CAPS_SLEEP_TO_TX capability.
 */

/**
 * Represents radio coexistence metrics.
 */
typedef struct otRadioCoexMetrics
{
    uint32_t mNumGrantGlitch;          ///< Number of grant glitches.
    uint32_t mNumTxRequest;            ///< Number of tx requests.
    uint32_t mNumTxGrantImmediate;     ///< Number of tx requests while grant was active.
    uint32_t mNumTxGrantWait;          ///< Number of tx requests while grant was inactive.
    uint32_t mNumTxGrantWaitActivated; ///< Number of tx requests while grant was inactive that were ultimately granted.
    uint32_t mNumTxGrantWaitTimeout;   ///< Number of tx requests while grant was inactive that timed out.
    uint32_t mNumTxGrantDeactivatedDuringRequest; ///< Number of tx that were in progress when grant was deactivated.
    uint32_t mNumTxDelayedGrant;                  ///< Number of tx requests that were not granted within 50us.
    uint32_t mAvgTxRequestToGrantTime;            ///< Average time in usec from tx request to grant.
    uint32_t mNumRxRequest;                       ///< Number of rx requests.
    uint32_t mNumRxGrantImmediate;                ///< Number of rx requests while grant was active.
    uint32_t mNumRxGrantWait;                     ///< Number of rx requests while grant was inactive.
    uint32_t mNumRxGrantWaitActivated; ///< Number of rx requests while grant was inactive that were ultimately granted.
    uint32_t mNumRxGrantWaitTimeout;   ///< Number of rx requests while grant was inactive that timed out.
    uint32_t mNumRxGrantDeactivatedDuringRequest; ///< Number of rx that were in progress when grant was deactivated.
    uint32_t mNumRxDelayedGrant;                  ///< Number of rx requests that were not granted within 50us.
    uint32_t mAvgRxRequestToGrantTime;            ///< Average time in usec from rx request to grant.
    uint32_t mNumRxGrantNone;                     ///< Number of rx requests that completed without receiving grant.
    bool     mStopped;                            ///< Stats collection stopped due to saturation.
} otRadioCoexMetrics;

/**
 * Represents what metrics are specified to query.
 *
 */
typedef struct otLinkMetrics
{
    bool mPduCount : 1;   ///< Pdu count.
    bool mLqi : 1;        ///< Link Quality Indicator.
    bool mLinkMargin : 1; ///< Link Margin.
    bool mRssi : 1;       ///< Received Signal Strength Indicator.
    bool mReserved : 1;   ///< Reserved, this is for reference device.
} otLinkMetrics;

/**
 * @}
 *
 */

/**
 * @defgroup radio-config Radio Configuration
 *
 * @brief
 *   This module includes the platform abstraction for radio configuration.
 *
 * @{
 *
 */

/**
 * Get the radio capabilities.
 *
 * @param[in] aInstance  The OpenThread instance structure.
 *
 * @returns The radio capability bit vector (see `OT_RADIO_CAP_*` definitions).
 *
 */
otRadioCaps otPlatRadioGetCaps(otInstance *aInstance);

/**
 * Get the radio version string.
 *
 * This is an optional radio driver platform function. If not provided by platform radio driver, OpenThread uses
 * the OpenThread version instead (@sa otGetVersionString()).
 *
 * @param[in]  aInstance   The OpenThread instance structure.
 *
 * @returns A pointer to the OpenThread radio version.
 *
 */
const char *otPlatRadioGetVersionString(otInstance *aInstance);

/**
 * Get the radio receive sensitivity value.
 *
 * @param[in] aInstance  The OpenThread instance structure.
 *
 * @returns The radio receive sensitivity value in dBm.
 *
 */
int8_t otPlatRadioGetReceiveSensitivity(otInstance *aInstance);

/**
 * Gets the factory-assigned IEEE EUI-64 for this interface.
 *
 * @param[in]  aInstance   The OpenThread instance structure.
 * @param[out] aIeeeEui64  A pointer to the factory-assigned IEEE EUI-64.
 *
 */
void otPlatRadioGetIeeeEui64(otInstance *aInstance, uint8_t *aIeeeEui64);

/**
 * Set the PAN ID for address filtering.
 *
 * @param[in] aInstance  The OpenThread instance structure.
 * @param[in] aPanId     The IEEE 802.15.4 PAN ID.
 *
 */
void otPlatRadioSetPanId(otInstance *aInstance, otPanId aPanId);

/**
 * Set the Extended Address for address filtering.
 *
 * @param[in] aInstance    The OpenThread instance structure.
 * @param[in] aExtAddress  A pointer to the IEEE 802.15.4 Extended Address stored in little-endian byte order.
 *
 *
 */
void otPlatRadioSetExtendedAddress(otInstance *aInstance, const otExtAddress *aExtAddress);

/**
 * Set the Short Address for address filtering.
 *
 * @param[in] aInstance      The OpenThread instance structure.
 * @param[in] aShortAddress  The IEEE 802.15.4 Short Address.
 *
 */
void otPlatRadioSetShortAddress(otInstance *aInstance, otShortAddress aShortAddress);

/**
 * Get the radio's transmit power in dBm.
 *
 * @note The transmit power returned will be no larger than the power specified in the max power table for
 * the current channel.
 *
 * @param[in] aInstance  The OpenThread instance structure.
 * @param[out] aPower    The transmit power in dBm.
 *
 * @retval OT_ERROR_NONE             Successfully retrieved the transmit power.
 * @retval OT_ERROR_INVALID_ARGS     @p aPower was NULL.
 * @retval OT_ERROR_NOT_IMPLEMENTED  Transmit power configuration via dBm is not implemented.
 *
 */
otError otPlatRadioGetTransmitPower(otInstance *aInstance, int8_t *aPower);

/**
 * Set the radio's transmit power in dBm.
 *
 * @note The real transmit power will be no larger than the power specified in the max power table for
 * the current channel.
 *
 * @param[in] aInstance  The OpenThread instance structure.
 * @param[in] aPower     The transmit power in dBm.
 *
 * @retval OT_ERROR_NONE             Successfully set the transmit power.
 * @retval OT_ERROR_NOT_IMPLEMENTED  Transmit power configuration via dBm is not implemented.
 *
 */
otError otPlatRadioSetTransmitPower(otInstance *aInstance, int8_t aPower);

/**
 * Get the radio's CCA ED threshold in dBm measured at antenna connector per IEEE 802.15.4 - 2015 section 10.1.4.
 *
 * @param[in] aInstance    The OpenThread instance structure.
 * @param[out] aThreshold  The CCA ED threshold in dBm.
 *
 * @retval OT_ERROR_NONE             Successfully retrieved the CCA ED threshold.
 * @retval OT_ERROR_INVALID_ARGS     @p aThreshold was NULL.
 * @retval OT_ERROR_NOT_IMPLEMENTED  CCA ED threshold configuration via dBm is not implemented.
 *
 */
otError otPlatRadioGetCcaEnergyDetectThreshold(otInstance *aInstance, int8_t *aThreshold);

/**
 * Set the radio's CCA ED threshold in dBm measured at antenna connector per IEEE 802.15.4 - 2015 section 10.1.4.
 *
 * @param[in] aInstance   The OpenThread instance structure.
 * @param[in] aThreshold  The CCA ED threshold in dBm.
 *
 * @retval OT_ERROR_NONE             Successfully set the transmit power.
 * @retval OT_ERROR_INVALID_ARGS     Given threshold is out of range.
 * @retval OT_ERROR_NOT_IMPLEMENTED  CCA ED threshold configuration via dBm is not implemented.
 *
 */
otError otPlatRadioSetCcaEnergyDetectThreshold(otInstance *aInstance, int8_t aThreshold);

/**
 * Gets the external FEM's Rx LNA gain in dBm.
 *
 * @param[in]  aInstance  The OpenThread instance structure.
 * @param[out] aGain     The external FEM's Rx LNA gain in dBm.
 *
 * @retval OT_ERROR_NONE             Successfully retrieved the external FEM's LNA gain.
 * @retval OT_ERROR_INVALID_ARGS     @p aGain was NULL.
 * @retval OT_ERROR_NOT_IMPLEMENTED  External FEM's LNA setting is not implemented.
 *
 */
otError otPlatRadioGetFemLnaGain(otInstance *aInstance, int8_t *aGain);

/**
 * Sets the external FEM's Rx LNA gain in dBm.
 *
 * @param[in] aInstance  The OpenThread instance structure.
 * @param[in] aGain      The external FEM's Rx LNA gain in dBm.
 *
 * @retval OT_ERROR_NONE             Successfully set the external FEM's LNA gain.
 * @retval OT_ERROR_NOT_IMPLEMENTED  External FEM's LNA gain setting is not implemented.
 *
 */
otError otPlatRadioSetFemLnaGain(otInstance *aInstance, int8_t aGain);

/**
 * Get the status of promiscuous mode.
 *
 * @param[in] aInstance  The OpenThread instance structure.
 *
 * @retval TRUE   Promiscuous mode is enabled.
 * @retval FALSE  Promiscuous mode is disabled.
 *
 */
bool otPlatRadioGetPromiscuous(otInstance *aInstance);

/**
 * Enable or disable promiscuous mode.
 *
 * @param[in]  aInstance The OpenThread instance structure.
 * @param[in]  aEnable   TRUE to enable or FALSE to disable promiscuous mode.
 *
 */
void otPlatRadioSetPromiscuous(otInstance *aInstance, bool aEnable);

/**
 * Sets the rx-on-when-idle state to the radio platform.
 *
 * There are a few situations that the radio can enter sleep state if the device is in rx-off-when-idle state but
 * it's hard and costly for the SubMac to identify these situations and instruct the radio to enter sleep:
 *
 * - Finalization of a regular frame reception task, provided that:
 *   - The frame is received without errors and passes the filtering and it's not an spurious ACK.
 *   - ACK is not requested or transmission of ACK is not possible due to internal conditions.
 * - Finalization of a frame transmission or transmission of an ACK frame, when ACK is not requested in the transmitted
 *   frame.
 * - Finalization of the reception operation of a requested ACK due to:
 *   - ACK timeout expiration.
 *   - Reception of an invalid ACK or not an ACK frame.
 *   - Reception of the proper ACK, unless the transmitted frame was a Data Request Command and the frame pending bit
 *     on the received ACK is set to true. In this case the radio platform implementation SHOULD keep the receiver on
 *     until a determined timeout which triggers an idle period start.`OPENTHREAD_CONFIG_MAC_DATA_POLL_TIMEOUT` can be
 *     taken as a reference for this.
 * - Finalization of a stand alone CCA task.
 * - Finalization of a CCA operation with busy result during CSMA/CA procedure.
 * - Finalization of an Energy Detection task.
 * - Finalization of a radio reception window scheduled with `otPlatRadioReceiveAt`.
 *
 * If a platform supports `OT_RADIO_CAPS_RX_ON_WHEN_IDLE` it must also support `OT_RADIO_CAPS_CSMA_BACKOFF` and handle
 * idle periods after CCA as described above.
 *
 * Upon the transition of the "RxOnWhenIdle" flag from TRUE to FALSE, the radio platform should enter sleep mode.
 * If the radio is currently in receive mode, it should enter sleep mode immediately. Otherwise, it should enter sleep
 * mode after the current operation is completed.
 *
 * @param[in]  aInstance    The OpenThread instance structure.
 * @param[in]  aEnable      TRUE to keep radio in Receive state, FALSE to put to Sleep state during idle periods.
 *
 */
void otPlatRadioSetRxOnWhenIdle(otInstance *aInstance, bool aEnable);

/**
 * Update MAC keys and key index
 *
 * Is used when radio provides OT_RADIO_CAPS_TRANSMIT_SEC capability.
 *
 * @param[in]   aInstance    A pointer to an OpenThread instance.
 * @param[in]   aKeyIdMode   The key ID mode.
 * @param[in]   aKeyId       Current MAC key index.
 * @param[in]   aPrevKey     A pointer to the previous MAC key.
 * @param[in]   aCurrKey     A pointer to the current MAC key.
 * @param[in]   aNextKey     A pointer to the next MAC key.
 * @param[in]   aKeyType     Key Type used.
 *
 */
void otPlatRadioSetMacKey(otInstance             *aInstance,
                          uint8_t                 aKeyIdMode,
                          uint8_t                 aKeyId,
                          const otMacKeyMaterial *aPrevKey,
                          const otMacKeyMaterial *aCurrKey,
                          const otMacKeyMaterial *aNextKey,
                          otRadioKeyType          aKeyType);

/**
 * Sets the current MAC frame counter value.
 *
 * Is used when radio provides `OT_RADIO_CAPS_TRANSMIT_SEC` capability.
 *
 * @param[in]   aInstance         A pointer to an OpenThread instance.
 * @param[in]   aMacFrameCounter  The MAC frame counter value.
 *
 */
void otPlatRadioSetMacFrameCounter(otInstance *aInstance, uint32_t aMacFrameCounter);

/**
 * Sets the current MAC frame counter value only if the new given value is larger than the current value.
 *
 * Is used when radio provides `OT_RADIO_CAPS_TRANSMIT_SEC` capability.
 *
 * @param[in]   aInstance         A pointer to an OpenThread instance.
 * @param[in]   aMacFrameCounter  The MAC frame counter value.
 *
 */
void otPlatRadioSetMacFrameCounterIfLarger(otInstance *aInstance, uint32_t aMacFrameCounter);

/**
 * Get the current time in microseconds referenced to a continuous monotonic
 * local radio clock (64 bits width).
 *
 * The radio clock SHALL NOT wrap during the device's uptime. Implementations
 * SHALL therefore identify and compensate for internal counter overflows. The
 * clock does not have a defined epoch and it SHALL NOT introduce any continuous
 * or discontinuous adjustments (e.g. leap seconds). Implementations SHALL
 * compensate for any sleep times of the device.
 *
 * Implementations MAY choose to discipline the radio clock and compensate for
 * sleep times by any means (e.g. by combining a high precision/low power RTC
 * with a high resolution counter) as long as the exposed combined clock
 * provides continuous monotonic microsecond resolution ticks within the
 * accuracy limits announced by @ref otPlatRadioGetCslAccuracy.
 *
 * @param[in]   aInstance    A pointer to an OpenThread instance.
 *
 * @returns The current time in microseconds. UINT64_MAX when platform does not
 * support or radio time is not ready.
 *
 */
uint64_t otPlatRadioGetNow(otInstance *aInstance);

/**
 * Get the bus speed in bits/second between the host and the radio chip.
 *
 * @param[in]   aInstance    A pointer to an OpenThread instance.
 *
 * @returns The bus speed in bits/second between the host and the radio chip.
 *          Return 0 when the MAC and above layer and Radio layer resides on the same chip.
 *
 */
uint32_t otPlatRadioGetBusSpeed(otInstance *aInstance);

/**
 * @}
 *
 */

/**
 * @defgroup radio-operation Radio Operation
 *
 * @brief
 *   This module includes the platform abstraction for radio operations.
 *
 * @{
 *
 */

/**
 * Get current state of the radio.
 *
 * Is not required by OpenThread. It may be used for debugging and/or application-specific purposes.
 *
 * @note This function may be not implemented. It does not affect OpenThread.
 *
 * @param[in] aInstance  The OpenThread instance structure.
 *
 * @return  Current state of the radio.
 *
 */
otRadioState otPlatRadioGetState(otInstance *aInstance);

/**
 * Enable the radio.
 *
 * @param[in] aInstance  The OpenThread instance structure.
 *
 * @retval OT_ERROR_NONE     Successfully enabled.
 * @retval OT_ERROR_FAILED   The radio could not be enabled.
 *
 */
otError otPlatRadioEnable(otInstance *aInstance);

/**
 * Disable the radio.
 *
 * @param[in] aInstance  The OpenThread instance structure.
 *
 * @retval OT_ERROR_NONE            Successfully transitioned to Disabled.
 * @retval OT_ERROR_INVALID_STATE   The radio was not in sleep state.
 *
 */
otError otPlatRadioDisable(otInstance *aInstance);

/**
 * Check whether radio is enabled or not.
 *
 * @param[in] aInstance  The OpenThread instance structure.
 *
 * @returns TRUE if the radio is enabled, FALSE otherwise.
 *
 */
bool otPlatRadioIsEnabled(otInstance *aInstance);

/**
 * Transition the radio from Receive to Sleep (turn off the radio).
 *
 * @param[in] aInstance  The OpenThread instance structure.
 *
 * @retval OT_ERROR_NONE          Successfully transitioned to Sleep.
 * @retval OT_ERROR_BUSY          The radio was transmitting.
 * @retval OT_ERROR_INVALID_STATE The radio was disabled.
 *
 */
otError otPlatRadioSleep(otInstance *aInstance);

/**
 * Transition the radio from Sleep to Receive (turn on the radio).
 *
 * @param[in]  aInstance  The OpenThread instance structure.
 * @param[in]  aChannel   The channel to use for receiving.
 *
 * @retval OT_ERROR_NONE          Successfully transitioned to Receive.
 * @retval OT_ERROR_INVALID_STATE The radio was disabled or transmitting.
 *
 */
otError otPlatRadioReceive(otInstance *aInstance, uint8_t aChannel);

/**
 * Schedule a radio reception window at a specific time and duration.
 *
 * @param[in]  aChannel   The radio channel on which to receive.
 * @param[in]  aStart     The receive window start time relative to the local
 *                        radio clock, see `otPlatRadioGetNow`. The radio
 *                        receiver SHALL be on and ready to receive the first
 *                        symbol of a frame's SHR at the window start time.
 * @param[in]  aDuration  The receive window duration, in microseconds, as
 *                        measured by the local radio clock. The radio SHOULD be
 *                        turned off (or switched to TX mode if an ACK frame
 *                        needs to be sent) after that duration unless it is
 *                        still actively receiving a frame. In the latter case
 *                        the radio SHALL be kept in reception mode until frame
 *                        reception has either succeeded or failed.
 *
 * @retval OT_ERROR_NONE    Successfully scheduled receive window.
 * @retval OT_ERROR_FAILED  The receive window could not be scheduled.
 */
otError otPlatRadioReceiveAt(otInstance *aInstance, uint8_t aChannel, uint32_t aStart, uint32_t aDuration);

/**
 * The radio driver calls this method to notify OpenThread of a received frame.
 *
 * @param[in]  aInstance The OpenThread instance structure.
 * @param[in]  aFrame    A pointer to the received frame or NULL if the receive operation failed.
 * @param[in]  aError    OT_ERROR_NONE when successfully received a frame,
 *                       OT_ERROR_ABORT when reception was aborted and a frame was not received,
 *                       OT_ERROR_NO_BUFS when a frame could not be received due to lack of rx buffer space.
 *
 */
extern void otPlatRadioReceiveDone(otInstance *aInstance, otRadioFrame *aFrame, otError aError);

/**
 * The radio driver calls this method to notify OpenThread diagnostics module of a received frame.
 *
 * Is used when diagnostics is enabled.
 *
 * @param[in]  aInstance The OpenThread instance structure.
 * @param[in]  aFrame    A pointer to the received frame or NULL if the receive operation failed.
 * @param[in]  aError    OT_ERROR_NONE when successfully received a frame,
 *                       OT_ERROR_ABORT when reception was aborted and a frame was not received,
 *                       OT_ERROR_NO_BUFS when a frame could not be received due to lack of rx buffer space.
 *
 */
extern void otPlatDiagRadioReceiveDone(otInstance *aInstance, otRadioFrame *aFrame, otError aError);

/**
 * Get the radio transmit frame buffer.
 *
 * OpenThread forms the IEEE 802.15.4 frame in this buffer then calls `otPlatRadioTransmit()` to request transmission.
 *
 * @param[in] aInstance  The OpenThread instance structure.
 *
 * @returns A pointer to the transmit frame buffer.
 *
 */
otRadioFrame *otPlatRadioGetTransmitBuffer(otInstance *aInstance);

/**
 * Begin the transmit sequence on the radio.
 *
 * The caller must form the IEEE 802.15.4 frame in the buffer provided by `otPlatRadioGetTransmitBuffer()` before
 * requesting transmission.  The channel and transmit power are also included in the otRadioFrame structure.
 *
 * The transmit sequence consists of:
 * 1. Transitioning the radio to Transmit from one of the following states:
 *    - Receive if RX is on when the device is idle or OT_RADIO_CAPS_SLEEP_TO_TX is not supported
 *    - Sleep if RX is off when the device is idle and OT_RADIO_CAPS_SLEEP_TO_TX is supported.
 * 2. Transmits the psdu on the given channel and at the given transmit power.
 *
 * @param[in] aInstance  The OpenThread instance structure.
 * @param[in] aFrame     A pointer to the frame to be transmitted.
 *
 * @retval OT_ERROR_NONE          Successfully transitioned to Transmit.
 * @retval OT_ERROR_INVALID_STATE The radio was not in the Receive state.
 *
 */
otError otPlatRadioTransmit(otInstance *aInstance, otRadioFrame *aFrame);

/**
 * The radio driver calls this method to notify OpenThread that the transmission has started.
 *
 * @note  This function should be called by the same thread that executes all of the other OpenThread code. It should
 *        not be called by ISR or any other task.
 *
 * @param[in]  aInstance  A pointer to the OpenThread instance structure.
 * @param[in]  aFrame     A pointer to the frame that is being transmitted.
 *
 */
extern void otPlatRadioTxStarted(otInstance *aInstance, otRadioFrame *aFrame);

/**
 * The radio driver calls this function to notify OpenThread that the transmit operation has completed,
 * providing both the transmitted frame and, if applicable, the received ack frame.
 *
 * When radio provides `OT_RADIO_CAPS_TRANSMIT_SEC` capability, radio platform layer updates @p aFrame
 * with the security frame counter and key index values maintained by the radio.
 *
 * @param[in]  aInstance  The OpenThread instance structure.
 * @param[in]  aFrame     A pointer to the frame that was transmitted.
 * @param[in]  aAckFrame  A pointer to the ACK frame, NULL if no ACK was received.
 * @param[in]  aError     OT_ERROR_NONE when the frame was transmitted,
 *                        OT_ERROR_NO_ACK when the frame was transmitted but no ACK was received,
 *                        OT_ERROR_CHANNEL_ACCESS_FAILURE tx could not take place due to activity on the channel,
 *                        OT_ERROR_ABORT when transmission was aborted for other reasons.
 *
 */
extern void otPlatRadioTxDone(otInstance *aInstance, otRadioFrame *aFrame, otRadioFrame *aAckFrame, otError aError);

/**
 * The radio driver calls this method to notify OpenThread diagnostics module that the transmission has completed.
 *
 * Is used when diagnostics is enabled.
 *
 * @param[in]  aInstance      The OpenThread instance structure.
 * @param[in]  aFrame         A pointer to the frame that was transmitted.
 * @param[in]  aError         OT_ERROR_NONE when the frame was transmitted,
 *                            OT_ERROR_CHANNEL_ACCESS_FAILURE tx could not take place due to activity on the channel,
 *                            OT_ERROR_ABORT when transmission was aborted for other reasons.
 *
 */
extern void otPlatDiagRadioTransmitDone(otInstance *aInstance, otRadioFrame *aFrame, otError aError);

/**
 * Get the most recent RSSI measurement.
 *
 * @param[in] aInstance  The OpenThread instance structure.
 *
 * @returns The RSSI in dBm when it is valid.  127 when RSSI is invalid.
 *
 */
int8_t otPlatRadioGetRssi(otInstance *aInstance);

/**
 * Begin the energy scan sequence on the radio.
 *
 * Is used when radio provides OT_RADIO_CAPS_ENERGY_SCAN capability.
 *
 * @param[in] aInstance      The OpenThread instance structure.
 * @param[in] aScanChannel   The channel to perform the energy scan on.
 * @param[in] aScanDuration  The duration, in milliseconds, for the channel to be scanned.
 *
 * @retval OT_ERROR_NONE             Successfully started scanning the channel.
 * @retval OT_ERROR_BUSY             The radio is performing energy scanning.
 * @retval OT_ERROR_NOT_IMPLEMENTED  The radio doesn't support energy scanning.
 *
 */
otError otPlatRadioEnergyScan(otInstance *aInstance, uint8_t aScanChannel, uint16_t aScanDuration);

/**
 * The radio driver calls this method to notify OpenThread that the energy scan is complete.
 *
 * Is used when radio provides OT_RADIO_CAPS_ENERGY_SCAN capability.
 *
 * @param[in]  aInstance           The OpenThread instance structure.
 * @param[in]  aEnergyScanMaxRssi  The maximum RSSI encountered on the scanned channel.
 *
 */
extern void otPlatRadioEnergyScanDone(otInstance *aInstance, int8_t aEnergyScanMaxRssi);

/**
 * Enable/Disable source address match feature.
 *
 * The source address match feature controls how the radio layer decides the "frame pending" bit for acks sent in
 * response to data request commands from children.
 *
 * If disabled, the radio layer must set the "frame pending" on all acks to data request commands.
 *
 * If enabled, the radio layer uses the source address match table to determine whether to set or clear the "frame
 * pending" bit in an ack to a data request command.
 *
 * The source address match table provides the list of children for which there is a pending frame. Either a short
 * address or an extended/long address can be added to the source address match table.
 *
 * @param[in]  aInstance   The OpenThread instance structure.
 * @param[in]  aEnable     Enable/disable source address match feature.
 *
 */
void otPlatRadioEnableSrcMatch(otInstance *aInstance, bool aEnable);

/**
 * Add a short address to the source address match table.
 *
 * @param[in]  aInstance      The OpenThread instance structure.
 * @param[in]  aShortAddress  The short address to be added.
 *
 * @retval OT_ERROR_NONE      Successfully added short address to the source match table.
 * @retval OT_ERROR_NO_BUFS   No available entry in the source match table.
 *
 */
otError otPlatRadioAddSrcMatchShortEntry(otInstance *aInstance, otShortAddress aShortAddress);

/**
 * Add an extended address to the source address match table.
 *
 * @param[in]  aInstance    The OpenThread instance structure.
 * @param[in]  aExtAddress  The extended address to be added stored in little-endian byte order.
 *
 * @retval OT_ERROR_NONE      Successfully added extended address to the source match table.
 * @retval OT_ERROR_NO_BUFS   No available entry in the source match table.
 *
 */
otError otPlatRadioAddSrcMatchExtEntry(otInstance *aInstance, const otExtAddress *aExtAddress);

/**
 * Remove a short address from the source address match table.
 *
 * @param[in]  aInstance      The OpenThread instance structure.
 * @param[in]  aShortAddress  The short address to be removed.
 *
 * @retval OT_ERROR_NONE        Successfully removed short address from the source match table.
 * @retval OT_ERROR_NO_ADDRESS  The short address is not in source address match table.
 *
 */
otError otPlatRadioClearSrcMatchShortEntry(otInstance *aInstance, otShortAddress aShortAddress);

/**
 * Remove an extended address from the source address match table.
 *
 * @param[in]  aInstance    The OpenThread instance structure.
 * @param[in]  aExtAddress  The extended address to be removed stored in little-endian byte order.
 *
 * @retval OT_ERROR_NONE        Successfully removed the extended address from the source match table.
 * @retval OT_ERROR_NO_ADDRESS  The extended address is not in source address match table.
 *
 */
otError otPlatRadioClearSrcMatchExtEntry(otInstance *aInstance, const otExtAddress *aExtAddress);

/**
 * Clear all short addresses from the source address match table.
 *
 * @param[in]  aInstance   The OpenThread instance structure.
 *
 */
void otPlatRadioClearSrcMatchShortEntries(otInstance *aInstance);

/**
 * Clear all the extended/long addresses from source address match table.
 *
 * @param[in]  aInstance   The OpenThread instance structure.
 *
 */
void otPlatRadioClearSrcMatchExtEntries(otInstance *aInstance);

/**
 * Get the radio supported channel mask that the device is allowed to be on.
 *
 * @param[in]  aInstance   The OpenThread instance structure.
 *
 * @returns The radio supported channel mask.
 *
 */
uint32_t otPlatRadioGetSupportedChannelMask(otInstance *aInstance);

/**
 * Gets the radio preferred channel mask that the device prefers to form on.
 *
 * @param[in]  aInstance   The OpenThread instance structure.
 *
 * @returns The radio preferred channel mask.
 *
 */
uint32_t otPlatRadioGetPreferredChannelMask(otInstance *aInstance);

/**
 * Enable the radio coex.
 *
 * Is used when feature OPENTHREAD_CONFIG_PLATFORM_RADIO_COEX_ENABLE is enabled.
 *
 * @param[in] aInstance  The OpenThread instance structure.
 * @param[in] aEnabled   TRUE to enable the radio coex, FALSE otherwise.
 *
 * @retval OT_ERROR_NONE     Successfully enabled.
 * @retval OT_ERROR_FAILED   The radio coex could not be enabled.
 *
 */
otError otPlatRadioSetCoexEnabled(otInstance *aInstance, bool aEnabled);

/**
 * Check whether radio coex is enabled or not.
 *
 * Is used when feature OPENTHREAD_CONFIG_PLATFORM_RADIO_COEX_ENABLE is enabled.
 *
 * @param[in] aInstance  The OpenThread instance structure.
 *
 * @returns TRUE if the radio coex is enabled, FALSE otherwise.
 *
 */
bool otPlatRadioIsCoexEnabled(otInstance *aInstance);

/**
 * Get the radio coexistence metrics.
 *
 * Is used when feature OPENTHREAD_CONFIG_PLATFORM_RADIO_COEX_ENABLE is enabled.
 *
 * @param[in]  aInstance     The OpenThread instance structure.
 * @param[out] aCoexMetrics  A pointer to the coexistence metrics structure.
 *
 * @retval OT_ERROR_NONE          Successfully retrieved the coex metrics.
 * @retval OT_ERROR_INVALID_ARGS  @p aCoexMetrics was NULL.
 */
otError otPlatRadioGetCoexMetrics(otInstance *aInstance, otRadioCoexMetrics *aCoexMetrics);

/**
 * Enable or disable CSL receiver.
 *
 * @param[in]  aInstance     The OpenThread instance structure.
 * @param[in]  aCslPeriod    CSL period, 0 for disabling CSL. CSL period is in unit of 10 symbols.
 * @param[in]  aShortAddr    The short source address of CSL receiver's peer.
 * @param[in]  aExtAddr      The extended source address of CSL receiver's peer.
 *
 * @note Platforms should use CSL peer addresses to include CSL IE when generating enhanced acks.
 *
 * @retval  OT_ERROR_NOT_IMPLEMENTED Radio driver doesn't support CSL.
 * @retval  OT_ERROR_FAILED          Other platform specific errors.
 * @retval  OT_ERROR_NONE            Successfully enabled or disabled CSL.
 *
 */
otError otPlatRadioEnableCsl(otInstance         *aInstance,
                             uint32_t            aCslPeriod,
                             otShortAddress      aShortAddr,
                             const otExtAddress *aExtAddr);

/**
 * Reset CSL receiver in the platform.
 *
 * @note Defaults to `otPlatRadioEnableCsl(aInstance,0, Mac::kShortAddrInvalid, nullptr);`
 *
 * @param[in]  aInstance     The OpenThread instance structure.
 *
 * @retval  OT_ERROR_NOT_IMPLEMENTED Radio driver doesn't support CSL.
 * @retval  OT_ERROR_FAILED          Other platform specific errors.
 * @retval  OT_ERROR_NONE            Successfully disabled CSL.
 *
 */
otError otPlatRadioResetCsl(otInstance *aInstance);

/**
 * Update CSL sample time in radio driver.
 *
 * Sample time is stored in radio driver as a copy to calculate phase when
 * sending ACK with CSL IE. The CSL sample (window) of the CSL receiver extends
 * before and after the sample time. The CSL sample time marks a timestamp in
 * the CSL sample window when a frame should be received in "ideal conditions"
 * if there would be no inaccuracy/clock-drift.
 *
 * @param[in]  aInstance         The OpenThread instance structure.
 * @param[in]  aCslSampleTime    The next sample time, in microseconds. It is
 *                               the time when the first symbol of the MHR of
 *                               the frame is expected.
 */
void otPlatRadioUpdateCslSampleTime(otInstance *aInstance, uint32_t aCslSampleTime);

/**
 * Get the current estimated worst case accuracy (maximum ± deviation from the
 * nominal frequency) of the local radio clock in units of PPM. This is the
 * clock used to schedule CSL operations.
 *
 * @note Implementations MAY estimate this value based on current operating
 * conditions (e.g. temperature).
 *
 * In case the implementation does not estimate the current value but returns a
 * fixed value, this value MUST be the worst-case accuracy over all possible
 * foreseen operating conditions (temperature, pressure, etc) of the
 * implementation.
 *
 * @param[in]   aInstance    A pointer to an OpenThread instance.
 *
 * @returns The current CSL rx/tx scheduling drift, in PPM.
 *
 */
uint8_t otPlatRadioGetCslAccuracy(otInstance *aInstance);

/**
 * The fixed uncertainty (i.e. random jitter) of the arrival time of CSL
 * transmissions received by this device in units of 10 microseconds.
 *
 * This designates the worst case constant positive or negative deviation of
 * the actual arrival time of a transmission from the transmission time
 * calculated relative to the local radio clock independent of elapsed time. In
 * addition to uncertainty accumulated over elapsed time, the CSL channel sample
 * ("RX window") must be extended by twice this deviation such that an actual
 * transmission is guaranteed to be detected by the local receiver in the
 * presence of random arrival time jitter.
 *
 * @param[in]   aInstance    A pointer to an OpenThread instance.
 *
 * @returns The CSL Uncertainty in units of 10 us.
 *
 */
uint8_t otPlatRadioGetCslUncertainty(otInstance *aInstance);

/**
 * Set the max transmit power for a specific channel.
 *
 * @param[in]  aInstance    The OpenThread instance structure.
 * @param[in]  aChannel     The radio channel.
 * @param[in]  aMaxPower    The max power in dBm, passing OT_RADIO_RSSI_INVALID will disable this channel.
 *
 * @retval  OT_ERROR_NOT_IMPLEMENTED  The feature is not implemented
 * @retval  OT_ERROR_INVALID_ARGS     The specified channel is not valid.
 * @retval  OT_ERROR_FAILED           Other platform specific errors.
 * @retval  OT_ERROR_NONE             Successfully set max transmit power.
 *
 */
otError otPlatRadioSetChannelMaxTransmitPower(otInstance *aInstance, uint8_t aChannel, int8_t aMaxPower);

/**
 * Set the region code.
 *
 * The radio region format is the 2-bytes ascii representation of the
 * ISO 3166 alpha-2 code.
 *
 * @param[in]  aInstance    The OpenThread instance structure.
 * @param[in]  aRegionCode  The radio region code. The `aRegionCode >> 8` is first ascii char
 *                          and the `aRegionCode & 0xff` is the second ascii char.
 *
 * @retval  OT_ERROR_FAILED           Other platform specific errors.
 * @retval  OT_ERROR_NONE             Successfully set region code.
 * @retval  OT_ERROR_NOT_IMPLEMENTED  The feature is not implemented.
 *
 */
otError otPlatRadioSetRegion(otInstance *aInstance, uint16_t aRegionCode);

/**
 * Get the region code.
 *
 * The radio region format is the 2-bytes ascii representation of the
 * ISO 3166 alpha-2 code.

 * @param[in]  aInstance    The OpenThread instance structure.
 * @param[out] aRegionCode  The radio region.
 *
 * @retval  OT_ERROR_INVALID_ARGS     @p aRegionCode is nullptr.
 * @retval  OT_ERROR_FAILED           Other platform specific errors.
 * @retval  OT_ERROR_NONE             Successfully got region code.
 * @retval  OT_ERROR_NOT_IMPLEMENTED  The feature is not implemented.
 *
 */
otError otPlatRadioGetRegion(otInstance *aInstance, uint16_t *aRegionCode);

/**
 * Enable/disable or update Enhanced-ACK Based Probing in radio for a specific Initiator.
 *
 * After Enhanced-ACK Based Probing is configured by a specific Probing Initiator, the Enhanced-ACK sent to that
 * node should include Vendor-Specific IE containing Link Metrics data. This method informs the radio to start/stop to
 * collect Link Metrics data and include Vendor-Specific IE that containing the data in Enhanced-ACK sent to that
 * Probing Initiator.
 *
 * @param[in]  aInstance     The OpenThread instance structure.
 * @param[in]  aLinkMetrics  This parameter specifies what metrics to query. Per spec 4.11.3.4.4.6, at most 2 metrics
 *                           can be specified. The probing would be disabled if @p `aLinkMetrics` is bitwise 0.
 * @param[in]  aShortAddress The short address of the Probing Initiator.
 * @param[in]  aExtAddress   The extended source address of the Probing Initiator. @p aExtAddr MUST NOT be `NULL`.
 *
 * @retval  OT_ERROR_NONE            Successfully configured the Enhanced-ACK Based Probing.
 * @retval  OT_ERROR_INVALID_ARGS    @p aExtAddress is `NULL`.
 * @retval  OT_ERROR_NOT_FOUND       The Initiator indicated by @p aShortAddress is not found when trying to clear.
 * @retval  OT_ERROR_NO_BUFS         No more Initiator can be supported.
 * @retval  OT_ERROR_NOT_IMPLEMENTED The feature is not implemented.
 *
 */
otError otPlatRadioConfigureEnhAckProbing(otInstance         *aInstance,
                                          otLinkMetrics       aLinkMetrics,
                                          otShortAddress      aShortAddress,
                                          const otExtAddress *aExtAddress);

/**
 * Add a calibrated power of the specified channel to the power calibration table.
 *
 * @note This API is an optional radio platform API. It's up to the platform layer to implement it.
 *
 * The @p aActualPower is the actual measured output power when the parameters of the radio hardware modules
 * are set to the @p aRawPowerSetting.
 *
 * The raw power setting is an opaque byte array. OpenThread doesn't define the format of the raw power setting.
 * Its format is radio hardware related and it should be defined by the developers in the platform radio driver.
 * For example, if the radio hardware contains both the radio chip and the FEM chip, the raw power setting can be
 * a combination of the radio power register and the FEM gain value.
 *
 * @param[in] aInstance               The OpenThread instance structure.
 * @param[in] aChannel                The radio channel.
 * @param[in] aActualPower            The actual power in 0.01dBm.
 * @param[in] aRawPowerSetting        A pointer to the raw power setting byte array.
 * @param[in] aRawPowerSettingLength  The length of the @p aRawPowerSetting.
 *
 * @retval OT_ERROR_NONE             Successfully added the calibrated power to the power calibration table.
 * @retval OT_ERROR_NO_BUFS          No available entry in the power calibration table.
 * @retval OT_ERROR_INVALID_ARGS     The @p aChannel, @p aActualPower or @p aRawPowerSetting is invalid or the
 *                                   @p aActualPower already exists in the power calibration table.
 * @retval OT_ERROR_NOT_IMPLEMENTED  This feature is not implemented.
 *
 */
otError otPlatRadioAddCalibratedPower(otInstance    *aInstance,
                                      uint8_t        aChannel,
                                      int16_t        aActualPower,
                                      const uint8_t *aRawPowerSetting,
                                      uint16_t       aRawPowerSettingLength);

/**
 * Clear all calibrated powers from the power calibration table.
 *
 * @note This API is an optional radio platform API. It's up to the platform layer to implement it.
 *
 * @param[in]  aInstance   The OpenThread instance structure.
 *
 * @retval OT_ERROR_NONE             Successfully cleared all calibrated powers from the power calibration table.
 * @retval OT_ERROR_NOT_IMPLEMENTED  This feature is not implemented.
 *
 */
otError otPlatRadioClearCalibratedPowers(otInstance *aInstance);

/**
 * Set the target power for the given channel.
 *
 * @note This API is an optional radio platform API. It's up to the platform layer to implement it.
 *       If this API is implemented, the function `otPlatRadioSetTransmitPower()` should be disabled.
 *
 * The radio driver should set the actual output power to be less than or equal to the target power and as close
 * as possible to the target power.
 *
 * @param[in]  aInstance     The OpenThread instance structure.
 * @param[in]  aChannel      The radio channel.
 * @param[in]  aTargetPower  The target power in 0.01dBm. Passing `INT16_MAX` will disable this channel to use the
 *                           target power.
 *
 * @retval  OT_ERROR_NONE             Successfully set the target power.
 * @retval  OT_ERROR_INVALID_ARGS     The @p aChannel or @p aTargetPower is invalid.
 * @retval  OT_ERROR_NOT_IMPLEMENTED  The feature is not implemented.
 *
 */
otError otPlatRadioSetChannelTargetPower(otInstance *aInstance, uint8_t aChannel, int16_t aTargetPower);

/**
 * Get the raw power setting for the given channel.
 *
 * @note OpenThread `src/core/utils` implements a default implementation of the API `otPlatRadioAddCalibratedPower()`,
 *       `otPlatRadioClearCalibratedPowers()` and `otPlatRadioSetChannelTargetPower()`. This API is provided by
 *       the default implementation to get the raw power setting for the given channel. If the platform doesn't
 *       use the default implementation, it can ignore this API.
 *
 * Platform radio layer should parse the raw power setting based on the radio layer defined format and set the
 * parameters of each radio hardware module.
 *
 * @param[in]      aInstance               The OpenThread instance structure.
 * @param[in]      aChannel                The radio channel.
 * @param[out]     aRawPowerSetting        A pointer to the raw power setting byte array.
 * @param[in,out]  aRawPowerSettingLength  On input, a pointer to the size of @p aRawPowerSetting.
 *                                         On output, a pointer to the length of the raw power setting data.
 *
 * @retval  OT_ERROR_NONE          Successfully got the target power.
 * @retval  OT_ERROR_INVALID_ARGS  The @p aChannel is invalid, @p aRawPowerSetting or @p aRawPowerSettingLength is NULL
 *                                 or @aRawPowerSettingLength is too short.
 * @retval  OT_ERROR_NOT_FOUND     The raw power setting for the @p aChannel was not found.
 *
 */
extern otError otPlatRadioGetRawPowerSetting(otInstance *aInstance,
                                             uint8_t     aChannel,
                                             uint8_t    *aRawPowerSetting,
                                             uint16_t   *aRawPowerSettingLength);

/**
 * @}
 *
 */

/**
 * @}
 *
 */

#ifdef __cplusplus
} // end of extern "C"
#endif

#endif // OPENTHREAD_PLATFORM_RADIO_H_
/**
 * @}
 */
