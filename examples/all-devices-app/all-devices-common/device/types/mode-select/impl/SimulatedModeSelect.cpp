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

#include "SimulatedModeSelect.h"

namespace chip {
namespace app {

using namespace Clusters::ModeSelect;

const Structs::ModeOptionStruct::Type SimulatedModeSelect::kSupportedModes[] = {
    { "Black"_span, 0, {} },
    { "White"_span, 1, {} },
    { "Red"_span, 2, {} },
};

SimulatedModeSelect::SimulatedModeSelect(DeviceLayer::DiagnosticDataProvider & diagnosticDataProvider) :
    ModeSelect(ModeSelect::Config{
        *this,
        {
            BitMask<Feature>(Feature::kOnOff),
            Clusters::ModeSelectCluster::OptionalAttributeSet(),
            "Mode Select Simulated Device"_span,
            DataModel::NullNullable,
            false,
            diagnosticDataProvider,
        }
    })
{}

Span<const Structs::ModeOptionStruct::Type> SimulatedModeSelect::GetSupportedModes() const
{
    return Span<const Structs::ModeOptionStruct::Type>(kSupportedModes);
}

void SimulatedModeSelect::OnModeChanged(uint8_t newMode)
{
    ChipLogProgress(Support, "SimulatedModeSelect: Mode changed to %u", newMode);
}

} // namespace app
} // namespace chip
