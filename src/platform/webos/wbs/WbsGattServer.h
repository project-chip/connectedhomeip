/* @@@LICENSE
 *
 * Copyright (c) 2025 LG Electronics, Inc.
 *
 * Confidential computer software. Valid license from LG required for
 * possession, use or copying. Consistent with FAR 12.211 and 12.212,
 * Commercial Computer Software, Computer Software Documentation, and
 * Technical Data for Commercial Items are licensed to the U.S. Government
 * under vendor's standard commercial license.
 *
 * LICENSE@@@
 */

#pragma once

#include <string>

#include <lib/core/CHIPError.h>
#include <luna-service2++/handle.hpp>
#include <system/SystemClock.h>
#include <system/SystemLayer.h>

namespace chip {
namespace DeviceLayer {
namespace Internal {

class WbsConnection;

/// Exposes the CHIPoBLE GATT service (RX write / TX indicate characteristics) as a local GATT
/// server via webOS's bluetooth2 LS2 service (gatt/openServer, gatt/addService,
/// gatt/removeService, gatt/closeServer), and implements the server-side data path on top of it:
///  - RX writes from a remote commissioner are delivered via a persistent
///    gatt/monitorCharacteristic(serverId, RX, subscribe:true) subscription.
///  - A remote commissioner subscribing to (or unsubscribing from) indications on TX - i.e. a
///    CCCD write - has no push notification in webOS's bluetooth2 LS2 API (unlike a remote write
///    to RX, this event is only ever delivered to in-process observers inside the bluetooth2
///    service, never to an LS2 client - see the investigation notes for WbsGattServer.cpp). It is
///    instead detected by short-interval polling of gatt/readDescriptorValue(serverId, TX, CCCD).
///  - Outgoing data is pushed via gatt/writeCharacteristicValue(serverId, TX, value), through
///    WbsConnection::SendIndication() on the WbsConnection returned by GetPeerConnection() (see
///    WbsConnection::ConfigureAsServerRole()).
///
/// Only a single inbound (peripheral-role) connection is modeled at a time, matching
/// BLEManagerImpl::kMaxConnections.
class WbsGattServer
{
public:
    WbsGattServer()  = default;
    ~WbsGattServer() = default;

    /// Open a local GATT server, register the CHIPoBLE service, and start the RX monitor
    /// subscription and TX CCCD poll timer described above.
    ///
    /// Must be called while in the Matter context (from the Matter event loop, or while holding
    /// the Matter stack lock).
    CHIP_ERROR Init();

    /// Stop the RX monitor subscription and TX CCCD poll timer, unregister the CHIPoBLE service,
    /// and close the local GATT server.
    void Shutdown();

    bool IsOpen() const { return mIsOpen; }

    /// The BLE_CONNECTION_OBJECT (see BlePlatformConfig.h) representing the inbound connection
    /// accepted by this GATT server, valid for the lifetime of the open server (i.e. not
    /// necessarily indicating a central is currently connected - webOS's bluetooth2 LS2 API gives
    /// us no push notification of physical connect/disconnect for the peripheral role either, only
    /// the RX-write and TX-CCCD signals described above).
    WbsConnection * GetPeerConnection() const { return mPeerConnection; }

private:
    static CHIP_ERROR InitImpl(WbsGattServer * self);
    static CHIP_ERROR ShutdownImpl(WbsGattServer * self);

    static bool OnRxCharacteristicChanged(LSHandle * sh, LSMessage * message, void * ctx);

    static void HandleCccdPollTimer(chip::System::Layer * layer, void * appState);
    static CHIP_ERROR PollCccdImpl(WbsGattServer * self);

    static constexpr System::Clock::Timeout kCccdPollInterval = System::Clock::Milliseconds32(200);

    bool mIsOpen = false;
    std::string mServerId;

    WbsConnection * mPeerConnection = nullptr;
    LSMessageToken mRxMonitorToken  = LSMESSAGE_TOKEN_INVALID;
    bool mCccdIndicateEnabled       = false;
};

} // namespace Internal
} // namespace DeviceLayer
} // namespace chip
