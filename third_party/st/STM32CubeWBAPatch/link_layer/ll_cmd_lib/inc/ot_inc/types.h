/*$Id: //dwh/bluetooth/DWC_ble154combo/firmware/rel/2.00a-lca01/firmware/public_inc/ot_inc/types.h#1 $*/
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
 *  This file defines the types and structures used in the OpenThread library API.
 */

#ifndef OPENTHREAD_TYPES_H_
#define OPENTHREAD_TYPES_H_

#include <stdint.h>
#include <stdbool.h>
#ifdef OTDLL
#include <guiddef.h>
#endif

#include <toolchain.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifdef _WIN32
#pragma warning(disable:4214)  // nonstandard extension used: bit field types other than int
#ifdef _KERNEL_MODE
#include <ntdef.h>
#else
#include <windows.h>
#endif
#else
#ifndef CONTAINING_RECORD
/*#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Winvalid-offsetof"
#define CONTAINING_RECORD(address, type, field) \
    ((type *)((uint8_t*)(address) - offsetof(type, field)))
#pragma GCC diagnostic pop*/
#define BASE 0x1
#define myoffsetof(s,m) (((size_t)&(((s*)BASE)->m))-BASE)
#define CONTAINING_RECORD(address, type, field) \
    ((type *)((uint8_t*)(address) - myoffsetof(type, field)))
#endif /* CONTAINING_RECORD */
#endif /* _WIN32 */

/**
 * This type represents the OpenThread instance structure.
 */
typedef struct otInstance otInstance;

#ifdef OTDLL

/**
 * This type represents the handle to the OpenThread API.
 */
typedef struct otApiInstance otApiInstance;

/**
 * This type represents a list of device GUIDs.
 */
typedef struct otDeviceList
{
    uint16_t aDevicesLength;
    GUID     aDevices[1];
} otDeviceList;

#endif

/**
 * This enumeration represents error codes used throughout OpenThread.
 */
typedef enum otError
{
    /**
     * No error.
     */
    OT_ERROR_NONE = 0,

    /**
     * Operational failed.
     */
    OT_ERROR_FAILED = 1,

    /**
     * Message was dropped.
     */
    OT_ERROR_DROP = 2,

    /**
     * Insufficient buffers.
     */
    OT_ERROR_NO_BUFS = 3,

    /**
     * No route available.
     */
    OT_ERROR_NO_ROUTE = 4,

    /**
     * Service is busy and could not service the operation.
     */
    OT_ERROR_BUSY = 5,

    /**
     * Failed to parse message or arguments.
     */
    OT_ERROR_PARSE = 6,

    /**
     * Input arguments are invalid.
     */
    OT_ERROR_INVALID_ARGS = 7,

    /**
     * Security checks failed.
     */
    OT_ERROR_SECURITY = 8,

    /**
     * Address resolution requires an address query operation.
     */
    OT_ERROR_ADDRESS_QUERY = 9,

    /**
     * Address is not in the source match table.
     */
    OT_ERROR_NO_ADDRESS = 10,

    /**
     * Operation was aborted.
     */
    OT_ERROR_ABORT = 11,

    /**
     * Function or method is not implemented.
     */
    OT_ERROR_NOT_IMPLEMENTED = 12,

    /**
     * Cannot complete due to invalid state.
     */
    OT_ERROR_INVALID_STATE = 13,

    /**
     * No acknowledgment was received after macMaxFrameRetries (IEEE 802.15.4-2006).
     */
    OT_ERROR_NO_ACK = 14,

    /**
     * A transmission could not take place due to activity on the channel, i.e., the CSMA-CA mechanism has failed
     * (IEEE 802.15.4-2006).
     */
    OT_ERROR_CHANNEL_ACCESS_FAILURE = 15,

    /**
     * Not currently attached to a Thread Partition.
     */
    OT_ERROR_DETACHED = 16,

    /**
     * FCS check failure while receiving.
     */
    OT_ERROR_FCS = 17,

    /**
     * No frame received.
     */
    OT_ERROR_NO_FRAME_RECEIVED = 18,

    /**
     * Received a frame from an unknown neighbor.
     */
    OT_ERROR_UNKNOWN_NEIGHBOR = 19,

    /**
     * Received a frame from an invalid source address.
     */
    OT_ERROR_INVALID_SOURCE_ADDRESS = 20,

    /**
     * Received a frame filtered by the whitelist.
     */
    OT_ERROR_WHITELIST_FILTERED = 21,

    /**
     * Received a frame filtered by the destination address check.
     */
    OT_ERROR_DESTINATION_ADDRESS_FILTERED = 22,

    /**
     * The requested item could not be found.
     */
    OT_ERROR_NOT_FOUND = 23,

    /**
     * The operation is already in progress.
     */
    OT_ERROR_ALREADY = 24,

    /**
     * Received a frame filtered by the blacklist.
     */
    OT_ERROR_BLACKLIST_FILTERED = 25,

    /**
     * The creation of IPv6 address failed.
     */
    OT_ERROR_IP6_ADDRESS_CREATION_FAILURE = 26,

    /**
     * Operation prevented by mode flags
     */
    OT_ERROR_NOT_CAPABLE = 27,

    /**
     * Coap response or acknowledgment or DNS response not received.
     */
    OT_ERROR_RESPONSE_TIMEOUT = 28,

    /**
     * Received a duplicated frame.
     */
    OT_ERROR_DUPLICATED = 29,

    /**
     * Message is being dropped from reassembly list due to timeout.
     */
    OT_ERROR_REASSEMBLY_TIMEOUT = 30,

    /**
     * Message is not a TMF Message.
     */
    OT_ERROR_NOT_TMF = 31,

    /**
     * Received a non-lowpan data frame.
     */
    OT_ERROR_NOT_LOWPAN_DATA_FRAME = 32,

    /**
     * A feature/functionality disabled by build-time configuration options.
     */
    OT_ERROR_DISABLED_FEATURE = 33,

    /**
     * Generic error (should not use).
     */
    OT_ERROR_GENERIC = 255,
} otError;

