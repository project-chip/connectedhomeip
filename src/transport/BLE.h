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

#include <ble/BleConfig.h>

#include <utility>

#include <ble/BleError.h>
#include <ble/BleLayer.h>
#include <core/CHIPCore.h>
#include <support/DLLUtil.h>
#include <transport/RendezvousParameters.h>
#include <transport/RendezvousSessionDelegate.h>
#include <transport/raw/Base.h>

namespace chip {
namespace Transport {

/** Implements a transport using BLE.
 *
 *  TODO: BLE transport currently does NOT receive messages as defined
 *        in the Transport::Base (i.e. no header is parsed and processed) and
 *        instead received packets are sent raw via BLE Handler callbacks.
 */
class DLL_EXPORT BLE : public Base
{
    /**
     *  The State of the BLE connection
     *
     */
    enum class State
    {
        kNotReady    = 0, /**< State before initialization. */
        kInitialized = 1, /**< State after class is connected and ready. */
    };

public:
    ~BLE() override;

    /**
     * Initialize a BLE transport to a given peripheral or a given device name.
     *
     * @param delegate      the delegate that will receive BLE events
     * @param params        BLE configuration parameters for this transport
     */
    CHIP_ERROR Init(RendezvousSessionDelegate * delegate, const RendezvousParameters & params);

    CHIP_ERROR SendMessage(const PacketHeader & header, const Transport::PeerAddress & address,
                           System::PacketBufferHandle msgBuf) override;

    bool CanSendToPeer(const Transport::PeerAddress & address) override
    {
        return (mState == State::kInitialized) && (address.GetTransportType() == Type::kBle);
    }

    CHIP_ERROR SetEndPoint(Ble::BLEEndPoint * endPoint);

private:
    CHIP_ERROR InitInternal(BLE_CONNECTION_OBJECT connObj);
    CHIP_ERROR DelegateConnection(uint16_t connDiscriminator);
    void SetupEvents(Ble::BLEEndPoint * endPoint);

    /**
     * @brief
     *  Called when the underlying BleLayer receive a new
     *  remote connection.
     *
     * @param endPoint The newly opened BLEEndPoint
     */
    static void OnNewConnection(Ble::BLEEndPoint * endPoint);

    // Those functions are BLEConnectionDelegate callbacks used when the connection
    // parameters used a name instead of a BLE_CONNECTION_OBJECT.
    static void OnBleConnectionComplete(void * appState, BLE_CONNECTION_OBJECT connObj);
    static void OnBleConnectionError(void * appState, BLE_ERROR err);

    // Those functions are BLEEndPoint callbacks
    static void OnBleEndPointReceive(Ble::BLEEndPoint * endPoint, System::PacketBufferHandle buffer);
    static void OnBleEndPointConnectionComplete(Ble::BLEEndPoint * endPoint, BLE_ERROR err);
    static void OnBleEndPointConnectionClosed(Ble::BLEEndPoint * endPoint, BLE_ERROR err);

    Ble::BleLayer * mBleLayer             = nullptr;          ///< Associated ble layer
    State mState                          = State::kNotReady; ///< State of the BLE transport
    Ble::BLEEndPoint * mBleEndPoint       = nullptr;          ///< BLE endpoint used by transport
    RendezvousSessionDelegate * mDelegate = nullptr;          ///< BLE events from transport
};

} // namespace Transport
} // namespace chip
