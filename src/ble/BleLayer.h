/*
 *
 *    Copyright (c) 2014-2017 Nest Labs, Inc.
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
 *      This file defines objects that provide an abstraction layer between a
 *      platform's Bluetooth Low Energy (BLE) implementation and the Weave
 *      stack.
 *
 *      The BleLayer obect accepts BLE data and control input from the
 *      application via a functional interface. It performs the fragmentation
 *      and reassembly required to transmit Weave message via a BLE GATT
 *      characteristic interface, and drives incoming messages up the Weave
 *      stack.
 *
 *      During initialization, the BleLayer object requires a pointer to the
 *      platform's implementation of the BleAdapter object. This object is
 *      defined but not implemented by the Weave stack, and provides the
 *      BleLayer with a functional interface to drive outgoing GATT
 *      characteristic writes and indications. It also provides a mechanism
 *      for Weave to inform the application when it has finished using a given
 *      BLE connection, i.e., when the WeaveConnection object wrapping this
 *      connection has closed.
 *
 *      To enable Weave over BLE for a new platform, the application developer
 *      must implement the BleAdapter class for their platform, pass it to the
 *      BleLayer on startup, pass a pointer to this BleLayer to their instance
 *      of WeaveMessageLayer, and ensure that the application calls the
 *      necessary BleLayer functions to drive BLE data and control input up the
 *      stack.
 */

#ifndef BLELAYER_H_
#define BLELAYER_H_

#ifndef __STDC_LIMIT_MACROS
#define __STDC_LIMIT_MACROS
#endif
#include <stdint.h>

#include <BleLayer/BleConfig.h>

#include <SystemLayer/SystemLayer.h>
#include <SystemLayer/SystemPacketBuffer.h>

#include <BleLayer/BlePlatformDelegate.h>
#include <BleLayer/BleApplicationDelegate.h>
#include <BleLayer/BleUUID.h>
#include <BleLayer/BleError.h>

#if BLE_CONFIG_PROVIDE_OBSOLESCENT_INTERFACES
#include <InetLayer/InetLayer.h>
#endif // BLE_CONFIG_PROVIDE_OBSOLESCENT_INTERFACES

namespace nl {
namespace Ble {

using ::nl::Weave::System::PacketBuffer;

/**
 *  @def NUM_SUPPORTED_PROTOCOL_VERSIONS
 *
 *  Number of unsigned 4-bit representations of supported transport protocol
 *  versions encapsulated in a BleTransportCapabilitiesRequest. Defined by Weave
 *  over BLE protocol specification.
 */
#define NUM_SUPPORTED_PROTOCOL_VERSIONS     8
/// Version(s) of the Nest BLE Transport Protocol that this stack supports.
#define NL_BLE_TRANSPORT_PROTOCOL_MIN_SUPPORTED_VERSION kBleTransportProtocolVersion_V2
#define NL_BLE_TRANSPORT_PROTOCOL_MAX_SUPPORTED_VERSION kBleTransportProtocolVersion_V3

/// Forward declarations.
class BleLayer;
class BLEEndPoint;

/// Role of end points' associated BLE connections. Determines means used by end points to send and receive data.
typedef enum
{
    kBleRole_Central    = 0,
    kBleRole_Peripheral = 1
} BleRole;

/// Enum defining versions of Weave over BLE transport protocol.
typedef enum
{
    kBleTransportProtocolVersion_None = 0,
    kBleTransportProtocolVersion_V1   = 1, // Prototype WoBLe version without ACKs or flow-control.
    kBleTransportProtocolVersion_V2   = 2, // First WoBLE version with ACKs and flow-control.
    kBleTransportProtocolVersion_V3   = 3  // First WoBLE version with asymetric fragement sizes.
} BleTransportProtocolVersion;

class BleLayerObject
{
    friend class BleLayer;

public:
    // Public data members:
    BleLayer * mBle;  ///< [READ-ONLY] Pointer to the BleLayer object that owns this object.
    void * mAppState; ///< Generic pointer to app-specific data associated with the object.

protected:
    uint32_t mRefCount;