#define OT_IP6_IID_SIZE            8   ///< Size of an IPv6 Interface Identifier (bytes)

#define OT_MASTER_KEY_SIZE         16  ///< Size of the Thread Master Key (bytes)

/**
 * Concatenated List of Type Identifiers of Other Diagnostics TLVs Used to Request or Reset Multiple Diagnostic Values.
 */
#define OT_NETWORK_DIAGNOSTIC_TYPELIST_TYPE          18

#define OT_NETWORK_DIAGNOSTIC_TYPELIST_MAX_ENTRIES   19  ///< Maximum Number of Other Network Diagnostic TLV Types

/**
 * @struct otMasterKey
 *
 * This structure represents a Thread Master Key.
 *
 */
OT_TOOL_PACKED_BEGIN
struct otMasterKey
{
    uint8_t m8[OT_MASTER_KEY_SIZE];    ///< Byte values
} OT_TOOL_PACKED_END;

/**
 * This type represents a Thread Master Key.
 *
 */
typedef struct otMasterKey otMasterKey;

#define OT_NETWORK_NAME_MAX_SIZE   16  ///< Maximum size of the Thread Network Name field (bytes)

/**
 * This structure represents a Network Name.
 *
 */
typedef struct otNetworkName
{
    char m8[OT_NETWORK_NAME_MAX_SIZE + 1];  ///< Byte values
} otNetworkName;

#define OT_EXT_PAN_ID_SIZE         8   ///< Size of a Thread PAN ID (bytes)

/**
 * This structure represents an Extended PAN ID.
 *
 */
typedef struct otExtendedPanId
{
    uint8_t m8[OT_EXT_PAN_ID_SIZE];   ///< Byte values
} otExtendedPanId;

#define OT_MESH_LOCAL_PREFIX_SIZE  8  ///< Size of the Mesh Local Prefix (bytes)

/**
 * This structure represents a Mesh Local Prefix.
 *
 */
typedef struct otMeshLocalPrefix
{
    uint8_t m8[OT_MESH_LOCAL_PREFIX_SIZE];  ///< Byte values
} otMeshLocalPrefix;

#define OT_PSKC_MAX_SIZE                             16  ///< Maximum size of the PSKc (bytes)

#define OT_COMMISSIONING_PASSPHRASE_MIN_SIZE         6   ///< Minimum size of the Commissioning Passphrase
#define OT_COMMISSIONING_PASSPHRASE_MAX_SIZE         255 ///< Maximum size of the Commissioning Passphrase

/**
  * This structure represents PSKc.
  *
  */
typedef struct otPSKc
{
    uint8_t m8[OT_PSKC_MAX_SIZE];  ///< Byte values
} otPSKc;

/**
  * This structure represent Security Policy.
  *
  */
typedef struct otSecurityPolicy
{
    uint16_t mRotationTime;  ///< The value for thrKeyRotation in units of hours
    uint8_t mFlags;          ///< Flags as defined in Thread 1.1 Section 8.10.1.15
} otSecurityPolicy;

/**
 * This enumeration defines the Security Policy TLV flags.
 *
 */
enum
{
    OT_SECURITY_POLICY_OBTAIN_MASTER_KEY      = 1 << 7,  ///< Obtaining the Master Key
    OT_SECURITY_POLICY_NATIVE_COMMISSIONING   = 1 << 6,  ///< Native Commissioning
    OT_SECURITY_POLICY_ROUTERS                = 1 << 5,  ///< Routers enabled
    OT_SECURITY_POLICY_EXTERNAL_COMMISSIONER  = 1 << 4,  ///< External Commissioner allowed
    OT_SECURITY_POLICY_BEACONS                = 1 << 3,  ///< Beacons enabled
};

/**
  * This type represents Channel Mask Page 0.
  *
  */
typedef uint32_t otChannelMaskPage0;

/**
 * This type represents the IEEE 802.15.4 PAN ID.
 *
 */
typedef uint16_t otPanId;

/**
 * This type represents the IEEE 802.15.4 Short Address.
 *
 */
typedef uint16_t otShortAddress;

#define OT_EXT_ADDRESS_SIZE        8   ///< Size of an IEEE 802.15.4 Extended Address (bytes)

/**
 * This type represents the IEEE 802.15.4 Extended Address.
 *
 */
typedef struct otExtAddress
{
    uint8_t m8[OT_EXT_ADDRESS_SIZE];  ///< IEEE 802.15.4 Extended Address bytes
} otExtAddress;

#define OT_IP6_PREFIX_SIZE         8   ///< Size of an IPv6 prefix (bytes)
#define OT_IP6_ADDRESS_SIZE        16  ///< Size of an IPv6 address (bytes)

/**
 * @struct otIp6Address
 *
 * This structure represents an IPv6 address.
 *
 */
