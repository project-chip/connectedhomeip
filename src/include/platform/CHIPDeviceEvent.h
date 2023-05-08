/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
 *    Copyright (c) 2018 Nest Labs, Inc.
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
 *          Defines events used by the chip Device Layer to signal actions
 *          or changes in device state.
 */

#pragma once
#include <stdint.h>

#include <inet/IPAddress.h>
#include <lib/core/DataModelTypes.h>

namespace chip {
namespace DeviceLayer {
namespace DeviceEventType {

enum
{
    kFlag_IsPublic           = 0x8000,
    kFlag_IsPlatformSpecific = 0x4000,
    kFlag_Reserved1          = 0x2000,
    kFlag_Reserved2          = 0x1000,
    kMaxEventNum             = 0x0FFF,
};

/**
 * Event Type Ranges
 *
 * Defines numeric ranges for event types based on their visibility to the application,
 * an whether or not they are specific to a particular platform adaptation.
 */
enum EventTypeRanges
{
    /**
     * Public Event Range
     *
     * Denotes a range of event types that are publicly visible to applications.  Events in this
     * range a generic to all platforms.
     */
    kRange_Public = kFlag_IsPublic,

    /**
     * Public, Platform-specific Event Range
     *
     * Denotes a range of platform-specific event types that are publicly visible to applications.
     */
    kRange_PublicPlatformSpecific = kFlag_IsPublic | kFlag_IsPlatformSpecific,

    /**
     * Internal Event Range
     *
     * Denotes a range of event types that are internal to the chip Device Layer.  Events in this
     * range a generic to all platforms.
     */
    kRange_Internal = 0,

    /**
     * Internal, Platform-specific Event Range
     *
     * Denotes a range of platform-specific event types that are internal to the chip Device Layer.
     */
    kRange_InternalPlatformSpecific = kFlag_IsPlatformSpecific,
};

inline bool IsPublic(uint16_t eventType)
{
    return (eventType & kFlag_IsPublic) != 0;
}
inline bool IsInternal(uint16_t eventType)
{
    return (eventType & kFlag_IsPublic) == 0;
}
inline bool IsPlatformSpecific(uint16_t eventType)
{
    return (eventType & kFlag_IsPlatformSpecific) != 0;
}
inline bool IsPlatformGeneric(uint16_t eventType)
{
    return (eventType & kFlag_IsPlatformSpecific) == 0;
}

/**
 * Public Event Types
 *
 * Enumerates event types that are visible to the application and common across
 * all platforms.
 */
enum PublicEventTypes
{
    /**
     * WiFi Connectivity Change
     *
     * Signals a change in connectivity of the device's WiFi station interface.
     */
    kWiFiConnectivityChange = kRange_Public,

    /**
     * Thread Connectivity Change
     *
     * Signals a change in connectivity of the device's Thread interface.
     */
    kThreadConnectivityChange,

    /**
     * Internet Connectivity Change
     *
     * Signals a change in the device's ability to communicate via the Internet.
     */
    kInternetConnectivityChange,

    /**
     * Service Connectivity Change
     *
     * Signals a change in the device's ability to communicate with a chip-enabled service.
     */
    kServiceConnectivityChange,

    /**
     * Service Provisioning Change
     *
     * Signals a change to the device's service provisioning state.
     */
    kServiceProvisioningChange,

    /**
     * Time Sync Change
     *
     * Signals a change to the device's real time clock synchronization state.
     */
    kTimeSyncChange,

    /**
     * SED Interval Change
     *
     * Signals a change to the sleepy end device interval.
     */
    kSEDIntervalChange,

    /**
     * CHIPoBLE Connection Established
     *
     * Signals that an external entity has established a new CHIPoBLE connection with the device.
     */
    kCHIPoBLEConnectionEstablished,

    /**
     * CHIPoBLE Connection Closed
     *
     * Signals that an external entity has closed existing CHIPoBLE connection with the device.
     */
    kCHIPoBLEConnectionClosed,

    /**
     * Thread State Change
     *
     * Signals that a state change has occurred in the Thread stack.
     */
    kThreadStateChange,

    /**
     * Thread Interface State Change
     *
     * Signals that the state of the Thread network interface has changed.
     */
    kThreadInterfaceStateChange,

    /**
     * chip-over-BLE (CHIPoBLE) Advertising Change
     *
     * Signals that the state of CHIPoBLE advertising has changed.
     */
    kCHIPoBLEAdvertisingChange,

    /**
     * IP address availability - either ipv4 or ipv6 addresses assigned to the underlying
     * wifi/ethernet interface.
     */
    kInterfaceIpAddressChanged,

