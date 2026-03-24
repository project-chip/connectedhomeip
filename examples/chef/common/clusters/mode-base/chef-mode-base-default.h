/*
 *
 *    Copyright (c) 2023 Project CHIP Authors
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

#include <app/clusters/mode-base-server/mode-base-server.h>
#include <lib/support/Span.h>

namespace chip {
namespace app {
namespace Clusters {
namespace ModeBase {

class DefaultChefDelegate : public Delegate
{
public:
    DefaultChefDelegate(const Span<const detail::Structs::ModeOptionStruct::Type> aModeOptions) : mModeOptions(aModeOptions) {}
    ~DefaultChefDelegate() override = default;

    CHIP_ERROR Init() override;
    CHIP_ERROR GetModeLabelByIndex(uint8_t modeIndex, MutableCharSpan & label) override;
    CHIP_ERROR GetModeValueByIndex(uint8_t modeIndex, uint8_t & value) override;
    CHIP_ERROR GetModeTagsByIndex(uint8_t modeIndex, DataModel::List<detail::Structs::ModeTagStruct::Type> & modeTags) override;
    void HandleChangeToMode(uint8_t NewMode, ModeBase::Commands::ChangeToModeResponse::Type & response) override;

private:
    Span<const detail::Structs::ModeOptionStruct::Type> mModeOptions;
};

} // namespace ModeBase
} // namespace Clusters
} // namespace app
} // namespace chip
