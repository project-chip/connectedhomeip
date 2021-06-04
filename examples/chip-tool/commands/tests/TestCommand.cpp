/*
 *   Copyright (c) 2021 Project CHIP Authors
 *   All rights reserved.
 *
 *   Licensed under the Apache License, Version 2.0 (the "License");
 *   you may not use this file except in compliance with the License.
 *   You may obtain a copy of the License at
 *
 *       http://www.apache.org/licenses/LICENSE-2.0
 *
 *   Unless required by applicable law or agreed to in writing, software
 *   distributed under the License is distributed on an "AS IS" BASIS,
 *   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *   See the License for the specific language governing permissions and
 *   limitations under the License.
 *
 */

#include "TestCommand.h"

#define DEFAULT_DEVICE_ID       1
#define DEFAULT_DESCRIMINATOR   3840
#define DEFAULT_SETUP_PIN_CODE  20202021
#define DEFAULT_REMOTE_PORT     11097
#define DEFAULT_TEST_ADDRESS    "::1"

constexpr uint16_t kWaitDurationInSeconds = 30;

CHIP_ERROR TestCommand::Run(PersistentStorage & storage, NodeId localId, NodeId remoteId)
{
    ReturnErrorOnFailure(mOpCredsIssuer.Initialize(storage));

    chip::Controller::CommissionerInitParams params;

    params.storageDelegate                = &storage;
    params.operationalCredentialsDelegate = &mOpCredsIssuer;

    ReturnErrorOnFailure(mCommissioner.SetUdpListenPort(storage.GetListenPort()));
    ReturnErrorOnFailure(mCommissioner.Init(localId, params));
    ReturnErrorOnFailure(mCommissioner.ServiceEvents());
    

    CHIP_ERROR result = mCommissioner.GetDevice(remoteId, &mDevice);

    if ( result != CHIP_NO_ERROR ) {
        chip::Inet::IPAddress addr;
        chip::Inet::IPAddress::FromString(DEFAULT_TEST_ADDRESS, addr);
        uint16_t port = DEFAULT_REMOTE_PORT;
        uint32_t setupPINCode = DEFAULT_SETUP_PIN_CODE;
        uint16_t discriminator = DEFAULT_DESCRIMINATOR;
        chip::NodeId deviceID = DEFAULT_DEVICE_ID;
        chip::Transport::PeerAddress peerAddress = chip::Transport::PeerAddress::UDP(addr, port);

        chip::RendezvousParameters pairingParams = chip::RendezvousParameters()
                                                    .SetSetupPINCode(setupPINCode)
                                                    .SetDiscriminator(discriminator)
                                                    .SetPeerAddress(peerAddress);

        ReturnErrorOnFailure(mCommissioner.PairDevice(deviceID, pairingParams));
        ChipLogDetail(chipTool, "Started pairing");
    }

    ReturnErrorOnFailure(NextTest());

    UpdateWaitForResponse(true);
    WaitForResponse(kWaitDurationInSeconds);

    mCommissioner.ServiceEventSignal();
    mCommissioner.Shutdown();

    VerifyOrReturnError(GetCommandExitStatus(), CHIP_ERROR_INTERNAL);

    return CHIP_NO_ERROR;
}
