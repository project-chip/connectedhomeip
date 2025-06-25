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
class InteractionModelEngine;
/**
 * @brief RefreshKeySender contains all the data and methods needed for key refresh and re-registration of an ICD client.
 */
class RefreshKeySender
{
public:
    typedef Crypto::SensitiveDataBuffer<Crypto::kAES_CCM128_Key_Length> RefreshKeyBuffer;

    RefreshKeySender(CheckInDelegate * checkInDelegate, const ICDClientInfo & icdClientInfo, ICDClientStorage * icdClientStorage,
                     InteractionModelEngine * engine, const RefreshKeyBuffer & refreshKeyBuffer);

    /**
     * @brief Sets up a CASE session to the peer for re-registering a client with the peer when a key refresh is required to avoid
     * ICD counter rollover. Returns error if we did not even manage to kick off a CASE attempt.
     */
    CHIP_ERROR EstablishSessionToPeer();

    /**
     * @brief Used to retrieve ICDClientInfo from RefreshKeySender.
     *
     * @return ICDClientInfo - ICDClientInfo object representing the state associated with the
                               node that requested a key refresh.
     */
    const ICDClientInfo & GetICDClientInfo();

private:
    // CASE session callbacks
    /**
     * @brief Callback received on successfully establishing a CASE session in order to re-register the client with the peer node
     * using a new key to avoid counter rollover problems.
     *
     * @param[in] context       - context of the client establishing the CASE session
     * @param[in] exchangeMgr   - exchange manager to use for the re-registration
     * @param[in] sessionHandle - session handle to use for the re-registration
     */
    static void HandleDeviceConnected(void * context, Messaging::ExchangeManager & exchangeMgr,
                                      const SessionHandle & sessionHandle);
    /**
     * @brief Callback received on failure to establish a CASE session in order to re-register the client with the peer node using a
     * new key to avoid counter rollover problems.
     *
     * @param[in] context - context of the client establishing the CASE session
     * @param[in] peerId  - Scoped Node ID of the peer node
     * @param[in] err     - failure reason
     */
    static void HandleDeviceConnectionFailure(void * context, const ScopedNodeId & peerId, CHIP_ERROR err);

    /**
     * @brief Used to send a re-registration command to the peer using a new key.
     *
     * @param[in] exchangeMgr   - exchange manager to use for the re-registration
     * @param[in] sessionHandle - session handle to use for the re-registration
     */
    CHIP_ERROR RegisterClientWithNewKey(Messaging::ExchangeManager & exchangeMgr, const SessionHandle & sessionHandle);

    CheckInDelegate * mpCheckInDelegate = nullptr;
    ICDClientInfo mICDClientInfo;
    ICDClientStorage * mpICDClientStorage = nullptr;
    InteractionModelEngine * mpImEngine   = nullptr;
    RefreshKeyBuffer mNewKey;
    Callback::Callback<OnDeviceConnected> mOnConnectedCallback;
    Callback::Callback<OnDeviceConnectionFailure> mOnConnectionFailureCallback;
};
} // namespace app
} // namespace chip
