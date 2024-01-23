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
#include "ICDClientStorage.h"
#include <app-common/zap-generated/cluster-objects.h>
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

class CheckInDelegate;
class ICDRefreshKeyInfo;

/**
 * @brief This class comprises Command sender callbacks for the registration command. The application can inherit this class and
 *        provide their own implementation
 */
class RegisterCommandSenderDelegate : public CommandSender::Callback
{
public:
    // CommandSender callbacks
    void OnResponse(CommandSender * apCommandSender, const ConcreteCommandPath & aPath, const StatusIB & aStatus,
                    chip::TLV::TLVReader * aData) override;
    void OnError(const CommandSender * apCommandSender, CHIP_ERROR aError) override;
    void OnDone(CommandSender * apCommandSender) override;
    /**
     * @brief Callback used to indicate failures before successfully sending out a re-register command.
     *
     * @param[in] aError Failure reason
     * */
    void OnFailure(CHIP_ERROR aError);

    void AdoptICDRefreshKeyInfo(Platform::UniquePtr<ICDRefreshKeyInfo> apICDRefreshKeyInfo)
    {
        mpICDRefreshKeyInfo = std::move(apICDRefreshKeyInfo);
    }

    virtual ~RegisterCommandSenderDelegate() { mpICDRefreshKeyInfo = nullptr; }

private:
    CHIP_ERROR mError                                          = CHIP_NO_ERROR;
    Platform::UniquePtr<ICDRefreshKeyInfo> mpICDRefreshKeyInfo = nullptr;
};

/**
 * @brief ICDRefreshKeyInfo contains all the data and methods needed for key refresh and re-registration of an ICD client.
 */
class ICDRefreshKeyInfo
{
public:
    typedef Crypto::SensitiveDataBuffer<Crypto::kAES_CCM128_Key_Length> RefreshKeyBuffer;

    ICDRefreshKeyInfo(CheckInDelegate * apCheckInDelegate, ICDClientInfo * aICDClientInfo, ICDClientStorage * aICDClientStorage);

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
    CHIP_ERROR EstablishSessionToPeer();

    /**
     * @brief Getter for ICDClientInfo pointer
     */
    ICDClientInfo * GetICDClientInfo(void) { return mpICDClientInfo; }

    /**
     * @brief Getter for ICDClientStorage pointer
     */
    ICDClientStorage * GetICDClientStorage(void) { return mpICDClientStorage; }

    /**
     * @brief Getter for CheckInDelegate pointer
     */
    CheckInDelegate * GetCheckInDelegate(void) { return mpCheckInDelegate; }

    /**
     * @brief Getter for Command Sender delegate
     */
    RegisterCommandSenderDelegate * GetCommandSenderDelegate(void) { return mpCommandSenderDelegate; }

    /**
     * @brief Setter for Command Sender delegate
     */
    void SetCommandSenderDelegate(RegisterCommandSenderDelegate * apCommandSenderDelegate)
    {
        mpCommandSenderDelegate = apCommandSenderDelegate;
    }

    RefreshKeyBuffer mNewKey;

private:
    // CASE session callbacks
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

    ICDClientInfo * mpICDClientInfo       = nullptr;
    ICDClientStorage * mpICDClientStorage = nullptr;
    CheckInDelegate * mpCheckInDelegate   = nullptr;
    chip::Optional<CommandSender> mRegisterCommandSender;
    RegisterCommandSenderDelegate * mpCommandSenderDelegate = nullptr;
    chip::Callback::Callback<OnDeviceConnected> mOnConnectedCallback;
    chip::Callback::Callback<OnDeviceConnectionFailure> mOnConnectionFailureCallback;
    CHIP_ERROR mError = CHIP_NO_ERROR;
};
} // namespace app
} // namespace chip