OT_TOOL_PACKED_BEGIN
struct otIp6Address
{
    union
    {
        uint8_t  m8[OT_IP6_ADDRESS_SIZE];                      ///< 8-bit fields
        uint16_t m16[OT_IP6_ADDRESS_SIZE / sizeof(uint16_t)];  ///< 16-bit fields
        uint32_t m32[OT_IP6_ADDRESS_SIZE / sizeof(uint32_t)];  ///< 32-bit fields
    } mFields;                                                 ///< IPv6 accessor fields
} OT_TOOL_PACKED_END;

/**
 * This type represents an IPv6 address.
 *
 */
typedef struct otIp6Address otIp6Address;

/**
 * This structure represents the local and peer IPv6 socket addresses.
 */
typedef struct otMessageInfo
{
    /**
     * The local IPv6 address.
     */
    otIp6Address mSockAddr;

    /**
     * The peer IPv6 address.
     */
    otIp6Address mPeerAddr;

    /**
     * The local transport-layer port.
     */
    uint16_t mSockPort;

    /**
     * The peer transport-layer port.
     */
    uint16_t mPeerPort;

    /**
     * An IPv6 interface identifier.
     */
    int8_t mInterfaceId;

    /**
     * The IPv6 Hop Limit.
     */
    uint8_t mHopLimit;

    /**
     * A pointer to link-specific information. In case @p mInterfaceId is set to OT_NETIF_INTERFACE_ID_THREAD,
     * @p mLinkInfo points to @sa otThreadLinkInfo. This field is only valid for messages received from the
     * Thread radio and is ignored on transmission.
     */
    const void *mLinkInfo;
} otMessageInfo;

/**
 * This type points to an OpenThread message buffer.
 */
typedef struct otMessage
{
    struct otMessage *mNext;  ///< A pointer to the next Message buffer.
} otMessage;


#define OT_PANID_BROADCAST   0xffff      ///< IEEE 802.15.4 Broadcast PAN ID

#define OT_CHANNEL_11_MASK   (1 << 11)   ///< Channel 11
#define OT_CHANNEL_12_MASK   (1 << 12)   ///< Channel 12
#define OT_CHANNEL_13_MASK   (1 << 13)   ///< Channel 13
#define OT_CHANNEL_14_MASK   (1 << 14)   ///< Channel 14
#define OT_CHANNEL_15_MASK   (1 << 15)   ///< Channel 15
#define OT_CHANNEL_16_MASK   (1 << 16)   ///< Channel 16
#define OT_CHANNEL_17_MASK   (1 << 17)   ///< Channel 17
#define OT_CHANNEL_18_MASK   (1 << 18)   ///< Channel 18
#define OT_CHANNEL_19_MASK   (1 << 19)   ///< Channel 19
#define OT_CHANNEL_20_MASK   (1 << 20)   ///< Channel 20
#define OT_CHANNEL_21_MASK   (1 << 21)   ///< Channel 21
#define OT_CHANNEL_22_MASK   (1 << 22)   ///< Channel 22
#define OT_CHANNEL_23_MASK   (1 << 23)   ///< Channel 23
#define OT_CHANNEL_24_MASK   (1 << 24)   ///< Channel 24
#define OT_CHANNEL_25_MASK   (1 << 25)   ///< Channel 25
#define OT_CHANNEL_26_MASK   (1 << 26)   ///< Channel 26

#define OT_CHANNEL_ALL       0xffffffff  ///< All channels

#define OT_STEERING_DATA_MAX_LENGTH       16  ///< Max steering data length (bytes)

/**
 * This structure represents the steering data.
 *
 */
typedef struct otSteeringData
{
    uint8_t mLength;                          ///< Length of steering data (bytes)
    uint8_t m8[OT_STEERING_DATA_MAX_LENGTH];  ///< Byte values
} otSteeringData;

/**
 * This struct represents a received IEEE 802.15.4 Beacon.
 *
 */
typedef struct otActiveScanResult
{
    otExtAddress    mExtAddress;      ///< IEEE 802.15.4 Extended Address
    otNetworkName   mNetworkName;     ///< Thread Network Name
    otExtendedPanId mExtendedPanId;   ///< Thread Extended PAN ID
    otSteeringData  mSteeringData;    ///< Steering Data
    uint16_t        mPanId;           ///< IEEE 802.15.4 PAN ID
    uint16_t        mJoinerUdpPort;   ///< Joiner UDP Port
    uint8_t         mChannel;         ///< IEEE 802.15.4 Channel
    int8_t          mRssi;            ///< RSSI (dBm)
    uint8_t         mLqi;             ///< LQI
    unsigned int    mVersion : 4;     ///< Version
    bool            mIsNative : 1;    ///< Native Commissioner flag
    bool            mIsJoinable : 1;  ///< Joining Permitted flag
} otActiveScanResult;

/**
 * This struct represents an energy scan result.
 *
 */
typedef struct otEnergyScanResult
{
    uint8_t mChannel;                ///< IEEE 802.15.4 Channel
    int8_t  mMaxRssi;                ///< The max RSSI (dBm)
} otEnergyScanResult;

/**
 * This structure represents an Active or Pending Operational Dataset.
 *
 */
