/**
 *
 *    Copyright (c) 2021 Project CHIP Authors
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

#include <optional>

#include "StructBuilder.h"
#include <app/SpecificationDefinedRevisions.h>
#include <app/util/basic-types.h>

namespace chip {
namespace app {

struct RequestFlagsOptions
{
    bool suppressResponse = false;
    bool timedRequest     = false;
    std::optional<uint16_t> peerRevision;
    bool isGroupcast = false;
};

class MessageBuilder : public StructBuilder
{
public:
    CHIP_ERROR EncodeInteractionModelRevision();

protected:
    CHIP_ERROR EncodeRequestFlags(const RequestFlagsOptions & options, TLV::Tag suppressResponseTag, TLV::Tag timedRequestTag);
};
} // namespace app
} // namespace chip
