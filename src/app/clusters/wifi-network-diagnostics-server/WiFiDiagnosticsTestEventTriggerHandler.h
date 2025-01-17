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

#include <app-common/zap-generated/cluster-objects.h>
#include <app/TestEventTriggerDelegate.h>

/**
 * @brief User handler for handling the test event trigger
 *
 * @note If TestEventTrigger is enabled, it needs to be implemented in the app
 *
 * @param eventTrigger Event trigger to handle
 *
 * @retval true on success
 * @retval false if error happened
 */
bool HandleWiFiDiagnosticsTestEventTrigger(uint64_t eventTrigger);

namespace chip {

/*
 * These Test EventTrigger values are specified in the TC_DGWIFI test plan
 * and are defined conditions used in test events.
 *
 * They are sent along with the enableKey (manufacturer defined secret)
 * in the General Diagnostic cluster TestEventTrigger command
 */
enum class WiFiDiagnosticsTrigger : uint64_t
{
    // Simulate a disconnection via de-authentication or dis-association.
    kDisconnection = 0x0036000000000000,

    // Force a scenario where the DUT exhausts all internal retries,
    // and triggers an AssociationFailure event.
    kAssociationFailure = 0x0036000000000001,

    // Simulate disconnecting and reconnecting the node’s Wi-Fi,
    // so that a ConnectionStatus event is triggered.
    kConnectionStatus = 0x0036000000000002,
};

class WiFiDiagnosticsTestEventTriggerHandler : public TestEventTriggerHandler
{
public:
    explicit WiFiDiagnosticsTestEventTriggerHandler() {}

    /** This function must return True if the eventTrigger is recognised and handled
     *  It must return False to allow a higher level TestEvent handler to check other
     *  clusters that may handle it.
     */
    CHIP_ERROR HandleEventTrigger(uint64_t eventTrigger) override
    {
        if (HandleWiFiDiagnosticsTestEventTrigger(eventTrigger))
        {
            return CHIP_NO_ERROR;
        }
        return CHIP_ERROR_INVALID_ARGUMENT;
    }
};

} // namespace chip