typedef struct otOperationalDataset
{
    uint64_t             mActiveTimestamp;            ///< Active Timestamp
    uint64_t             mPendingTimestamp;           ///< Pending Timestamp
    otMasterKey          mMasterKey;                  ///< Network Master Key
    otNetworkName        mNetworkName;                ///< Network Name
    otExtendedPanId      mExtendedPanId;              ///< Extended PAN ID
    otMeshLocalPrefix    mMeshLocalPrefix;            ///< Mesh Local Prefix
    uint32_t             mDelay;                      ///< Delay Timer
    otPanId              mPanId;                      ///< PAN ID
    uint16_t             mChannel;                    ///< Channel
    otPSKc               mPSKc;                       ///< PSKc
    otSecurityPolicy     mSecurityPolicy;             ///< Security Policy
    otChannelMaskPage0   mChannelMaskPage0;           ///< Channel Mask Page 0

    bool                 mIsActiveTimestampSet : 1;   ///< TRUE if Active Timestamp is set, FALSE otherwise.
    bool                 mIsPendingTimestampSet : 1;  ///< TRUE if Pending Timestamp is set, FALSE otherwise.
    bool                 mIsMasterKeySet : 1;         ///< TRUE if Network Master Key is set, FALSE otherwise.
    bool                 mIsNetworkNameSet : 1;       ///< TRUE if Network Name is set, FALSE otherwise.
    bool                 mIsExtendedPanIdSet : 1;     ///< TRUE if Extended PAN ID is set, FALSE otherwise.
    bool                 mIsMeshLocalPrefixSet : 1;   ///< TRUE if Mesh Local Prefix is set, FALSE otherwise.
    bool                 mIsDelaySet : 1;             ///< TRUE if Delay Timer is set, FALSE otherwise.
    bool                 mIsPanIdSet : 1;             ///< TRUE if PAN ID is set, FALSE otherwise.
    bool                 mIsChannelSet : 1;           ///< TRUE if Channel is set, FALSE otherwise.
    bool                 mIsPSKcSet : 1;              ///< TRUE if PSKc is set, FALSE otherwise.
    bool                 mIsSecurityPolicySet : 1;    ///< TRUE if Security Policy is set, FALSE otherwise.
    bool                 mIsChannelMaskPage0Set : 1;  ///< TRUE if Channel Mask Page 0 is set, FALSE otherwise.
} otOperationalDataset;

/**
 * This structure represents a Commissioning Dataset.
 *
 */
typedef struct otCommissioningDataset
{
    uint16_t              mLocator;                   ///< Border Router RLOC16
    uint16_t              mSessionId;                 ///< Commissioner Session Id
    otSteeringData        mSteeringData;              ///< Steering Data
    uint16_t              mJoinerUdpPort;             ///< Joiner UDP Port

    bool                  mIsLocatorSet : 1;          ///< TRUE if Border Router RLOC16 is set, FALSE otherwise.
    bool                  mIsSessionIdSet: 1;         ///< TRUE if Commissioner Session Id is set, FALSE otherwise.
    bool                  mIsSteeringDataSet : 1;     ///< TRUE if Steering Data is set, FALSE otherwise.
    bool                  mIsJoinerUdpPortSet : 1;    ///< TRUE if Joiner UDP Port is set, FALSE otherwise.
} otCommissioningDataset;

/**
 * This enumeration represents meshcop TLV types.
 *
 */
typedef enum otMeshcopTlvType
{
    OT_MESHCOP_TLV_CHANNEL                   = 0,    ///< meshcop Channel TLV
    OT_MESHCOP_TLV_PANID                     = 1,    ///< meshcop Pan Id TLV
    OT_MESHCOP_TLV_EXTPANID                  = 2,    ///< meshcop Extended Pan Id TLV
    OT_MESHCOP_TLV_NETWORKNAME               = 3,    ///< meshcop Network Name TLV
    OT_MESHCOP_TLV_PSKC                      = 4,    ///< meshcop PSKc TLV
    OT_MESHCOP_TLV_MASTERKEY                 = 5,    ///< meshcop Network Master Key TLV
    OT_MESHCOP_TLV_NETWORK_KEY_SEQUENCE      = 6,    ///< meshcop Network Key Sequence TLV
    OT_MESHCOP_TLV_MESHLOCALPREFIX           = 7,    ///< meshcop Mesh Local Prefix TLV
    OT_MESHCOP_TLV_STEERING_DATA             = 8,    ///< meshcop Steering Data TLV
    OT_MESHCOP_TLV_BORDER_AGENT_RLOC         = 9,    ///< meshcop Border Agent Locator TLV
    OT_MESHCOP_TLV_COMMISSIONER_ID           = 10,   ///< meshcop Commissioner ID TLV
    OT_MESHCOP_TLV_COMM_SESSION_ID           = 11,   ///< meshcop Commissioner Session ID TLV
    OT_MESHCOP_TLV_SECURITYPOLICY            = 12,   ///< meshcop Security Policy TLV
    OT_MESHCOP_TLV_GET                       = 13,   ///< meshcop Get TLV
    OT_MESHCOP_TLV_ACTIVETIMESTAMP           = 14,   ///< meshcop Active Timestamp TLV
    OT_MESHCOP_TLV_STATE                     = 16,   ///< meshcop State TLV
    OT_MESHCOP_TLV_JOINER_DTLS               = 17,   ///< meshcop Joiner DTLS Encapsulation TLV
    OT_MESHCOP_TLV_JOINER_UDP_PORT           = 18,   ///< meshcop Joiner UDP Port TLV
    OT_MESHCOP_TLV_JOINER_IID                = 19,   ///< meshcop Joiner IID TLV
    OT_MESHCOP_TLV_JOINER_RLOC               = 20,   ///< meshcop Joiner Router Locator TLV
    OT_MESHCOP_TLV_JOINER_ROUTER_KEK         = 21,   ///< meshcop Joiner Router KEK TLV
    OT_MESHCOP_TLV_PROVISIONING_URL          = 32,   ///< meshcop Provisioning URL TLV
    OT_MESHCOP_TLV_VENDOR_NAME_TLV           = 33,   ///< meshcop Vendor Name TLV
    OT_MESHCOP_TLV_VENDOR_MODEL_TLV          = 34,   ///< meshcop Vendor Model TLV
    OT_MESHCOP_TLV_VENDOR_SW_VERSION_TLV     = 35,   ///< meshcop Vendor SW Version TLV
    OT_MESHCOP_TLV_VENDOR_DATA_TLV           = 36,   ///< meshcop Vendor Data TLV
    OT_MESHCOP_TLV_VENDOR_STACK_VERSION_TLV  = 37,   ///< meshcop Vendor Stack Version TLV
    OT_MESHCOP_TLV_PENDINGTIMESTAMP          = 51,   ///< meshcop Pending Timestamp TLV
    OT_MESHCOP_TLV_DELAYTIMER                = 52,   ///< meshcop Delay Timer TLV
    OT_MESHCOP_TLV_CHANNELMASK               = 53,   ///< meshcop Channel Mask TLV
    OT_MESHCOP_TLV_COUNT                     = 54,   ///< meshcop Count TLV
    OT_MESHCOP_TLV_PERIOD                    = 55,   ///< meshcop Period TLV
    OT_MESHCOP_TLV_SCAN_DURATION             = 56,   ///< meshcop Scan Duration TLV
    OT_MESHCOP_TLV_ENERGY_LIST               = 57,   ///< meshcop Energy List TLV
    OT_MESHCOP_TLV_DISCOVERYREQUEST          = 128,  ///< meshcop Discovery Request TLV
    OT_MESHCOP_TLV_DISCOVERYRESPONSE         = 129,  ///< meshcop Discovery Response TLV
} otMeshcopTlvType;

