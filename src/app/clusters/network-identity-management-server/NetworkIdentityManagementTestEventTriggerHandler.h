/*
 *
 *    Copyright (c) 2026 Project CHIP Authors
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

#include <app/TestEventTriggerDelegate.h>
#include <lib/core/CHIPError.h>

#include <cstdint>

/**
 * @brief User handler for the Network Identity Management test event trigger.
 *
 * @note If the trigger is enabled it must be implemented by the application, since only the
 *       application holds a reference to its AuthenticatorDriver instance.
 *
 * @param eventTrigger Event trigger to handle.
 *
 * @retval true  if the trigger was recognised and handled.
 * @retval false to allow other cluster handlers to check the trigger.
 */
bool HandleNetworkIdentityManagementTestEventTrigger(uint64_t eventTrigger);

namespace chip {

/*
 * Test event triggers for the Network Identity Management cluster, used by TC-NETIM-1.4.
 *
 * A client's association with a Network Identity is normally established by an out-of-band
 * PDC / EAP-TLS authentication that a Matter test harness cannot perform. This trigger lets the
 * harness simulate it: it authenticates an existing client (whose ClientIndex is carried in the
 * low 16 bits of the trigger value) against the current ECDSA Network Identity. That makes the
 * Network Identity referenced by a client so it is not retired on the next ImportAdminSecret,
 * allowing the ActiveNetworkIdentities table to be filled to capacity.
 *
 * They are sent along with the enableKey (a manufacturer-defined secret) in the General
 * Diagnostics cluster TestEventTrigger command.
 */
enum class NetworkIdentityManagementTrigger : uint64_t
{
    // Authenticate the client identified by the low 16 bits against the current Network Identity.
    kAuthenticateClientAgainstCurrentIdentity = 0x0450000000000000,
};

class NetworkIdentityManagementTestEventTriggerHandler : public TestEventTriggerHandler
{
public:
    NetworkIdentityManagementTestEventTriggerHandler() = default;

    /** Returns CHIP_NO_ERROR if the trigger is recognised and handled, otherwise an error so a
     *  higher-level handler can check other clusters. */
    CHIP_ERROR HandleEventTrigger(uint64_t eventTrigger) override
    {
        // The framework encodes the endpoint in bits [32:47]; clear it before matching.
        eventTrigger = clearEndpointInEventTrigger(eventTrigger);
        if (HandleNetworkIdentityManagementTestEventTrigger(eventTrigger))
        {
            return CHIP_NO_ERROR;
        }
        return CHIP_ERROR_INVALID_ARGUMENT;
    }
};

} // namespace chip