    /**
     * Commissioning has completed by a call to the general commissioning cluster command.
     */
    kCommissioningComplete,

    /**
     * Signals that the fail-safe timer expired before the CommissioningComplete command was
     * successfully invoked.
     */
    kFailSafeTimerExpired,

    /**
     *
     */
    kOperationalNetworkEnabled,

    /**
     * Signals that DNS-SD has been initialized and is ready to operate.
     */
    kDnssdInitialized,

    /**
     * Signals that DNS-SD backend was restarted and services must be published again.
     */
    kDnssdRestartNeeded,

    /**
     * Signals that bindings were updated.
     */
    kBindingsChangedViaCluster,

    /**
     * Signals that the state of the OTA engine changed.
     */
    kOtaStateChanged,

    /**
     * Server initialization has completed.
     *
     * Signals that all server components have been initialized and the node is ready to establish
     * connections with other nodes. This event can be used to trigger on-boot actions that require
     * sending messages to other nodes.
     */
    kServerReady,
};

/**
 * Internal Event Types
 *
 * Enumerates event types that are internal to the chip Device Layer, but common across
 * all platforms.
 */
enum InternalEventTypes
{
    kEventTypeNotSet = kRange_Internal,
    kNoOp,
    kCallWorkFunct,
    kChipLambdaEvent,
    kChipSystemLayerEvent,
    kCHIPoBLESubscribe,
    kCHIPoBLEUnsubscribe,
    kCHIPoBLEWriteReceived,
    kCHIPoBLEIndicateConfirm,
    kCHIPoBLEConnectionError,
    kCHIPoBLENotifyConfirm
};

static_assert(kEventTypeNotSet == 0, "kEventTypeNotSet must be defined as 0");

} // namespace DeviceEventType

/**
 * Connectivity Change
 *
 * Describes a change in some aspect of connectivity associated with a chip device.
 */
enum ConnectivityChange
{
    kConnectivity_NoChange    = 0,
    kConnectivity_Established = 1,
    kConnectivity_Lost        = -1
};

enum class InterfaceIpChangeType
{
    kIpV4_Assigned,
    kIpV4_Lost,
    kIpV6_Assigned,
    kIpV6_Lost,
};

/**
 * Activity Change
 *
 * Describes a change in some activity associated with a chip device.
 */
enum ActivityChange
{
    kActivity_NoChange = 0,
    kActivity_Started  = 1,
    kActivity_Stopped  = -1,
};

enum OtaState
{
    kOtaSpaceAvailable = 0,
    /**
     * This state indicates that Node is currently downloading a software update.
     */
    kOtaDownloadInProgress,
    /**
     * This state indicates that Node has successfully downloaded a software update.
     */
    kOtaDownloadComplete,
    /**
     * This state indicates that Node has failed to download a software update.
     */
    kOtaDownloadFailed,
    /**
     * This state indicates that Node has aborted the download of a software update.
     */
    kOtaDownloadAborted,
    /**
     * This state indicate that Node is currently in the process of verifying and applying a software update.
     */
    kOtaApplyInProgress,
    /**
     * This state indicates that Node has successfully applied a software update.
     */
    kOtaApplyComplete,
    /**
     * This state indicates that Node has failed to apply a software update.
     */
    kOtaApplyFailed,
};

inline ConnectivityChange GetConnectivityChange(bool prevState, bool newState)
{
    if (prevState == newState)
        return kConnectivity_NoChange;
    if (newState)
        return kConnectivity_Established;
    return kConnectivity_Lost;
}

/**
 * A pointer to a function that performs work asynchronously.
 */
typedef void (*AsyncWorkFunct)(intptr_t arg);

} // namespace DeviceLayer
} // namespace chip

/* Include a header file containing platform-specific event definitions.
 */
#ifdef EXTERNAL_CHIPDEVICEPLATFORMEVENT_HEADER
#include EXTERNAL_CHIPDEVICEPLATFORMEVENT_HEADER
#elif defined(CHIP_DEVICE_LAYER_TARGET)
#define CHIPDEVICEPLATFORMEVENT_HEADER <platform/CHIP_DEVICE_LAYER_TARGET/CHIPDevicePlatformEvent.h>
#include CHIPDEVICEPLATFORMEVENT_HEADER
#endif // defined(CHIP_DEVICE_LAYER_TARGET)

#include <ble/BleConfig.h>
#include <inet/InetInterface.h>
#include <lib/support/LambdaBridge.h>
#include <system/SystemEvent.h>
#include <system/SystemLayer.h>
#include <system/SystemPacketBuffer.h>