    void AddRef(void) { mRefCount++; }
    void Release(void);
};

class BleTransportCapabilitiesRequestMessage
{
public:
    /**
     * An array of size NUM_SUPPORTED_PROTOCOL_VERSIONS listing versions of the
     * BLE transport protocol that this node supports. Each protocol version is
     * specified as a 4-bit unsigned integer. A zero-value represents unused
     * array elements. Counting up from the zero-index, the first zero-value
     * specifies the end of the list of supported protocol versions.
     */
    uint8_t mSupportedProtocolVersions[(NUM_SUPPORTED_PROTOCOL_VERSIONS / 2) + (NUM_SUPPORTED_PROTOCOL_VERSIONS % 2)];

    /**
     *  The MTU that has been negotiated for this BLE connection. Specified in
     *  the BleTransportCapabilitiesRequestMessage because the remote node may
     *  be unable to glean this info from its own BLE hardware/software stack,
     *  such as on older Android platforms.
     *
     *  A value of 0 means that the central could not determine the negotiated
     *  BLE connection MTU.
     */
    uint16_t mMtu;

    /**
     *  The initial and maximum receive window size offered by the central,
     *  defined in terms of GATT indication payloads.
     */
    uint8_t mWindowSize;

    /**
     *  Set supported version value at given index in
     *  SupportedProtocolVersions. uint8_t version argument is truncated to 4
     *  least-significant bits. Index shall be 0 through number of
     *  SupportedProtocolVersions elements - 1.
     */
    void SetSupportedProtocolVersion(uint8_t index, uint8_t version);

    /// Must be able to reserve 20 byte data length in msgBuf.
    BLE_ERROR Encode(PacketBuffer * msgBuf) const;

    static BLE_ERROR Decode(const PacketBuffer & msgBuf, BleTransportCapabilitiesRequestMessage & msg);
};

class BleTransportCapabilitiesResponseMessage
{
public:
    /**
     *  The lower 4 bits specify the BLE transport protocol version that the BLE
     *  peripheral has selected for this connection.
     *
     *  A value of kBleTransportProtocolVersion_None means that no supported
     *  protocol version was found in the central's capabilities request. The
     *  central should unsubscribe after such a response has been sent to free
     *  up the peripheral for connections from devices with supported protocol
     *  versions.
     */
    uint8_t mSelectedProtocolVersion;

    /**
     *  BLE transport fragment size selected by peripheral in response to MTU
     *  value in BleTransportCapabilitiesRequestMessage and its local
     *  observation of the BLE connection MTU.
     */
    uint16_t mFragmentSize;

    /**
     *  The initial and maximum receive window size offered by the peripheral,
     *  defined in terms of GATT write payloads.
     */
    uint8_t mWindowSize;

    /// Must be able to reserve 20 byte data length in msgBuf.
    BLE_ERROR Encode(PacketBuffer * msgBuf) const;

    static BLE_ERROR Decode(const PacketBuffer & msgBuf, BleTransportCapabilitiesResponseMessage & msg);
};

/**
 *  @class BleLayer
 *
 *  @brief
 *    This class provides an interface for a single thread to drive data
 *    either up the stack via the BleLayer platform interface functions,
 *    or down the stack via a WeaveConnection object associated with a
 *    BLEEndPoint.
 *
 *    There are two ways to associate a WeaveConnection (defined by the
 *    WeaveMessageLayer) with a BLE connection:
 *
 *    First, the application can passively receive an incoming BLE connection
 *    and hand the platform-specific BLE_CONNECTION_OBJECT that this receipt
 *    generates to BleLayer via the corresponding platform interface function.
 *    This causes BleLayer to wrap the BLE_CONNECTION_OBJECT in a BLEEndPoint,
 *    and notify WeaveMessageLayer that a new BLE conneciotn has been received.
 *    The message layer then wraps the new BLEEndPoint object in a
 *    WeaveConnection, and hands this object to the application via the message
 *    layer's OnConnectionReceived callback.
 *
 *    Second, the application can actively form an outgoing BLE connection, e.g.,
 *    by connecting to a BLE peripheral. It then creates a new WeaveConnection
 *    via the WeaveMessageLayer, assigns an authentication type to this
 *    connection, and binds it to the BLE_CONNECTION_OBJECT for the new BLE
 *    connection via WeaveConnection::ConnectBle. This function then
 *    establishes the secure session type specified by the WeaveConnection's
 *    authentication type member variable.
 *
 */
class NL_DLL_EXPORT BleLayer
{
    friend class BLEEndPoint;
#if WEAVE_ENABLE_WOBLE_TEST
    friend class WoBleTest;
#endif

public:
    // Public data members:
    enum
    {
        kState_NotInitialized = 0,
        kState_Initialized    = 1
    } mState; ///< [READ-ONLY] Current state

