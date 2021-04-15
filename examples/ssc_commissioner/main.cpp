/*
 *   Copyright (c) 2020 Project CHIP Authors
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

#include "config/PersistentStorage.h"
#include <app/server/DataModelHandler.h>
#include <condition_variable>
#include <controller/CHIPDeviceController.h>
#include <inet/Inet.h>
#include <mutex>
#include <support/CHIPMem.h>
#include <support/logging/CHIPLogging.h>
#include <transport/PASESession.h>

using namespace chip;

std::condition_variable cvWaitingForResponse;
std::mutex cvWaitingForResponseMutex;
bool mWaitingForResponse{ false };
constexpr uint16_t kWaitDurationInSeconds = 12000;

void UpdateWaitForResponse(bool value)
{
    {
        std::lock_guard<std::mutex> lk(cvWaitingForResponseMutex);
        mWaitingForResponse = value;
    }
    cvWaitingForResponse.notify_all();
}

void WaitForResponse(uint16_t duration)
{
    std::chrono::seconds waitingForResponseTimeout(duration);
    std::unique_lock<std::mutex> lk(cvWaitingForResponseMutex);
    auto waitingUntil = std::chrono::system_clock::now() + waitingForResponseTimeout;
    if (!cvWaitingForResponse.wait_until(lk, waitingUntil, []() { return !mWaitingForResponse; }))
    {
        ChipLogError(chipTool, "No response from device");
    }
}

class PairDelegate : public chip::Controller::DevicePairingDelegate
{
    virtual void OnStatusUpdate(chip::RendezvousSessionDelegate::Status status)
    {
        switch (status)
        {
        case chip::RendezvousSessionDelegate::Status::SecurePairingSuccess:
            ChipLogProgress(chipTool, "Secure Pairing Success");
            break;
        case chip::RendezvousSessionDelegate::Status::SecurePairingFailed:
            ChipLogError(chipTool, "Secure Pairing Failed");
            break;
        case chip::RendezvousSessionDelegate::Status::NetworkProvisioningSuccess:
            ChipLogProgress(chipTool, "Network Provisioning Success");
            break;
        case chip::RendezvousSessionDelegate::Status::NetworkProvisioningFailed:
            ChipLogError(chipTool, "Network Provisioning Failed");
            break;
        }
    }

    virtual void OnNetworkCredentialsRequested(chip::RendezvousDeviceCredentialsDelegate * callback)
    {
        ChipLogProgress(chipTool, "OnNetworkCredentialsRequested");
        callback->SendNetworkCredentials("My_SSID", "myPassword");
    }

    virtual void OnOperationalCredentialsRequested(const char * csr, size_t csr_length,
                                                   RendezvousDeviceCredentialsDelegate * callback)
    {
        ChipLogProgress(chipTool, "OnOperationalCredentialsRequested");
    }

    virtual void OnPairingComplete(CHIP_ERROR error) { ChipLogProgress(chipTool, "Good news, paring was successfull"); }

    virtual void OnPairingDeleted(CHIP_ERROR error) { ChipLogProgress(chipTool, "Something went wrong"); }
};

// ================================================================================
// Main Code
// ================================================================================
int main(int argc, char * argv[])
{
    // InitDataModelHandler();

    CHIP_ERROR err                  = CHIP_NO_ERROR;
    chip::NodeId localId            = chip::kTestControllerNodeId;
    constexpr chip::NodeId remoteId = chip::kTestDeviceNodeId;
    PersistentStorage storage;
    uint16_t mDiscriminator = 3840;
    uint32_t mSetupPINCode  = 12345678;
    chip::Inet::IPAddress mIPAddr;
    chip::Controller::DeviceCommissioner mCommissioner;
    chip::Transport::PeerAddress mPeerAddr;
    PairDelegate mPairDelegate;
    bool openPairingWindow = true;
    char peerIpAddress[40] = "127.0.0.1";
    uint16_t peerPort      = 11097;

    chip::Logging::SetLogFilter(storage.GetLoggingLevel());

    // TODO: Add better system like getopt
    if (argc == 4)
    {
        mDiscriminator    = static_cast<uint16_t>(atoi(argv[1]));
        mSetupPINCode     = static_cast<uint32_t>(atoi(argv[2]));
        localId           = atoi(argv[3]);
        openPairingWindow = false;
    }
    else if (argc == 3)
    {
        strcpy(peerIpAddress, argv[1]);
        peerPort = static_cast<uint16_t>(atoi(argv[2]));
    }

    chip::Inet::IPAddress::FromString(peerIpAddress, mIPAddr);
    mPeerAddr.SetIPAddress(mIPAddr);
    mPeerAddr.SetPort(peerPort);
    mPeerAddr.SetTransportType(chip::Transport::Type::kUdp);
    ChipLogProgress(chipTool, "Connected to device at %s:%d", peerIpAddress, peerPort);
    chip::RendezvousParameters params =
        chip::RendezvousParameters().SetSetupPINCode(mSetupPINCode).SetDiscriminator(mDiscriminator).SetPeerAddress(mPeerAddr);

    err = chip::Platform::MemoryInit();
    VerifyOrExit(err == CHIP_NO_ERROR, ChipLogError(Controller, "Init Memory failure: %s", chip::ErrorStr(err)));

    err = storage.Init();
    VerifyOrExit(err == CHIP_NO_ERROR, ChipLogError(Controller, "Init Storage failure: %s", chip::ErrorStr(err)));

    mCommissioner.SetUdpListenPort(12300);
    err = mCommissioner.Init(localId, &storage, &mPairDelegate);
    VerifyOrExit(err == CHIP_NO_ERROR, ChipLogError(Controller, "Init failure! Commissioner: %s", chip::ErrorStr(err)));

    err = mCommissioner.ServiceEvents();
    VerifyOrExit(err == CHIP_NO_ERROR, ChipLogError(Controller, "Init failure! Run Loop: %s", chip::ErrorStr(err)));

    mCommissioner.SetOpenPairingWindow(openPairingWindow);

    UpdateWaitForResponse(true);
    err = mCommissioner.PairDevice(remoteId, params);
    WaitForResponse(kWaitDurationInSeconds);

exit:
    mCommissioner.ServiceEventSignal();
    mCommissioner.Shutdown();
}