/**
 * This structure represents an MLE Link Mode configuration.
 */
OT_TOOL_ALIGN(4)
typedef struct otLinkModeConfig
{
    /**
     * 1, if the sender has its receiver on when not transmitting.  0, otherwise.
     */
    bool mRxOnWhenIdle : 1;

    /**
     * 1, if the sender will use IEEE 802.15.4 to secure all data requests.  0, otherwise.
     */
    bool mSecureDataRequests : 1;

    /**
     * 1, if the sender is an FFD.  0, otherwise.
     */
    bool mDeviceType : 1;

    /**
     * 1, if the sender requires the full Network Data.  0, otherwise.
     */
    bool mNetworkData : 1;
} otLinkModeConfig;

/**
 * This enumeration represents flags that indicate what configuration or state has changed within OpenThread.
 *
 */
enum
{
    OT_CHANGED_IP6_ADDRESS_ADDED            = 1 << 0,   ///< IPv6 address was added
    OT_CHANGED_IP6_ADDRESS_REMOVED          = 1 << 1,   ///< IPv6 address was removed
    OT_CHANGED_THREAD_ROLE                  = 1 << 2,   ///< Role (disabled, detached, child, router, leader) changed
    OT_CHANGED_THREAD_LL_ADDR               = 1 << 3,   ///< The link-local address changed
    OT_CHANGED_THREAD_ML_ADDR               = 1 << 4,   ///< The mesh-local address changed
    OT_CHANGED_THREAD_RLOC_ADDED            = 1 << 5,   ///< RLOC was added
    OT_CHANGED_THREAD_RLOC_REMOVED          = 1 << 6,   ///< RLOC was removed
    OT_CHANGED_THREAD_PARTITION_ID          = 1 << 7,   ///< Partition ID changed
    OT_CHANGED_THREAD_KEY_SEQUENCE_COUNTER  = 1 << 8,   ///< Thread Key Sequence changed
    OT_CHANGED_THREAD_NETDATA               = 1 << 9,   ///< Thread Network Data changed
    OT_CHANGED_THREAD_CHILD_ADDED           = 1 << 10,  ///< Child was added
    OT_CHANGED_THREAD_CHILD_REMOVED         = 1 << 11,  ///< Child was removed
    OT_CHANGED_IP6_MULTICAST_SUBSRCRIBED    = 1 << 12,  ///< Subscribed to a IPv6 multicast address
    OT_CHANGED_IP6_MULTICAST_UNSUBSRCRIBED  = 1 << 13,  ///< Unsubscribed from a IPv6 multicast address
};

/**
 * This structure represents an IPv6 prefix.
 */
typedef struct otIp6Prefix
{
    otIp6Address  mPrefix;  ///< The IPv6 prefix.
    uint8_t       mLength;  ///< The IPv6 prefix length.
} otIp6Prefix;

#define OT_NETWORK_DATA_ITERATOR_INIT  0  ///< Initializer for otNetworkDataIterator.

typedef uint16_t otNetworkDataIterator;  ///< Used to iterate through Network Data information.

/**
 * This structure represents a Border Router configuration.
 */
