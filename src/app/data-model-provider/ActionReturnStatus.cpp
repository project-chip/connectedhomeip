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
#include "lib/core/CHIPError.h"
#include "lib/support/CodeUtils.h"
#include "protocols/interaction_model/StatusCode.h"
#include <app/data-model-provider/ActionReturnStatus.h>

#include <lib/support/StringBuilder.h>

namespace chip {
namespace app {
namespace DataModel {

using Protocols::InteractionModel::ClusterStatusCode;
using Protocols::InteractionModel::Status;

bool ActionReturnStatus::operator==(Protocols::InteractionModel::Status status)
{
    if (const CHIP_ERROR * err = std::get_if<CHIP_ERROR>(&mReturnStatus))
    {
        return *err == CHIP_ERROR_IM_GLOBAL_STATUS_VALUE(status);
    }

    if (const ClusterStatusCode * internal_status = std::get_if<ClusterStatusCode>(&mReturnStatus))
    {
        if (internal_status->HasClusterSpecificCode())
        {
            return false; // status has no cluster-specific code, so reject equality
        }

        return internal_status->GetStatus() == status;
    }

    chipDie();
}

CHIP_ERROR ActionReturnStatus::GetUnderlyingError() const
{

    if (const CHIP_ERROR * err = std::get_if<CHIP_ERROR>(&mReturnStatus))
    {
        return *err;
    }

    if (const ClusterStatusCode * status = std::get_if<ClusterStatusCode>(&mReturnStatus))
    {
        if (status->IsSuccess())
        {
            return CHIP_NO_ERROR;
        }

        chip::Optional<ClusterStatus> code = status->GetClusterSpecificCode();

        return code.HasValue() ? CHIP_ERROR_IM_CLUSTER_STATUS_VALUE(code.Value())
                               : CHIP_ERROR_IM_GLOBAL_STATUS_VALUE(status->GetStatus());
    }

    chipDie();
}

ClusterStatusCode ActionReturnStatus::GetStatusCode() const
{
    if (const ClusterStatusCode * status = std::get_if<ClusterStatusCode>(&mReturnStatus))
    {
        return *status;
    }

    if (const CHIP_ERROR * err = std::get_if<CHIP_ERROR>(&mReturnStatus))
    {
        if (err->IsPart(ChipError::SdkPart::kIMClusterStatus))
        {
            return ClusterStatusCode::ClusterSpecificFailure(err->GetSdkCode());
        }

        if (*err == CHIP_NO_ERROR)
        {
            return ClusterStatusCode(Status::Success);
        }

        if (err->IsPart(ChipError::SdkPart::kIMGlobalStatus))
        {
            return ClusterStatusCode(static_cast<Status>(err->GetSdkCode()));
        }

        return ClusterStatusCode(Status::Failure);
    }

    // all std::variant cases exhausted
    chipDie();
}

bool ActionReturnStatus::IsSuccess() const
{
    if (const CHIP_ERROR * err = std::get_if<CHIP_ERROR>(&mReturnStatus))
    {
        return (*err == CHIP_NO_ERROR);
    }

    if (const ClusterStatusCode * status = std::get_if<ClusterStatusCode>(&mReturnStatus))
    {
        return status->IsSuccess();
    }

    // all std::variant cases exhausted
    chipDie();
}

bool ActionReturnStatus::IsOutOfSpaceError() const
{
    if (const CHIP_ERROR * err = std::get_if<CHIP_ERROR>(&mReturnStatus))
    {
        return (*err == CHIP_ERROR_NO_MEMORY) || (*err == CHIP_ERROR_BUFFER_TOO_SMALL);
    }

    return false;
}

void ActionReturnStatus::AddTo(StringBuilderBase & buffer) const
{
    if (const CHIP_ERROR * err = std::get_if<CHIP_ERROR>(&mReturnStatus))
    {
        buffer.AddFormat("%" CHIP_ERROR_FORMAT, err->Format());
        return;
    }

    if (const ClusterStatusCode * status = std::get_if<ClusterStatusCode>(&mReturnStatus))
    {
#if CHIP_CONFIG_IM_STATUS_CODE_VERBOSE_FORMAT
        buffer.AddFormat("%s(%d)", Protocols::InteractionModel::StatusName(status->GetStatus()),
                      static_cast<int>(status->GetStatus()));
#else
        if (status->IsSuccess())
        {
            buffer.Add("Success");
        }
        else
        {
            buffer.AddFormat("Status<%d>", static_cast<int>(status->GetStatus()));
        }
#endif

        chip::Optional<ClusterStatus> clusterCode = status->GetClusterSpecificCode();
        if (clusterCode.HasValue())
        {
            buffer.AddFormat(", Code %d", static_cast<int>(clusterCode.Value()));
        }
        return;
    }

    // all std::variant cases exhausted
    chipDie();
}

} // namespace DataModel
} // namespace app
} // namespace chip
