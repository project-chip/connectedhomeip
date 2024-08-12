/*
 *    Copyright (c) 2022 Project CHIP Authors
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

#include <lib/core/CHIPConfig.h>
#include <protocols/interaction_model/StatusCode.h>

namespace chip {
namespace Protocols {
namespace InteractionModel {

#if CHIP_CONFIG_IM_STATUS_CODE_VERBOSE_FORMAT
const char * StatusName(Status status)
{
    switch (status)
    {
#define CHIP_IM_STATUS_CODE(name, spec_name, value)                                                                                \
    case Status(value):                                                                                                            \
        return #spec_name;
#include <protocols/interaction_model/StatusCodeList.h>
#undef CHIP_IM_STATUS_CODE
    }

    return "Unallocated";
}
#endif // CHIP_CONFIG_IM_STATUS_CODE_VERBOSE_FORMAT
//
ClusterStatusCode::ClusterStatusCode(CHIP_ERROR err)
{
    if (err.IsPart(ChipError::SdkPart::kIMClusterStatus))
    {
        mStatus              = Status::Failure;
        mClusterSpecificCode = chip::MakeOptional(err.GetSdkCode());
        return;
    }

    if (err == CHIP_NO_ERROR)
    {
        mStatus = Status::Success;
        return;
    }

    if (err.IsPart(ChipError::SdkPart::kIMGlobalStatus))
    {
        mStatus = static_cast<Status>(err.GetSdkCode());
        return;
    }

    mStatus = Status::Failure;
}

} // namespace InteractionModel
} // namespace Protocols
} // namespace chip
