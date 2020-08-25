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

#include "BluetoothWidget.h"

#include <platform/CHIPDeviceLayer.h>
#include <transport/SecurePairingSession.h>

using namespace ::chip;

class RendezvousSession : public SecurePairingSessionDelegate
{
public:
    RendezvousSession(BluetoothWidget * virtualLed, uint32_t setUpPINCode, NodeId myNodeId);
    CHIP_ERROR Send(const char * msg);

private:
    static void HandleConnectionOpened(Ble::BLEEndPoint * endPoint);
    static void HandleConnectionClosed(Ble::BLEEndPoint * endPoint, BLE_ERROR err);
    static void HandleMessageReceived(Ble::BLEEndPoint * endPoint, System::PacketBuffer * buffer);

    virtual CHIP_ERROR OnNewMessageForPeer(System::PacketBuffer * msgBuf);
    virtual void OnPairingError(CHIP_ERROR error);
    virtual void OnPairingComplete(Optional<NodeId> peerNodeId, uint16_t peerKeyId, uint16_t localKeyId);

    static BluetoothWidget * mVirtualLed;
    static Ble::BLEEndPoint * mEndPoint;

    static SecurePairingSession mPairing;
    static bool mPairingInProgress;

    bool mPaired           = false;
    uint32_t mSetUpPINCode = 0;
    NodeId mNodeId;
};