    void * mAppState;

    typedef void (*BleConnectionReceivedFunct)(BLEEndPoint * newEndPoint);
    BleConnectionReceivedFunct OnWeaveBleConnectReceived;

public:
    // Public functions:
    BleLayer(void);

    BLE_ERROR Init(BlePlatformDelegate * platformDelegate, BleApplicationDelegate * appDelegate,
                   Weave::System::Layer * systemLayer);

#if BLE_CONFIG_PROVIDE_OBSOLESCENT_INTERFACES
    BLE_ERROR Init(BlePlatformDelegate * platformDelegate, BleApplicationDelegate * appDelegate, Inet::InetLayer * inetLayer);
#endif // BLE_CONFIG_PROVIDE_OBSOLESCENT_INTERFACES

    BLE_ERROR Shutdown(void);

    BLE_ERROR NewBleEndPoint(BLEEndPoint ** retEndPoint, BLE_CONNECTION_OBJECT connObj, BleRole role, bool autoClose);

    nl::Weave::System::Error ScheduleWork(nl::Weave::System::Layer::TimerCompleteFunct aComplete, void* aAppState)
    {
        return mSystemLayer->ScheduleWork(aComplete, aAppState);
    }

    /**< Platform interface functions:

     *   Calling conventions:
     *     Weave takes ownership of PacketBuffers received through these functions,
     *     and will free them or pass ownership up the stack.
     *
     *     Beyond each call, no guarantees are provided as to the lifetime of UUID arguments.
     *
     *     A 'true' return value means the Weave stack successfully handled the
     *     corresponding message or state indication. 'false' means the Weave stack either
     *     failed or chose not to handle this. In case of 'false,' the Weave stack will not
     *     have freed or taken ownership of any PacketBuffer arguments. This contract allows the
     *     platform to pass BLE events to Weave without needing to know which characteristics
     *     Weave cares about.

     *     Platform must call this function when a GATT subscription has been established to any Weave service
     *     charateristic.
     *
     *     If this function returns true, Weave has accepted the BLE connection and wrapped it
     *     in a WeaveConnection object. If Weave accepts a BLE connection, the platform MUST
     *     notify Weave if the subscription is canceled or the underlying BLE connection is
     *     closed, or the associated WeaveConnection will never be closed or freed. */
    bool HandleSubscribeReceived(BLE_CONNECTION_OBJECT connObj, const WeaveBleUUID * svcId, const WeaveBleUUID * charId);

    /// Call when a GATT subscribe request succeeds.
    bool HandleSubscribeComplete(BLE_CONNECTION_OBJECT connObj, const WeaveBleUUID * svcId, const WeaveBleUUID * charId);

    /**< Platform must call this function when a GATT unsubscribe is requested on any Weave
     *   service charateristic, that is, when an existing GATT subscription on a Weave service
     *   characteristic is canceled. */
    bool HandleUnsubscribeReceived(BLE_CONNECTION_OBJECT connObj, const WeaveBleUUID * svcId, const WeaveBleUUID * charId);

