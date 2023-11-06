/**
 *
 *    Copyright (c) 2020 Project CHIP Authors
 *    Copyright (c) 2016-2017 Nest Labs, Inc.
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

#include "StructParser.h"
#include <app/AppBuildConfig.h>
#include <app/InteractionModelRevision.h>
#include <app/util/basic-types.h>

namespace chip {
namespace app {
class MessageParser : public StructParser
{
public:
    CHIP_ERROR Init(TLV::TLVReader & aReader);
    CHIP_ERROR ExitContainer();
#if CHIP_CONFIG_IM_PRETTY_PRINT
    CHIP_ERROR CheckInteractionModelRevision(TLV::TLVReader & aReader) const;
#endif // CHIP_CONFIG_IM_PRETTY_PRINT

    CHIP_ERROR GetInteractionModelRevision(InteractionModelRevision * const apInteractionModelRevision) const;
};
} // namespace app
} // namespace chip
