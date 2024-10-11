/*
 *
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
 *      This file defines the CHIP Connection object that maintains a BLE connection.
 *
 */

#pragma once

#include <utility>

#include <ble/Ble.h>
#include <lib/core/CHIPCore.h>
#include <lib/support/DLLUtil.h>
#include <system/SystemPacketBuffer.h>
#include <transport/raw/Base.h>

namespace chip {
namespace Transport {

/** Defines listening parameters for setting up a BLE transport */
class BleListenParameters
{
public:
    explicit BleListenParameters(Ble::BleLayer * layer) : mLayer(layer) {}
    BleListenParameters(const BleListenParameters &) = default;
    BleListenParameters(BleListenParameters &&)      = default;

    Ble::BleLayer * GetBleLayer() const { return mLayer; }

    /**
     * PreserveExistingBleLayerTransport controls whether the BleBase transport
     * initialized with these parameters should update the global BleLayer transport
     * if it is already set.
     *
     * This is relevant when there is more than one TransportMgr,
     * for example, when a device is both a commissioner (has a CHIPDeviceController)
     * and a commissionee (has a Server) since each TransportMgr will have a BleBase
     * which can override the global BleLayer transport to point to itself.
     *
     * The default value is true - don't override the global BleLayer transport if it is
     * already set. In other words, the first BleBase to initialize (eg. Server) will be
     * the active BleBase transport, and the second BleBase to initialize (eg. CHIPDeviceController)
     * will need to call BleBase.SetBleLayerTransportToSelf if it needs to commission using BLE.
     *
     * Call SetPreserveExistingBleLayerTransport(false) to set the global
     * BleLayer transport to the BleBase created with these parameters, even if it is already
     * set to another BleBase.
     *
     * Use the BleBase.IsBleLayerTransportSetToSelf() and BleBase.SetBleLayerTransportToSelf
     * methods to toggle between BleBase transports when there is more than one.
     */
    bool PreserveExistingBleLayerTransport() const { return mPreserveExistingBleLayerTransport; }
    BleListenParameters & SetPreserveExistingBleLayerTransport(bool preserveExistingBleLayerTransport)
    {
        mPreserveExistingBleLayerTransport = preserveExistingBleLayerTransport;

        return *this;
    }

private:
    Ble::BleLayer * mLayer;
    bool mPreserveExistingBleLayerTransport = true;
};

/** Implements a transport using BLE.
 *
 *  TODO: BLE transport currently only allow one BLE connection, neet to clearify if we should support multiple BLE connections.
 */
class DLL_EXPORT BLEBase : public Base, public Ble::BleLayerDelegate
{
    /**
     *  The State of the BLE connection
     *
     */
    enum class State
    {
        kNotReady    = 0, /**< State before initialization. */
        kInitialized = 1, /**< State after class is connected and ready. */
        kConnected   = 2, /**< Endpoint connected. */
    };

public:
    BLEBase(System::PacketBufferHandle * packetBuffers, size_t packetBuffersSize) :
        mPendingPackets(packetBuffers), mPendingPacketsSize(packetBuffersSize)
    {}
    ~BLEBase() override;

    /**
     * Initialize a BLE transport to a given peripheral or a given device name.
     *
     * @param param        BLE configuration parameters for this transport
     */
    CHIP_ERROR Init(const BleListenParameters & param);

    CHIP_ERROR SendMessage(const Transport::PeerAddress & address, System::PacketBufferHandle && msgBuf) override;

    bool CanSendToPeer(const Transport::PeerAddress & address) override
    {
        return (mState != State::kNotReady) && (address.GetTransportType() == Type::kBle);
    }

    CHIP_ERROR SetEndPoint(Ble::BLEEndPoint * endPoint) override;

    /**
     * Change BLE transport to this
     *
     * This is relevant when there is more than one TransportMgr,
     * for example, when a device is both a commissioner (has a CHIPDeviceController)
     * and a commissionee (has a Server) since each TransportMgr will set
     * the global BleLayer transport to point to itself.
     *
     * In this scenario, the device will need the ability to toggle between a
     * BleLayer transport for commissioner functionality and one for commissionee functionality.
     */
    void SetBleLayerTransportToSelf() { mBleLayer->mBleTransport = this; }
    bool IsBleLayerTransportSetToSelf() { return mBleLayer->mBleTransport == this; }

private:
    void ClearState();

    /**
     * Sends the specified message once a connection has been established.
     *
     * @param msg - what buffer to send once a connection has been established.
     *
     * Ownership of msg is taken over and will be freed at some unspecified time
     * in the future (once connection succeeds/fails).
     */
    CHIP_ERROR SendAfterConnect(System::PacketBufferHandle && msg);

    // Those functions are BLEConnectionDelegate callbacks used when the connection
    // parameters used a name instead of a BLE_CONNECTION_OBJECT.
    void OnBleConnectionComplete(Ble::BLEEndPoint * endPoint) override;
    void OnBleConnectionError(CHIP_ERROR err) override;

    void ClearPendingPackets();

    // Those functions are BLEEndPoint callbacks
    void OnEndPointMessageReceived(Ble::BLEEndPoint * endPoint, System::PacketBufferHandle && buffer) override;
    void OnEndPointConnectComplete(Ble::BLEEndPoint * endPoint, CHIP_ERROR err) override;
    void OnEndPointConnectionClosed(Ble::BLEEndPoint * endPoint, CHIP_ERROR err) override;

    Ble::BleLayer * mBleLayer       = nullptr;          ///< Associated ble layer
    State mState                    = State::kNotReady; ///< State of the BLE transport
    Ble::BLEEndPoint * mBleEndPoint = nullptr;          ///< BLE endpoint used by transport

    // Data to be sent when connections succeed
    System::PacketBufferHandle * mPendingPackets;
    const size_t mPendingPacketsSize;
};

template <size_t kPendingPacketSize>
class BLE : public BLEBase
{
public:
    BLE() : BLEBase(mPendingPackets, kPendingPacketSize) {}

private:
    System::PacketBufferHandle mPendingPackets[kPendingPacketSize];
};

} // namespace Transport
} // namespace chip
