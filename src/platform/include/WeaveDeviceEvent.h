/*
 *
 *    Copyright (c) 2018 Nest Labs, Inc.
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
 *          Defines events used by the Weave Device Layer to signal actions
 *          or changes in device state.
 */

#ifndef WEAVE_DEVICE_EVENT_H
#define WEAVE_DEVICE_EVENT_H

namespace nl {
namespace Weave {
namespace DeviceLayer {

namespace DeviceEventType {

enum
{
    kFlag_IsPublic                      = 0x8000,
    kFlag_IsPlatformSpecific            = 0x4000,
    kFlag_Reserved1                     = 0x2000,
    kFlag_Reserved2                     = 0x1000,
    kMaxEventNum                        = 0x0FFF,
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
    kRange_Public                       = kFlag_IsPublic,

    /**
     * Public, Platform-specific Event Range
     *
     * Denotes a range of platform-specific event types that are publicly visible to applications.
     */
    kRange_PublicPlatformSpecific       = kFlag_IsPublic | kFlag_IsPlatformSpecific,

    /**
     * Internal Event Range
     *
     * Denotes a range of event types that are internal to the Weave Device Layer.  Events in this
     * range a generic to all platforms.
     */
    kRange_Internal                     = 0,

    /**
     * Internal, Platform-specific Event Range
     *
     * Denotes a range of platform-specific event types that are internal to the Weave Device Layer.
     */
    kRange_InternalPlatformSpecific     = kFlag_IsPlatformSpecific,
};

inline bool IsPublic(uint16_t eventType) { return (eventType & kFlag_IsPublic) != 0; }
inline bool IsInternal(uint16_t eventType) { return (eventType & kFlag_IsPublic) == 0; }
inline bool IsPlatformSpecific(uint16_t eventType) { return (eventType & kFlag_IsPlatformSpecific) != 0; }
inline bool IsPlatformGeneric(uint16_t eventType) { return (eventType & kFlag_IsPlatformSpecific) == 0; }

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
    kWiFiConnectivityChange             = kRange_Public,

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
     * Service Tunnel State Change
     *
     * Signals a change in connectivity of the device's IP tunnel to a Weave-enabled service.
     */
    kServiceTunnelStateChange,

    /**
     * Service Connectivity Change
     *
     * Signals a change in the device's ability to communicate with a Weave-enabled service.
     */
    kServiceConnectivityChange,

    /**
     * Service Subscription State Change
     *
     * Signals a change in the device's WDM subscription state with a Weave-enabled service.
     */
    kServiceSubscriptionStateChange,

    /**
     * Fabric Membership Change
     *
     * Signals a change in the device's membership in a Weave fabric.
     */
    kFabricMembershipChange,

    /**
     * Service Provisioning Change
     *
     * Signals a change to the device's service provisioning state.
     */
    kServiceProvisioningChange,

    /**
     * Account Pairing Change
     *
     * Signals a change to the device's state with respect to being paired to a user account.
     */
    kAccountPairingChange,

    /**
     * Time Sync Change
     *
     * Signals a change to the device's real time clock synchronization state.
     */
    kTimeSyncChange,

    /**
     * Security Session Established
     *
     * Signals that an external entity has established a new security session with the device.
     */
    kSessionEstablished,

