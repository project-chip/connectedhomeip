/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
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

#ifndef __TRANSPORT_RENDEZVOUSPARAMETERS_H__
#define __TRANSPORT_RENDEZVOUSPARAMETERS_H__

#include <transport/raw/Base.h>

#if CONFIG_NETWORK_LAYER_BLE
#include <ble/Ble.h>
#endif // CONFIG_NETWORK_LAYER_BLE

#include <support/logging/CHIPLogging.h>

namespace chip {

class RendezvousParameters
{
public:
    RendezvousParameters() = default;

    bool IsController() const { return HasDiscriminator() || HasConnectionObject(); }

    bool HasSetupPINCode() const { return mSetupPINCode != 0; }
    uint32_t GetSetupPINCode() const { return mSetupPINCode; }
    RendezvousParameters & SetSetupPINCode(uint32_t setupPINCode)
    {
        mSetupPINCode = setupPINCode;
        return *this;
    }

    bool HasDiscriminator() const { return mDiscriminator != 0; }
    uint16_t GetDiscriminator() const { return mDiscriminator; }
    RendezvousParameters & SetDiscriminator(uint16_t discriminator)
    {
        mDiscriminator = discriminator;
        return *this;
    }

    bool HasLocalNodeId() const { return mLocalNodeId.HasValue(); }
    const Optional<NodeId> GetLocalNodeId() const { return mLocalNodeId; }
    RendezvousParameters & SetLocalNodeId(NodeId nodeId)
    {
        mLocalNodeId.SetValue(nodeId);
        return *this;
    }

#if CONFIG_NETWORK_LAYER_BLE
    bool HasBleLayer() const { return mBleLayer != nullptr; }
    Ble::BleLayer * GetBleLayer() const { return mBleLayer; }
    RendezvousParameters & SetBleLayer(Ble::BleLayer * value)
    {
        mBleLayer = value;
        return *this;
    }

    bool HasConnectionObject() const { return mConnectionObject != 0; }
    BLE_CONNECTION_OBJECT GetConnectionObject() const { return mConnectionObject; }
    RendezvousParameters & SetConnectionObject(BLE_CONNECTION_OBJECT connObj)
    {
        mConnectionObject = connObj;
        return *this;
    }
#else
    bool HasConnectionObject() const { return false; }
#endif // CONFIG_NETWORK_LAYER_BLE

private:
    Optional<NodeId> mLocalNodeId; ///< the local node id
    uint32_t mSetupPINCode  = 0;   ///< the target peripheral setup PIN Code
    uint16_t mDiscriminator = 0;   ///< the target peripheral discriminator

#if CONFIG_NETWORK_LAYER_BLE
    Ble::BleLayer * mBleLayer               = nullptr;
    BLE_CONNECTION_OBJECT mConnectionObject = 0;
#endif // CONFIG_NETWORK_LAYER_BLE
};

} // namespace chip

#endif // __TRANSPORT_RENDEZVOUSPARAMETERS_H__
