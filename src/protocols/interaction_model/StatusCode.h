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

#pragma once

#include <limits>
#include <stdint.h>

#include <lib/core/CHIPConfig.h>
#include <lib/core/Optional.h>
#include <lib/support/TypeTraits.h>

#if CHIP_CONFIG_IM_STATUS_CODE_VERBOSE_FORMAT
#define ChipLogFormatIMStatus "0x%02x (%s)"
#define ChipLogValueIMStatus(status) chip::to_underlying(status), chip::Protocols::InteractionModel::StatusName(status)
#else // CHIP_CONFIG_IM_STATUS_CODE_VERBOSE_FORMAT
#define ChipLogFormatIMStatus "0x%02x"
#define ChipLogValueIMStatus(status) chip::to_underlying(status)
#endif // CHIP_CONFIG_IM_STATUS_CODE_VERBOSE_FORMAT

namespace chip {
namespace Protocols {
namespace InteractionModel {

// This table comes from the IM's "Status Code Table" section from the Interaction Model spec.
enum class Status : uint8_t
{
#define CHIP_IM_STATUS_CODE(name, spec_name, value) name = value,
#include <protocols/interaction_model/StatusCodeList.h>
#undef CHIP_IM_STATUS_CODE

    InvalidValue = ConstraintError, // Deprecated
};

#if CHIP_CONFIG_IM_STATUS_CODE_VERBOSE_FORMAT
const char * StatusName(Status status);
#endif // CHIP_CONFIG_IM_STATUS_CODE_VERBOSE_FORMAT

/**
 * @brief Class to encapsulate a Status code, including possibly a
 *        cluster-specific code for generic SUCCESS/FAILURE.
 *
 * This can be used everywhere a `Status` is used, but it is lossy
 * to the cluster-specific code if used in place of `Status` when
 * the cluster-specific code is set.
 *
 * This class can only be directly constructed from a `Status`. To
 * attach a cluster-specific-code, please use the `ClusterSpecificFailure`
 * and `ClusterSpecificSuccess` factory methods.
 */
class StatusCode
{
  public:
    explicit StatusCode(Status status) : mStatus(status) {}

    // We only have simple copyable members, so we should be trivially copyable.
    StatusCode(const StatusCode & other) = default;
    StatusCode & operator=(const StatusCode & other) = default;

    bool operator==(const StatusCode &other)
    {
        return (this->mStatus == other.mStatus)
            && (this->HasClusterSpecificCode() == other.HasClusterSpecificCode())
            && (this->GetClusterSpecificCode() == other.GetClusterSpecificCode());
    }

    bool operator!=(const StatusCode &other)
    {
        return !(*this == other);
    }

    StatusCode & operator=(const Status & status)
    {
        this->mStatus = status;
        this->mClusterSpecificCode = chip::NullOptional;
        return *this;
    }

    /**
     * @brief Builder for a cluster-specific failure status code.
     *
     * @tparam T - enum type for the cluster-specific status code
     *             (e.g. chip::app::Clusters::AdministratorCommissioning::CommissioningWindowStatusEnum)
     * @param cluster_specific_code - cluster-specific code to record with the failure
     *             (e.g. chip::app::Clusters::AdministratorCommissioning::CommissioningWindowStatusEnum::kWindowNotOpen)
     * @return a StatusCode instance properly configured.
     */
    template <typename T>
    static StatusCode ClusterSpecificFailure(T cluster_specific_code)
    {
        static_assert(std::numeric_limits<T>::max() <= std::numeric_limits<uint8_t>::max(), "Type used must fit in uint8_t");
        return StatusCode(Status::Failure, static_cast<uint8_t>(cluster_specific_code));
    }

    /**
     * @brief Builder for a cluster-specific success status code.
     *
     * @tparam T - enum type for the cluster-specific status code
     *             (e.g. chip::app::Clusters::AdministratorCommissioning::CommissioningWindowStatusEnum)
     * @param cluster_specific_code - cluster-specific code to record with the success
     *             (e.g. chip::app::Clusters::AdministratorCommissioning::CommissioningWindowStatusEnum::kBasicWindowOpen)
     * @return a StatusCode instance properly configured.
     */
    template <typename T>
    static StatusCode ClusterSpecificSuccess(T cluster_specific_code)
    {
        static_assert(std::numeric_limits<T>::max() <= std::numeric_limits<uint8_t>::max(), "Type used must fit in uint8_t");
        return StatusCode(Status::Success, static_cast<uint8_t>(cluster_specific_code));
    }

    /// @return true if the core Status associated with this StatusCode is the one for success.
    bool IsSuccess() const { return mStatus == Status::Success; }

    /// @return the core Status code associated withi this StatusCode.
    Status GetStatus() const { return mStatus; }

    /// @return true if a cluster-specific code is associated with the StatusCode.
    bool HasClusterSpecificCode() const { return mClusterSpecificCode.HasValue(); }

    /// @return the cluster-specific code associated with this StatusCode or chip::NullOptional if none is associated.
    chip::Optional<uint8_t> GetClusterSpecificCode() const
    {
        if (!mClusterSpecificCode.HasValue() || !((mStatus == Status::Failure) || (mStatus == Status::Success)))
        {
            return chip::NullOptional;
        }
        return mClusterSpecificCode;
    }

    // Automatic conversions to common types, using the status code alone.
    operator Status() const { return mStatus; }
    operator int() const { return static_cast<int>(mStatus); }
    operator uint8_t() const { return static_cast<uint8_t>(mStatus); }

  private:
    StatusCode() = delete;
    StatusCode(Status status, uint8_t cluster_specific_code) : mStatus(status), mClusterSpecificCode(chip::MakeOptional(cluster_specific_code)) { }

    Status mStatus;
    chip::Optional<uint8_t> mClusterSpecificCode;
};

} // namespace InteractionModel
} // namespace Protocols
} // namespace chip