    /// Call when a GATT unsubscribe request succeeds.
    bool HandleUnsubscribeComplete(BLE_CONNECTION_OBJECT connObj, const WeaveBleUUID * svcId, const WeaveBleUUID * charId);

    /// Call when a GATT write request is received.
    bool HandleWriteReceived(BLE_CONNECTION_OBJECT connObj, const WeaveBleUUID * svcId, const WeaveBleUUID * charId,
                             PacketBuffer * pBuf);

    /// Call when a GATT indication is received.
    bool HandleIndicationReceived(BLE_CONNECTION_OBJECT connObj, const WeaveBleUUID * svcId, const WeaveBleUUID * charId,
                                  PacketBuffer * pBuf);

    /// Call when an outstanding GATT write request receives a positive receipt confirmation.
    bool HandleWriteConfirmation(BLE_CONNECTION_OBJECT connObj, const WeaveBleUUID * svcId, const WeaveBleUUID * charId);

    /// Call when an oustanding GATT indication receives a positive receipt confirmation.
    bool HandleIndicationConfirmation(BLE_CONNECTION_OBJECT connObj, const WeaveBleUUID * svcId, const WeaveBleUUID * charId);

    /// Call when a GATT read request is received.
    bool HandleReadReceived(BLE_CONNECTION_OBJECT connObj, BLE_READ_REQUEST_CONTEXT requestContext, const WeaveBleUUID * svcId,
                            const WeaveBleUUID * charId);

    /**< Platform must call this function when any previous operation undertaken by the BleLayer via BleAdapter
     *   fails, such as a characteristic write request or subscribe attempt, or when a BLE connection is closed.
     *
     *   In most cases, this will prompt Weave to close the associated WeaveConnection and notify that platform that
     *   it has abandoned the underlying BLE connection.
     *
     *   NOTE: if the application explicitly closes a BLE connection with an associated WeaveConnection such that
     *   the BLE connection close will not generate an upcall to Weave, HandleConnectionError must be called with
     *   err = BLE_ERROR_APP_CLOSED_CONNECTION to prevent the leak of this WeaveConnection and its end point object. */
    void HandleConnectionError(BLE_CONNECTION_OBJECT connObj, BLE_ERROR err);

#if WEAVE_ENABLE_WOBLE_TEST
    BLEEndPoint * mTestBleEndPoint;
#endif

private:
    // Private data members:

    // UUID of Weave service characteristic used for central writes.
    static const WeaveBleUUID WEAVE_BLE_CHAR_1_ID;
    // UUID of Weave service characteristic used for peripheral indications.
    static const WeaveBleUUID WEAVE_BLE_CHAR_2_ID;

    BlePlatformDelegate * mPlatformDelegate;
    BleApplicationDelegate * mApplicationDelegate;
    Weave::System::Layer * mSystemLayer;

private:
    // Private functions:
    void HandleDataReceived(BLE_CONNECTION_OBJECT connObj, PacketBuffer * pBuf);
    void HandleAckReceived(BLE_CONNECTION_OBJECT connObj);
    void DriveSending(void);
    BLE_ERROR HandleBleTransportConnectionInitiated(BLE_CONNECTION_OBJECT connObj, PacketBuffer * pBuf);

    static BleTransportProtocolVersion GetHighestSupportedProtocolVersion(const BleTransportCapabilitiesRequestMessage & reqMsg);
};

#if BLE_CONFIG_PROVIDE_OBSOLESCENT_INTERFACES
inline BLE_ERROR BleLayer::Init(BlePlatformDelegate * aPlatformDelegate, BleApplicationDelegate * aAppDelegate,
                                Inet::InetLayer * aInetLayer)
{
    return Init(aPlatformDelegate, aAppDelegate, aInetLayer->SystemLayer());
}
#endif // BLE_CONFIG_PROVIDE_OBSOLESCENT_INTERFACES

} /* namespace Ble */
} /* namespace nl */

#include "BLEEndPoint.h"

#endif /* BLELAYER_H_ */
