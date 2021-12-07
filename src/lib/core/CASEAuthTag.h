/*
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

#include <cstdint>
#include <lib/core/CHIPConfig.h>
#include <lib/support/CodeUtils.h>
#include <stdlib.h>

namespace chip {

typedef uint32_t CASEAuthTag;

static constexpr CASEAuthTag kUndefinedCAT = 0;

// Muximum number of CASE Authenticated Tags (CAT) in the CHIP certificate subject.
static constexpr size_t kMaxSubjectCATAttributeCount = CHIP_CONFIG_CERT_MAX_RDN_ATTRIBUTES - 2;

struct CATValues
{
    CASEAuthTag val[kMaxSubjectCATAttributeCount];

    size_t size() const { return ArraySize(val); }
};

static constexpr CATValues kUndefinedCATs = { { kUndefinedCAT } };

} // namespace chip