typedef struct otBorderRouterConfig
{
    /**
     * The IPv6 prefix.
     */
    otIp6Prefix mPrefix;

    /**
     * A 2-bit signed integer indicating router preference as defined in RFC 4291.
     */
    int mPreference : 2;

    /**
     * TRUE, if @p mPrefix is preferred.  FALSE, otherwise.
     */
    bool mPreferred : 1;

    /**
     * TRUE, if @p mPrefix should be used for address autoconfiguration.  FALSE, otherwise.
     */
    bool mSlaac : 1;

    /**
     * TRUE, if this border router is a DHCPv6 Agent that supplies IPv6 address configuration.  FALSE, otherwise.
     */
    bool mDhcp : 1;

    /**
     * TRUE, if this border router is a DHCPv6 Agent that supplies other configuration data.  FALSE, otherwise.
     */
    bool mConfigure : 1;

    /**
     * TRUE, if this border router is a default route for @p mPrefix.  FALSE, otherwise.
     */
    bool mDefaultRoute : 1;

    /**
     * TRUE, if this prefix is considered on-mesh.  FALSE, otherwise.
     */
    bool mOnMesh : 1;

    /**
     * TRUE, if this configuration is considered Stable Network Data.  FALSE, otherwise.
     */
    bool mStable : 1;

    /**
     * The Border Agent Rloc.
     */
    uint16_t mRloc16;
} otBorderRouterConfig;

/**
 * This structure represents an External Route configuration.
 */
typedef struct otExternalRouteConfig
{
    /**
     * The prefix for the off-mesh route.
     */
    otIp6Prefix mPrefix;

    /**
     * The Rloc associated with the external route entry.
     *
     * This value is ignored when adding an external route. For any added route the device's Rloc will be used.
     */
    uint16_t mRloc16;

    /**
     * A 2-bit signed integer indicating router preference as defined in RFC 4291.
     */
    int mPreference : 2;

    /**
     * TRUE, if this configuration is considered Stable Network Data.  FALSE, otherwise.
     */
    bool mStable : 1;

    /**
     * TRUE if the external route entry's next hop is this device itself (i.e., the route was added earlier by this
     * device). FALSE otherwise.
     *
     * This value is ignored when adding an external route. For any added route the next hop is this device.
     */
    bool mNextHopIsThisDevice : 1;

} otExternalRouteConfig;

/**
 * Defines valid values for member mPreference in otExternalRouteConfig and otBorderRouterConfig.
 */
typedef enum otRoutePreference
{
    OT_ROUTE_PREFERENCE_LOW  = -1,  ///< Low route preference.
    OT_ROUTE_PREFERENCE_MED  = 0,   ///< Medium route preference.
    OT_ROUTE_PREFERENCE_HIGH = 1,   ///< High route preference.
} otRoutePreference;

/**
 * Used to indicate no fixed received signal strength was set
 */
#define OT_MAC_FILTER_FIXED_RSS_DISABLED       127

#define OT_MAC_FILTER_ITERATOR_INIT            0     ///< Initializer for otMacFilterIterator.

typedef uint8_t otMacFilterIterator;                 ///< Used to iterate through mac filter entries.

/**
 * Defines address mode of the mac filter.
 */
typedef enum otMacFilterAddressMode
{
    OT_MAC_FILTER_ADDRESS_MODE_DISABLED,     ///< Address filter is disabled.
    OT_MAC_FILTER_ADDRESS_MODE_WHITELIST,    ///< Whitelist address filter mode is enabled.
    OT_MAC_FILTER_ADDRESS_MODE_BLACKLIST,    ///< Blacklist address filter mode is enabled.
} otMacFilterAddressMode;

/**
 * This structure represents a Mac Filter entry.
 *
 */
typedef struct otMacFilterEntry
{
    otExtAddress mExtAddress;       ///< IEEE 802.15.4 Extended Address
    int8_t       mRssIn;            ///< Received signal strength
    bool         mFiltered;         ///< Indicates whether or not this entry is filtered.
} otMacFilterEntry;

/**
 * Represents a Thread device role.
 */
typedef enum
{
    OT_DEVICE_ROLE_DISABLED = 0,  ///< The Thread stack is disabled.
    OT_DEVICE_ROLE_DETACHED = 1,  ///< Not currently participating in a Thread network/partition.
    OT_DEVICE_ROLE_CHILD    = 2,  ///< The Thread Child role.
    OT_DEVICE_ROLE_ROUTER   = 3,  ///< The Thread Router role.
    OT_DEVICE_ROLE_LEADER   = 4,  ///< The Thread Leader role.
} otDeviceRole;

/**
 * This structure holds diagnostic information for a neighboring Thread node
 *
 */
typedef struct
{
    otExtAddress   mExtAddress;            ///< IEEE 802.15.4 Extended Address
    uint32_t       mAge;                   ///< Time last heard
    uint16_t       mRloc16;                ///< RLOC16
    uint32_t       mLinkFrameCounter;      ///< Link Frame Counter
    uint32_t       mMleFrameCounter;       ///< MLE Frame Counter
    uint8_t        mLinkQualityIn;         ///< Link Quality In
    int8_t         mAverageRssi;           ///< Average RSSI
    int8_t         mLastRssi;              ///< Last observed RSSI
    bool           mRxOnWhenIdle : 1;      ///< rx-on-when-idle
    bool           mSecureDataRequest : 1; ///< Secure Data Requests
    bool           mFullFunction : 1;      ///< Full Function Device
    bool           mFullNetworkData : 1;   ///< Full Network Data
    bool           mIsChild : 1;           ///< Is the neighbor a child
} otNeighborInfo;

#define OT_NEIGHBOR_INFO_ITERATOR_INIT  0  ///< Initializer for otNeighborInfoIterator.

typedef int16_t otNeighborInfoIterator;    ///< Used to iterate through neighbor table.

/**
 * This structure holds diagnostic information for a Thread Child
 *
 */
