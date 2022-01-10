/*
 *    Copyright (c) 2020-2021 Project CHIP Authors
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

#include <commissioner/Discoverer.h>

namespace chip {
namespace Commissioner {
namespace CommissionableNodeDiscoverer {

#if CONFIG_NETWORK_LAYER_BLE
BleDiscoverer::BleDiscoverer(Controller::DeviceControllerSystemState & systemState, Delegate ** delegate) :
    Joinable(delegate), mSystemState(systemState)
{
    mSystemState.Retain();
}

BleDiscoverer::~BleDiscoverer()
{
    mSystemState.Release();
}

void BleDiscoverer::Shutdown()
{
    if (InProgress())
    {
        VerifyOrReturn(mSystemState.BleLayer()->CancelBleIncompleteConnection() == CHIP_NO_ERROR);
        ReleaseShutdownToken();
    }
}

CHIP_ERROR BleDiscoverer::StartBleDiscovery(SetupPayload & payload, Platform::SharedPtr<ShutdownToken> token)
{
    VerifyOrReturnError(token.get() != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    ReturnErrorOnFailure(mSystemState.BleLayer()->NewBleConnectionByDiscriminator(payload.discriminator, this,
                                                                                  OnBleDiscoverySuccess, OnBleDiscoveryError));
    RetainShutdownToken(token);
    return CHIP_NO_ERROR;
}

void BleDiscoverer::TallyBleDiscovery(BLE_CONNECTION_OBJECT connection)
{
    mBleConnection.SetValue(connection);
    (*mDelegate)->OnDiscovery();
    ReleaseShutdownToken();
}

void BleDiscoverer::TallyBleFailure()
{
    (*mDelegate)->OnDiscovery();
    ReleaseShutdownToken();
}

CHIP_ERROR BleDiscoverer::GetNextBleCandidate(Commissionee & commissionee)
{
    VerifyOrReturnError(mBleConnection.HasValue(), CHIP_ERROR_NOT_FOUND);
    commissionee.CloseBle(); // close if connection currently exists; failure is a no-op
    commissionee.mBleConnection = mBleConnection;
    commissionee.mCommissionableNodeAddress.SetValue(Transport::PeerAddress::BLE());
    commissionee.mMrpConfig.ClearValue();
    ReturnErrorOnFailure(commissionee.OpenBle());
    mBleConnection.ClearValue();
    ChipLogProgress(Controller, "Commissionable node candidate identified via BLE");
    return CHIP_NO_ERROR;
}

void BleDiscoverer::OnBleDiscoverySuccess(void * context, BLE_CONNECTION_OBJECT connection)
{
    BleDiscoverer * instance = static_cast<BleDiscoverer *>(context);
    instance->TallyBleDiscovery(connection);
}

void BleDiscoverer::OnBleDiscoveryError(void * context, CHIP_ERROR err)
{
    ChipLogError(Controller, "BLE Commissionable Node Discovery failed: %s", ErrorStr(err));
    BleDiscoverer * instance = static_cast<BleDiscoverer *>(context);
    instance->TallyBleFailure();
}

#endif // CONFIG_NETWORK_LAYER_BLE

} // namespace CommissionableNodeDiscoverer
} // namespace Commissioner
} // namespace chip
