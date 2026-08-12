/**
 *    Copyright (c) 2024-2025 Project CHIP Authors
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

#include "PresetStructWithOwnedMembers.h"
#include "Setpoints.h"
#include <app/AttributeValueDecoder.h>
#include <app/AttributeValueEncoder.h>
#include <app/CommandHandler.h>
#include <app/ConcreteAttributePath.h>
#include <app/ConcreteCommandPath.h>
#include <app/data-model-provider/ActionReturnStatus.h>
#include <app/data-model-provider/OperationTypes.h>

namespace chip {
namespace app {
namespace Clusters {
namespace Thermostat {

class ThermostatCluster;

class ThermostatPresets
{
public:
    class Delegate
    {
    public:
        virtual ~Delegate() = default;

        virtual CHIP_ERROR GetPresetTypeAtIndex(size_t index, Structs::PresetTypeStruct::Type & presetType)       = 0;
        virtual uint8_t GetNumberOfPresets()                                                                      = 0;
        virtual CHIP_ERROR GetPresetAtIndex(size_t index, PresetStructWithOwnedMembers & preset)                  = 0;
        virtual CHIP_ERROR GetPendingPresetAtIndex(size_t index, PresetStructWithOwnedMembers & preset)           = 0;
        virtual CHIP_ERROR GetActivePresetHandle(DataModel::Nullable<MutableByteSpan> & activePresetHandle)       = 0;
        virtual CHIP_ERROR SetActivePresetHandle(const DataModel::Nullable<ByteSpan> & activePresetHandle)        = 0;
        virtual CHIP_ERROR GetScheduleTypeAtIndex(size_t index, Structs::ScheduleTypeStruct::Type & scheduleType) = 0;

        virtual void InitializePendingPresets()                                                   = 0;
        virtual void ClearPendingPresetList()                                                     = 0;
        virtual CHIP_ERROR AppendToPendingPresetList(const PresetStructWithOwnedMembers & preset) = 0;
        virtual CHIP_ERROR CommitPendingPresets()                                                 = 0;

        virtual std::optional<System::Clock::Milliseconds16> GetMaxAtomicWriteTimeout(chip::AttributeId attributeId)
        {
            return std::nullopt;
        }
    };

    ThermostatPresets() = delete;
    ThermostatPresets(ThermostatCluster & cluster) : mCluster(cluster) {}

    void SetDelegate(Delegate * delegate) { mDelegate = delegate; }
    Delegate * GetDelegate() const { return mDelegate; }

    std::optional<DataModel::ActionReturnStatus> ReadAttribute(const DataModel::ReadAttributeRequest & request,
                                                               AttributeValueEncoder & encoder);
    std::optional<DataModel::ActionReturnStatus> WriteAttribute(const DataModel::WriteAttributeRequest & request,
                                                                AttributeValueDecoder & decoder);
    std::optional<DataModel::ActionReturnStatus> InvokeCommand(const DataModel::InvokeRequest & request,
                                                               TLV::TLVReader & input_arguments, CommandHandler * handler);

    std::optional<Protocols::InteractionModel::Status> OnAtomicWriteBegin(AttributeId attributeId);
    std::optional<Protocols::InteractionModel::Status> OnAtomicWritePrecommit(AttributeId attributeId);
    std::optional<Protocols::InteractionModel::Status> OnAtomicWriteCommit(AttributeId attributeId);
    std::optional<Protocols::InteractionModel::Status> OnAtomicWriteRollback(AttributeId attributeId);

    std::optional<System::Clock::Milliseconds16> GetMaxAtomicWriteTimeout(AttributeId attributeId);

    Protocols::InteractionModel::Status SetActivePreset(DataModel::Nullable<ByteSpan> presetHandle);
    CHIP_ERROR AppendPendingPreset(const Structs::PresetStruct::Type & newPreset);
    Protocols::InteractionModel::Status PrecommitPresets();

    bool IsPresetHandlePresentInPresets(const ByteSpan & presetHandleToMatch);

private:
    ThermostatCluster & mCluster;
    Delegate * mDelegate = nullptr;
};

} // namespace Thermostat
} // namespace Clusters
} // namespace app
} // namespace chip