typedef struct
{
    otExtAddress   mExtAddress;            ///< IEEE 802.15.4 Extended Address
    uint32_t       mTimeout;               ///< Timeout
    uint32_t       mAge;                   ///< Time last heard
    uint16_t       mRloc16;                ///< RLOC16
    uint16_t       mChildId;               ///< Child ID
    uint8_t        mNetworkDataVersion;    ///< Network Data Version
    uint8_t        mLinkQualityIn;         ///< Link Quality In
    int8_t         mAverageRssi;           ///< Average RSSI
    int8_t         mLastRssi;              ///< Last observed RSSI
    bool           mRxOnWhenIdle : 1;      ///< rx-on-when-idle
    bool           mSecureDataRequest : 1; ///< Secure Data Requests
    bool           mFullFunction : 1;      ///< Full Function Device
    bool           mFullNetworkData : 1;   ///< Full Network Data
} otChildInfo;

/**
 * This structure holds diagnostic information for a Thread Router
 *
 */
OT_TOOL_ALIGN(4)
typedef struct
{
    otExtAddress   mExtAddress;            ///< IEEE 802.15.4 Extended Address
    uint16_t       mRloc16;                ///< RLOC16
    uint8_t        mRouterId;              ///< Router ID
    uint8_t        mNextHop;               ///< Next hop to router
    uint8_t        mPathCost;              ///< Path cost to router
    uint8_t        mLinkQualityIn;         ///< Link Quality In
    uint8_t        mLinkQualityOut;        ///< Link Quality Out
    uint8_t        mAge;                   ///< Time last heard
    bool           mAllocated : 1;         ///< Router ID allocated or not
    bool           mLinkEstablished : 1;   ///< Link established with Router ID or not
} otRouterInfo;

/**
 * This structure represents an EID cache entry.
 *
 */
typedef struct otEidCacheEntry
{
    otIp6Address    mTarget;          ///< Target
    otShortAddress  mRloc16;          ///< RLOC16
    bool            mValid : 1;       ///< Indicates whether or not the cache entry is valid
} otEidCacheEntry;

/**
 * This structure represents the Thread Leader Data.
 *
 */
typedef struct otLeaderData
{
    uint32_t mPartitionId;            ///< Partition ID
    uint8_t mWeighting;               ///< Leader Weight
    uint8_t mDataVersion;             ///< Full Network Data Version
    uint8_t mStableDataVersion;       ///< Stable Network Data Version
    uint8_t mLeaderRouterId;          ///< Leader Router ID
} otLeaderData;

/**
 * This structure represents the MAC layer counters.
 */
typedef struct otMacCounters
{
    uint32_t mTxTotal;                ///< The total number of transmissions.
    uint32_t mTxUnicast;              ///< The total number of unicast transmissions.
    uint32_t mTxBroadcast;            ///< The total number of broadcast transmissions.
    uint32_t mTxAckRequested;         ///< The number of transmissions with ack request.
    uint32_t mTxAcked;                ///< The number of transmissions that were acked.
    uint32_t mTxNoAckRequested;       ///< The number of transmissions without ack request.
    uint32_t mTxData;                 ///< The number of transmitted data.
    uint32_t mTxDataPoll;             ///< The number of transmitted data poll.
    uint32_t mTxBeacon;               ///< The number of transmitted beacon.
    uint32_t mTxBeaconRequest;        ///< The number of transmitted beacon request.
    uint32_t mTxOther;                ///< The number of transmitted other types of frames.
    uint32_t mTxRetry;                ///< The number of retransmission times.
    uint32_t mTxErrCca;               ///< The number of CCA failure times.
    uint32_t mTxErrAbort;             ///< The number of frame transmission failures due to abort error.
    uint32_t mRxTotal;                ///< The total number of received packets.
    uint32_t mRxUnicast;              ///< The total number of unicast packets received.
    uint32_t mRxBroadcast;            ///< The total number of broadcast packets received.
    uint32_t mRxData;                 ///< The number of received data.
    uint32_t mRxDataPoll;             ///< The number of received data poll.
    uint32_t mRxBeacon;               ///< The number of received beacon.
    uint32_t mRxBeaconRequest;        ///< The number of received beacon request.
    uint32_t mRxOther;                ///< The number of received other types of frames.
    uint32_t mRxWhitelistFiltered;    ///< The number of received packets filtered by whitelist.
    uint32_t mRxDestAddrFiltered;     ///< The number of received packets filtered by destination check.
    uint32_t mRxDuplicated;           ///< The number of received duplicated packets.
    uint32_t mRxErrNoFrame;           ///< The number of received packets that do not contain contents.
    uint32_t mRxErrUnknownNeighbor;   ///< The number of received packets from unknown neighbor.
    uint32_t mRxErrInvalidSrcAddr;    ///< The number of received packets whose source address is invalid.
    uint32_t mRxErrSec;               ///< The number of received packets with security error.
    uint32_t mRxErrFcs;               ///< The number of received packets with FCS error.
    uint32_t mRxErrOther;             ///< The number of received packets with other error.
} otMacCounters;

/**
 * This structure represents the IP level counters
 */
typedef struct otIpCounters
{
    uint32_t mTxSuccess;              ///< The number of IPv6 packets successfully transmitted.
    uint32_t mRxSuccess;              ///< The number of IPv6 packets successfully received.
    uint32_t mTxFailure;              ///< The number of IPv6 packets failed to transmit.
    uint32_t mRxFailure;              ///< The number of IPv6 packets failed to receive.
} otIpCounters;

