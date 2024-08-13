/**
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

/****************************************************************************
 * @file
 * @brief APIs for the  Thermostat cluster.
 *
 *******************************************************************************
 ******************************************************************************/

#pragma once

#include "atomic-write.h"
#include "thermostat-delegate.h"

#include <app/AttributeAccessInterfaceRegistry.h>
#include <app/CommandHandler.h>

namespace chip {
namespace app {
namespace Clusters {
namespace Thermostat {

/**
 * @brief  Thermostat Attribute Access Interface.
 */
class ThermostatAttrAccess : public chip::app::AttributeAccessInterface,
                             public chip::FabricTable::Delegate,
                             public AtomicWriteDelegate
{
public:
    ThermostatAttrAccess() : AttributeAccessInterface(Optional<chip::EndpointId>::Missing(), Thermostat::Id) {}

    CHIP_ERROR Read(const ConcreteReadAttributePath & aPath, AttributeValueEncoder & aEncoder) override;
    CHIP_ERROR Write(const ConcreteDataAttributePath & aPath, chip::app::AttributeValueDecoder & aDecoder) override;

    imcode OnBeginWrite(EndpointId endpoint, AttributeId attributeId) override;
    imcode OnPreCommitWrite(EndpointId endpoint, AttributeId attributeId) override;
    imcode OnCommitWrite(EndpointId endpoint, AttributeId attributeId) override;
    imcode OnRollbackWrite(EndpointId endpoint, AttributeId attributeId) override;
    std::optional<System::Clock::Milliseconds16> GetWriteTimeout(EndpointId endpoint, chip::AttributeId attributeId) override;

    imcode SetActivePreset(EndpointId endpoint, ByteSpan newPresetHandle);

private:
    Thermostat::Delegate * GetDelegate(EndpointId endpoint);

    CHIP_ERROR AppendPendingPreset(Thermostat::Delegate * delegate, const Structs::PresetStruct::Type & preset);

    imcode BeginPresets(EndpointId endpoint);
    imcode PreCommitPresets(EndpointId endpoint);
    imcode CommitPresets(EndpointId endpoint);
    imcode RollbackPresets(EndpointId endpoint);

    void OnFabricRemoved(const FabricTable & fabricTable, FabricIndex fabricIndex) override;
};

/**
 * @brief Sets the default delegate for the  specific thermostat features.
 *
 * @param[in] endpoint The endpoint to set the default delegate on.
 * @param[in] delegate The default delegate.
 */
void SetDefaultDelegate(EndpointId endpoint, Delegate * delegate);

/**
 * @brief Sets the default atomic write manager for the specific thermostat endpoint.
 *
 * @param[in] endpoint The endpoint to set the default atomic write manager on.
 * @param[in] atomicWriteManager The default atomic write manager.
 */
void SetDefaultAtomicWriteManager(EndpointId endpoint, AtomicWriteManager * atomicWriteManager);

} // namespace Thermostat
} // namespace Clusters
} // namespace app
} // namespace chip