namespace chip {
namespace DeviceLayer {

/**
 * Represents a chip Device Layer event.
 */
struct ChipDeviceEvent final
{
    uint16_t Type;

    union
    {
        ChipDevicePlatformEvent Platform;
        LambdaBridge LambdaEvent;
        struct
        {
            AsyncWorkFunct WorkFunct;
            intptr_t Arg;
        } CallWorkFunct;
        struct
        {
            ConnectivityChange Result;
        } WiFiConnectivityChange;
        struct
        {
            ConnectivityChange Result;
        } ThreadConnectivityChange;
        struct
        {
            ConnectivityChange IPv4;
            ConnectivityChange IPv6;
            // WARNING: There used to be `char address[INET6_ADDRSTRLEN]` here and it is
            //          deprecated/removed since it was too large and only used for logging.
            //          Consider not relying on ipAddress field either since the platform
            //          layer *does not actually validate* that the actual internet is reachable
            //          before issuing this event *and* there may be multiple addresses
            //          (especially IPv6) so it's recommended to use `ChipDevicePlatformEvent`
            //          instead and do something that is better for your platform.
            chip::Inet::IPAddress ipAddress;
        } InternetConnectivityChange;
        struct
        {
            struct
            {
                ConnectivityChange Result;
            } Overall;
            struct
            {
                ConnectivityChange Result;
            } ViaThread;
        } ServiceConnectivityChange;
        struct
        {
            ConnectivityChange Result;
        } ServiceSubscriptionStateChange;
        struct
        {
            bool IsMemberOfFabric;
        } FabricMembershipChange;
        struct
        {
            bool IsServiceProvisioned;
            bool ServiceConfigUpdated;
        } ServiceProvisioningChange;
        struct
        {
            bool IsPairedToAccount;
        } AccountPairingChange;
        struct
        {
            bool IsTimeSynchronized;
        } TimeSyncChange;
        struct
        {
            uint64_t PeerNodeId;
            uint16_t SessionKeyId;
            uint8_t SessionType;
            bool IsCommissioner;
        } SessionEstablished;
        struct
        {
            BLE_CONNECTION_OBJECT ConId;
        } CHIPoBLESubscribe;
        struct
        {
            BLE_CONNECTION_OBJECT ConId;
        } CHIPoBLEUnsubscribe;
        struct
        {
            BLE_CONNECTION_OBJECT ConId;
            chip::System::PacketBuffer * Data;
        } CHIPoBLEWriteReceived;
        struct
        {
            BLE_CONNECTION_OBJECT ConId;
        } CHIPoBLEIndicateConfirm;
        struct
        {
            BLE_CONNECTION_OBJECT ConId;
            CHIP_ERROR Reason;
        } CHIPoBLEConnectionError;
        struct
        {
            BLE_CONNECTION_OBJECT ConId;
        } CHIPoBLENotifyConfirm;
        struct
        {
            bool RoleChanged : 1;
            bool AddressChanged : 1;
            bool NetDataChanged : 1;
            bool ChildNodesChanged : 1;
            struct
            {
                uint32_t Flags;
            } OpenThread;
        } ThreadStateChange;
        struct
        {
            ActivityChange Result;
        } CHIPoBLEAdvertisingChange;
        struct
        {
            InterfaceIpChangeType Type;
        } InterfaceIpAddressChanged;

        struct
        {
            uint64_t nodeId;
            FabricIndex fabricIndex;
        } CommissioningComplete;
        struct
        {
            FabricIndex fabricIndex;
        } BindingsChanged;

        struct
        {
            FabricIndex fabricIndex;
            bool addNocCommandHasBeenInvoked;
            bool updateNocCommandHasBeenInvoked;
        } FailSafeTimerExpired;

        struct
        {
            // TODO(cecille): This should just specify wifi or thread since we assume at most 1.
            int network;
        } OperationalNetwork;

        struct
        {
            OtaState newState;
        } OtaStateChanged;
    };

    void Clear() { memset(this, 0, sizeof(*this)); }
    bool IsPublic() const { return DeviceEventType::IsPublic(Type); }
    bool IsInternal() const { return DeviceEventType::IsInternal(Type); }
    bool IsPlatformSpecific() const { return DeviceEventType::IsPlatformSpecific(Type); }
    bool IsPlatformGeneric() const { return DeviceEventType::IsPlatformGeneric(Type); }
};

} // namespace DeviceLayer
} // namespace chip
