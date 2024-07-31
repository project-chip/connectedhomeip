/*
 *    Copyright (c) 2024 Project CHIP Authors
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
#include <app/data-model-provider/ActionReturnStatus.h>

#include <lib/support/StringBuilder.h>

namespace chip {
namespace app {
namespace DataModel {

Protocols::InteractionModel::ClusterStatusCode ActionReturnStatus::GetStatusCode() const
{
    // FIXME
}

bool ActionReturnStatus::IsError() const
{

    // FIXME
}

bool ActionReturnStatus::IsOutOfSpaceError() const
{

    // FIXME
}

void ActionReturnStatus::LogError(const char * prefix) const
{

    if (mError.has_value())
    {
        ChipLogError(InteractionModel, "%s: %" CHIP_ERROR_FORMAT, prefix, mError->Format());
    }

    if (mStatusCode.has_value())
    {
        StringBuilder<48> txt;

#if CHIP_CONFIG_IM_STATUS_CODE_VERBOSE_FORMAT
        txt.AddFormat("%s(%d)", Protocols::InteractionModel::StatusName(mStatusCode->GetStatus()),
                      static_cast<int>(mStatusCode->GetStatus()));
#else
        if (mStatusCode->IsSuccess())
        {
            txt.Add("Success");
        }
        else
        {
            txt.AddFormat("Status<%d>", static_cast<int>(mStatusCode->GetStatus()));
        }
#endif

        chip::Optional<ClusterStatus> clusterCode = mStatusCode->GetClusterSpecificCode();
        if (mStatusCode.has_value())
        {
            txt.AddFormat(", Code %d", static_cast<int>(clusterCode.Value()));
        }

        ChipLogError(InteractionModel, "%s: %s", prefix, txt.c_str());
    }

    if (!mError.has_value() && !mStatusCode.has_value())
    {
        // This should be impossible - we enforce this in constructors ...
        ChipLogError(InteractionModel, "%s: Unknown/Invalid action return status", prefix);
    }
}

} // namespace DataModel
} // namespace app
} // namespace chip
