/*
 *
 *    Copyright (c) 2024 Project CHIP Authors
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

#include "ICDClientInfo.h"
#include <app/CommandSender.h>
#include <app/OperationalSessionSetup.h>

#include <crypto/CHIPCryptoPAL.h>
#include <lib/core/CHIPConfig.h>
#include <lib/core/DataModelTypes.h>
#include <lib/core/ScopedNodeId.h>
#include <lib/support/CodeUtils.h>
#include <stddef.h>

namespace chip {
namespace app {

typedef Crypto::SensitiveDataBuffer<Crypto::kAES_CCM128_Key_Length> RefreshKeyBuffer;

class CheckInDelegate;

class ICDRefreshKeyInfo
{
public:
    ICDRefreshKeyInfo(CheckInDelegate * apCheckInDelegate, ICDClientInfo aICDClientInfo);
    // ICDRefreshKeyInfo(const ICDRefreshKeyInfo & other);
    // ICDRefreshKeyInfo & operator=(const ICDRefreshKeyInfo & other);

    class RegisterCommandSenderCallback : public CommandSender::Callback
    {
    public:
        void OnResponse(chip::app::CommandSender * apCommandSender, const chip::app::ConcreteCommandPath & aPath,
                        const chip::app::StatusIB & aStatus, chip::TLV::TLVReader * aData) override
        {
            // TODO
        }
        void OnError(const chip::app::CommandSender * apCommandSender, CHIP_ERROR aError) override { mError = aError; }
        void OnDone(chip::app::CommandSender * apCommandSender) override {}

        CHIP_ERROR mError = CHIP_NO_ERROR;
    };

    void SetCheckInDelegate(CheckInDelegate * apCheckInDelegate) { mpCheckInDelegate = apCheckInDelegate; }
    void SetICDClientInfo(ICDClientInfo icdClientInfo) { mICDClientInfo = icdClientInfo; }
    //   RefreshKeyBuffer GetNewKey() { return mNewKey; }
    /**
     * @brief Callback received on successfully establishing a CASE session in order to re-register the client with the peer node
     * using a new key to avoid counter rollover problems.
     *
     * @param[in] context context of the client establishing the CASE session
     * @param[in] exchangeMgr exchange manager to use for the re-registration
     * @param[in] sessionHandle session handle to use for the re-registration
     */
    static void HandleDeviceConnected(void * context, Messaging::ExchangeManager & exchangeMgr,
                                      const SessionHandle & sessionHandle);
    /**
     * @brief Callback received on failure to establish a CASE session in order to re-register the client with the peer node using a
     * new key to avoid counter rollover problems.
     *
     * @param[in] context context of the client establishing the CASE session
     * @param[in] peerId Scoped Node ID of the peer node
     * @param[in] err failure reason
     */
    static void HandleDeviceConnectionFailure(void * context, const ScopedNodeId & peerId, CHIP_ERROR err);

    /**
     * @brief Used by the application to set a new key to avoid counter rollover problems.
     *
     * @param[in] clientInfo clientInfo object
     * @param[in] keyData New key data to use to re-register the client with the server
     * @param[in] exchangeMgr exchange manager to use for the re-registration
     * @param[in] sessionHandle session handle to use for the re-registration
     */
    CHIP_ERROR RegisterClientWithNewKey(Messaging::ExchangeManager & exchangeMgr, const SessionHandle & sessionHandle);

    /**
     * @brief Sets up a CASE session to the peer for re-registering a client with the peer when a key refresh is required to avoid
     * ICD counter rollover. Returns error if we did not even manage to kick off a CASE attempt.
     */
    void EstablishSessionToPeer();
    RefreshKeyBuffer mNewKey;

private:
    ICDClientInfo mICDClientInfo;
    CheckInDelegate * mpCheckInDelegate = nullptr;
    RegisterCommandSenderCallback mCommandSenderDelegate;
    app::CommandSender mRegisterCommandSender;
    chip::Callback::Callback<OnDeviceConnected> mOnConnectedCallback;
    chip::Callback::Callback<OnDeviceConnectionFailure> mOnConnectionFailureCallback;
};

} // namespace app
} // namespace chip
