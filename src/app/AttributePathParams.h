/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
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

#include <app/util/basic-types.h>
#include <support/BitFlags.h>

namespace chip {
namespace app {
enum class AttributePathSelectorFlag : uint8_t
{
    kInValid        = 0x00,
    kFieldIdValid   = 0x01,
    kListIndexValid = 0x02,
};

struct AttributePathSelector
{
    BitFlags<AttributePathSelectorFlag> mFlag = AttributePathSelectorFlag::kInValid;
    FieldId mFieldId                          = 0;
    ListIndex mListIndex                      = 0;
    AttributePathSelector * mpNext            = nullptr;
};

struct AttributePathParams
{
    NodeId mNodeId                     = 0;
    EndpointId mEndpointId             = 0;
    ClusterId mClusterId               = 0;
    AttributePathSelector * mpSelector = nullptr;
};
} // namespace app
} // namespace chip