/**
 * This structure represents the message buffer information.
 */
typedef struct otBufferInfo
{
    uint16_t mTotalBuffers;           ///< The number of buffers in the pool.
    uint16_t mFreeBuffers;            ///< The number of free message buffers.
    uint16_t m6loSendMessages;        ///< The number of messages in the 6lo send queue.
    uint16_t m6loSendBuffers;         ///< The number of buffers in the 6lo send queue.
    uint16_t m6loReassemblyMessages;  ///< The number of messages in the 6LoWPAN reassembly queue.
    uint16_t m6loReassemblyBuffers;   ///< The number of buffers in the 6LoWPAN reassembly queue.
    uint16_t mIp6Messages;            ///< The number of messages in the IPv6 send queue.
    uint16_t mIp6Buffers;             ///< The number of buffers in the IPv6 send queue.
    uint16_t mMplMessages;            ///< The number of messages in the MPL send queue.
    uint16_t mMplBuffers;             ///< The number of buffers in the MPL send queue.
    uint16_t mMleMessages;            ///< The number of messages in the MLE send queue.
    uint16_t mMleBuffers;             ///< The number of buffers in the MLE send queue.
    uint16_t mArpMessages;            ///< The number of messages in the ARP send queue.
    uint16_t mArpBuffers;             ///< The number of buffers in the ARP send queue.
    uint16_t mCoapMessages;           ///< The number of messages in the CoAP send queue.
    uint16_t mCoapBuffers;            ///< The number of buffers in the CoAP send queue.
    uint16_t mCoapSecureMessages;     ///< The number of messages in the CoAP secure send queue.
    uint16_t mCoapSecureBuffers;      ///< The number of buffers in the CoAP secure send queue.
} otBufferInfo;

/**
 * This structure represents an IPv6 network interface unicast address.
 *
 */
typedef struct otNetifAddress
{
    otIp6Address           mAddress;                 ///< The IPv6 unicast address.
    uint8_t                mPrefixLength;            ///< The Prefix length.
    bool                   mPreferred : 1;           ///< TRUE if the address is preferred, FALSE otherwise.
    bool                   mValid : 1;               ///< TRUE if the address is valid, FALSE otherwise.
    bool                   mScopeOverrideValid : 1;  ///< TRUE if the mScopeOverride value is valid, FALSE otherwise.
    unsigned int           mScopeOverride : 4;       ///< The IPv6 scope of this address.
    bool                   mRloc : 1;                ///< TRUE if the address is an RLOC, FALSE otherwise.
    struct otNetifAddress *mNext;                    ///< A pointer to the next network interface address.
} otNetifAddress;

/**
 * This structure represents an IPv6 network interface multicast address.
 *
 */
typedef struct otNetifMulticastAddress
{
    otIp6Address                    mAddress;   ///< The IPv6 multicast address.
    struct otNetifMulticastAddress *mNext;      ///< A pointer to the next network interface multicast address.
} otNetifMulticastAddress;

/**
 * This enumeration represents the list of allowable values for an InterfaceId.
 */
typedef enum otNetifInterfaceId
{
    OT_NETIF_INTERFACE_ID_THREAD = 1,  ///< The Thread Network interface ID.
} otNetifInterfaceId;

/**
 * This structure represents data used by Semantically Opaque IID Generator.
 *
 */
typedef struct
{
    uint8_t        *mInterfaceId;        ///< String of bytes representing interface ID. Like "eth0" or "wlan0".
    uint8_t         mInterfaceIdLength;  ///< Length of interface ID string.

    uint8_t        *mNetworkId;          ///< Network ID (or name). Can be null if mNetworkIdLength is 0.
    uint8_t         mNetworkIdLength;    ///< Length of Network ID string.

    uint8_t         mDadCounter;         ///< Duplicate address detection counter.

    uint8_t        *mSecretKey;          ///< Secret key used to create IID. Cannot be null.
    uint16_t        mSecretKeyLength;    ///< Secret key length in bytes. Should be at least 16 bytes == 128 bits.
} otSemanticallyOpaqueIidGeneratorData;

/**
 * This structure represents an IPv6 socket address.
 */
typedef struct otSockAddr
{
    otIp6Address mAddress;  ///< An IPv6 address.
    uint16_t     mPort;     ///< A transport-layer port.
    int8_t       mScopeId;  ///< An IPv6 scope identifier.
} otSockAddr;

/**
 * This structure represents link-specific information for messages received from the Thread radio.
 *
 */
typedef struct otThreadLinkInfo
{
    uint16_t mPanId;         ///< Source PAN ID
    uint8_t  mChannel;       ///< 802.15.4 Channel
    int8_t   mRss;           ///< Received Signal Strength in dBm.
    uint8_t  mLqi;           ///< Link Quality Indicator for a received message.
    bool     mLinkSecurity;  ///< Indicates whether or not link security is enabled.
} otThreadLinkInfo;

#ifdef OTDLL

/**
 * This function pointer is called to notify addition and removal of OpenThread devices.
 *
 * @param[in]  aAdded       A flag indicating if the device was added or removed.
 * @param[in]  aDeviceGuid  A GUID indicating which device state changed.
 * @param[in]  aContext     A pointer to application-specific context.
 *
 */
typedef void (OTCALL *otDeviceAvailabilityChangedCallback)(bool aAdded, const GUID *aDeviceGuid, void *aContext);

#endif // OTDLL

#ifdef __cplusplus
}  // extern "C"
#endif

#endif  // OPENTHREAD_TYPES_H_
