/*
 *
 *    Copyright (c) 2026 Project CHIP Authors
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

#include <app/OperationalSessionSetup.h>
#include <app/clusters/bindings/binding-table.h>
#include <lib/core/DataModelTypes.h>
#include <messaging/ExchangeMgr.h>

namespace chip {
namespace app {

/**
 * Abstract base class for binding command contexts.
 *
 * Ensures type-safe dispatch and safe deallocation of context data passed to
 * Clusters::Binding::Manager::NotifyBoundClusterChanged.
 */
class BindingContext
{
public:
    virtual ~BindingContext() = default;

    /**
     * Target cluster ID for this binding command.
     */
    virtual ClusterId GetClusterId() const = 0;

    /**
     * Dispatches the command to a bound peer (unicast or multicast).
     *
     * @param binding Table entry for the binding.
     * @param peer_device Proxy to connected peer (nullptr for multicast group bindings).
     * @param exchangeManager Exchange manager registered through BindingHandler::Init. Unicast commands
     *                        should use peer_device->GetExchangeManager(); this one is meant for
     *                        multicast (group) commands, which have no peer device.
     */
    virtual void DispatchCommand(const Clusters::Binding::TableEntry & binding, OperationalDeviceProxy * peer_device,
                                 Messaging::ExchangeManager & exchangeManager) = 0;
};

} // namespace app
} // namespace chip
