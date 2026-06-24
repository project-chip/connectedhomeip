/*
 *
 *    Copyright (c) 2025 Project CHIP Authors
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
#pragma once

#include <controller/SetUpCodePairer.h>

namespace chip {
namespace Testing {

// Provides access to private members of SetUpCodePairer for testing.
class SetUpCodePairerTestAccess
{
public:
    SetUpCodePairerTestAccess() = delete;
    explicit SetUpCodePairerTestAccess(Controller::SetUpCodePairer * pairer) : mPairer(pairer) {}

    // Re-export private transport type indices for use in tests.
    static constexpr int kBLETransport     = Controller::SetUpCodePairer::kBLETransport;
    static constexpr int kIPTransport      = Controller::SetUpCodePairer::kIPTransport;
    static constexpr int kWiFiPAFTransport = Controller::SetUpCodePairer::kWiFiPAFTransport;

    bool GetWaitingForDiscovery(int transport) const { return mPairer->mWaitingForDiscovery[transport]; }
    void SetWaitingForDiscovery(int transport, bool val) { mPairer->mWaitingForDiscovery[transport] = val; }

    bool GetWaitingForPASE() const { return mPairer->mWaitingForPASE; }
    void SetWaitingForPASE(bool val) { mPairer->mWaitingForPASE = val; }

    NodeId GetRemoteId() const { return mPairer->mRemoteId; }
    void SetRemoteId(NodeId id) { mPairer->mRemoteId = id; }

    CHIP_ERROR GetLastPASEError() const { return mPairer->mLastPASEError; }

    bool HasCurrentPASEParameters() const { return mPairer->mCurrentPASEParameters.HasValue(); }
    void SetCurrentPASEParameters(const Controller::SetUpCodePairerParameters & params)
    {
        mPairer->mCurrentPASEParameters.SetValue(params);
    }

    void ExpectPASEEstablishment() { mPairer->ExpectPASEEstablishment(); }

    void CallOnPairingComplete(CHIP_ERROR error) { mPairer->OnPairingComplete(error, std::nullopt, std::nullopt); }

    void FireTimeoutCallback() { Controller::SetUpCodePairer::OnDeviceDiscoveredTimeoutCallback(nullptr, mPairer); }

private:
    Controller::SetUpCodePairer * mPairer = nullptr;
};

} // namespace Testing
} // namespace chip