    /**
     * WoBLE Connection Established
     *
     * Signals that an external entity has established a new WoBLE connection with the device.
     */
    kWoBLEConnectionEstablished,

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
     * Weave-over-BLE (WoBLE) Advertising Change
     *
     * Signals that the state of WoBLE advertising has changed.
     */
    kWoBLEAdvertisingChange,
};

/**
 * Internal Event Types
 *
 * Enumerates event types that are internal to the Weave Device Layer, but common across
 * all platforms.
 */
enum InternalEventTypes
{
    kEventTypeNotSet                    = kRange_Internal,
    kNoOp,
    kCallWorkFunct,
    kWeaveSystemLayerEvent,
    kWoBLESubscribe,
    kWoBLEUnsubscribe,
    kWoBLEWriteReceived,
    kWoBLEIndicateConfirm,
    kWoBLEConnectionError,
};

static_assert(kEventTypeNotSet == 0, "kEventTypeNotSet must be defined as 0");

} // namespace DeviceEventType

/**
 * Connectivity Change
 *
 * Describes a change in some aspect of connectivity associated with a Weave device.
 */
enum ConnectivityChange
{
    kConnectivity_NoChange      = 0,
    kConnectivity_Established   = 1,
    kConnectivity_Lost          = -1
};

/**
 * Activity Change
 *
 * Describes a change in some activity associated with a Weave device.
 */
enum ActivityChange
{
    kActivity_NoChange          = 0,
    kActivity_Started           = 1,
    kActivity_Stopped           = -1,
};

inline ConnectivityChange GetConnectivityChange(bool prevState, bool newState)
{
    if (prevState == newState)
        return kConnectivity_NoChange;
    else if (newState)
        return kConnectivity_Established;
    else
        return kConnectivity_Lost;
}

/**
 * A pointer to a function that performs work asynchronously.
 */
typedef void (*AsyncWorkFunct)(intptr_t arg);

} // namespace DeviceLayer
} // namespace Weave
} // namespace nl

/* Include a header file containing platform-specific event definitions.
 */
#ifdef EXTERNAL_WEAVEDEVICEPLATFORMEVENT_HEADER
#include EXTERNAL_WEAVEDEVICEPLATFORMEVENT_HEADER
#else
#define WEAVEDEVICEPLATFORMEVENT_HEADER <Weave/DeviceLayer/WEAVE_DEVICE_LAYER_TARGET/WeaveDevicePlatformEvent.h>
#include WEAVEDEVICEPLATFORMEVENT_HEADER
#endif

namespace nl {
namespace Weave {
namespace DeviceLayer {

/**
 * Represents a Weave Device Layer event.
 */
struct WeaveDeviceEvent final
{
    uint16_t Type;

    union
    {
        WeaveDevicePlatformEvent Platform;
        struct
        {
            ::nl::Weave::System::EventType Type;
            ::nl::Weave::System::Object * Target;
            uintptr_t Argument;
        } WeaveSystemLayerEvent;
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
        } InternetConnectivityChange;
        struct
        {
            ConnectivityChange Result;
            bool IsRestricted;
        } ServiceTunnelStateChange;
        struct
        {
            struct
            {
                ConnectivityChange Result;
            } Overall;
            struct
            {
                ConnectivityChange Result;
            } ViaTunnel;
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
            uint8_t EncType;
            ::nl::Weave::WeaveAuthMode AuthMode;
            bool IsCommissioner;
        } SessionEstablished;
        struct
        {
            BLE_CONNECTION_OBJECT ConId;
        } WoBLESubscribe;
        struct
        {
            BLE_CONNECTION_OBJECT ConId;
        } WoBLEUnsubscribe;
        struct
        {
            BLE_CONNECTION_OBJECT ConId;
            PacketBuffer * Data;
        } WoBLEWriteReceived;
        struct
        {
            BLE_CONNECTION_OBJECT ConId;
        } WoBLEIndicateConfirm;
        struct
        {
            BLE_CONNECTION_OBJECT ConId;
            WEAVE_ERROR Reason;
        } WoBLEConnectionError;
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
        } WoBLEAdvertisingChange;
    };

    void Clear() { memset(this, 0, sizeof(*this)); }
    bool IsPublic() const { return DeviceEventType::IsPublic(Type); }
    bool IsInternal() const { return DeviceEventType::IsInternal(Type); }
    bool IsPlatformSpecific() const { return DeviceEventType::IsPlatformSpecific(Type); }
    bool IsPlatformGeneric() const { return DeviceEventType::IsPlatformGeneric(Type); }
};

} // namespace DeviceLayer
} // namespace Weave
} // namespace nl

#endif // WEAVE_DEVICE_EVENT_H
