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
#include <lib/core/CHIPConfig.h>
#include <lib/core/CHIPError.h>
#include <lib/support/CodeUtils.h>
#include <protocols/interaction_model/StatusCode.h>

#include <lib/support/StringBuilder.h>

namespace chip {
namespace app {
namespace DataModel {

using Protocols::InteractionModel::ClusterStatusCode;
using Protocols::InteractionModel::Status;

namespace {

bool StatusIsTheSameAsError(const ClusterStatusCode & status, const CHIP_ERROR & err)
{
    auto cluster_code = status.GetClusterSpecificCode();
    if (!cluster_code.HasValue())
    {
        // there exist Status::Success, however that may not be encoded
        // as a CHIP_ERROR_IM_GLOBAL_STATUS_VALUE as it is just as well a CHIP_NO_ERROR.
        // handle that separately
        if ((status.GetStatus() == Status::Success) && (err == CHIP_NO_ERROR))
        {
            return true;
        }

        return err == CHIP_ERROR_IM_GLOBAL_STATUS_VALUE(status.GetStatus());
    }

    if (status.GetStatus() != Status::Failure)
    {
        return false;
    }

    return err == CHIP_ERROR_IM_CLUSTER_STATUS_VALUE(cluster_code.Value());
}

} // namespace

bool ActionReturnStatus::operator==(const ActionReturnStatus & other) const
{
    if (mReturnStatus == other.mReturnStatus)
    {
        return true;
    }

    const ClusterStatusCode * thisStatus  = std::get_if<ClusterStatusCode>(&mReturnStatus);
    const ClusterStatusCode * otherStatus = std::get_if<ClusterStatusCode>(&other.mReturnStatus);

    const CHIP_ERROR * thisErr  = std::get_if<CHIP_ERROR>(&mReturnStatus);
    const CHIP_ERROR * otherErr = std::get_if<CHIP_ERROR>(&other.mReturnStatus);

    if (thisStatus && otherErr)
    {
        return StatusIsTheSameAsError(*thisStatus, *otherErr);
    }

    if (otherStatus && thisErr)
    {
        return StatusIsTheSameAsError(*otherStatus, *thisErr);
    }

    return false;
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
        return ClusterStatusCode(*err);
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

bool ActionReturnStatus::IsOutOfSpaceEncodingResponse() const
{
    if (const CHIP_ERROR * err = std::get_if<CHIP_ERROR>(&mReturnStatus))
    {
        return (*err == CHIP_ERROR_NO_MEMORY) || (*err == CHIP_ERROR_BUFFER_TOO_SMALL);
    }

    return false;
}

const char * ActionReturnStatus::c_str(ActionReturnStatus::StringStorage & storage) const
{
    if (const CHIP_ERROR * err = std::get_if<CHIP_ERROR>(&mReturnStatus))
    {
#if CHIP_CONFIG_ERROR_FORMAT_AS_STRING
        return err->Format(); // any length
#else
        storage.formatBuffer.Reset().AddFormat("%" CHIP_ERROR_FORMAT, err->Format());
        return storage.formatBuffer.c_str();
#endif
    }

    if (const ClusterStatusCode * status = std::get_if<ClusterStatusCode>(&mReturnStatus))
    {
        storage.formatBuffer.Reset();

#if CHIP_CONFIG_IM_STATUS_CODE_VERBOSE_FORMAT
        storage.formatBuffer.AddFormat("%s(%d)", Protocols::InteractionModel::StatusName(status->GetStatus()),
                                       static_cast<int>(status->GetStatus()));
#else
        if (status->IsSuccess())
        {
            storage.formatBuffer.Add("Success");
        }
        else
        {
            storage.formatBuffer.AddFormat("Status<%d>", static_cast<int>(status->GetStatus()));
        }
#endif

        chip::Optional<ClusterStatus> clusterCode = status->GetClusterSpecificCode();
        if (clusterCode.HasValue())
        {
            storage.formatBuffer.AddFormat(", Code %d", static_cast<int>(clusterCode.Value()));
        }
        return storage.formatBuffer.c_str();
    }

    // all std::variant cases exhausted
    chipDie();
}

} // namespace DataModel
} // namespace app
} // namespace chip
