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
#include <type_traits>

#include <stdint.h>

#include <lib/core/CHIPConfig.h>
#include <lib/core/DataModelTypes.h>
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
 * This abstractions joins together Status and ClusterStatus, which
 * are the components of a StatusIB, used in many IM actions, in a
 * way which allows both of them to carry together.
 *
 * This class can only be directly constructed from a `Status`. To
 * attach a cluster-specific-code, please use the `ClusterSpecificFailure()`
 * and `ClusterSpecificSuccess()` factory methods.
 */
class ClusterStatusCode
{
public:
    explicit ClusterStatusCode(Status status) : mStatus(status) {}
    explicit ClusterStatusCode(CHIP_ERROR err);

    // We only have simple copyable members, so we should be trivially copyable.
    ClusterStatusCode(const ClusterStatusCode & other)             = default;
    ClusterStatusCode & operator=(const ClusterStatusCode & other) = default;

    bool operator==(const ClusterStatusCode & other) const
    {
        return (this->mStatus == other.mStatus) && (this->HasClusterSpecificCode() == other.HasClusterSpecificCode()) &&
            (this->GetClusterSpecificCode() == other.GetClusterSpecificCode());
    }

    bool operator!=(const ClusterStatusCode & other) const { return !(*this == other); }

    ClusterStatusCode & operator=(const Status & status)
    {
        this->mStatus              = status;
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
     * @return a ClusterStatusCode instance properly configured.
     */
    template <typename T, typename std::enable_if_t<std::is_enum<T>::value, bool> = true>
    static ClusterStatusCode ClusterSpecificFailure(T cluster_specific_code)
    {
        static_assert(std::numeric_limits<std::underlying_type_t<T>>::max() <= std::numeric_limits<ClusterStatus>::max(),
                      "Type used must fit in uint8_t");
        return ClusterStatusCode(Status::Failure, chip::to_underlying(cluster_specific_code));
    }

    static ClusterStatusCode ClusterSpecificFailure(ClusterStatus cluster_specific_code)
    {
        return ClusterStatusCode(Status::Failure, cluster_specific_code);
    }

    /**
     * @brief Builder for a cluster-specific success status code.
     *
     * @tparam T - enum type for the cluster-specific status code
     *             (e.g. chip::app::Clusters::AdministratorCommissioning::CommissioningWindowStatusEnum)
     * @param cluster_specific_code - cluster-specific code to record with the success
     *             (e.g. chip::app::Clusters::AdministratorCommissioning::CommissioningWindowStatusEnum::kBasicWindowOpen)
     * @return a ClusterStatusCode instance properly configured.
     */
    template <typename T, typename std::enable_if_t<std::is_enum<T>::value, bool> = true>
    static ClusterStatusCode ClusterSpecificSuccess(T cluster_specific_code)
    {
        static_assert(std::numeric_limits<std::underlying_type_t<T>>::max() <= std::numeric_limits<ClusterStatus>::max(),
                      "Type used must fit in uint8_t");
        return ClusterStatusCode(Status::Success, chip::to_underlying(cluster_specific_code));
    }

    static ClusterStatusCode ClusterSpecificSuccess(ClusterStatus cluster_specific_code)
    {
        return ClusterStatusCode(Status::Success, cluster_specific_code);
    }

    /// @return true if the core Status associated with this ClusterStatusCode is the one for success.
    bool IsSuccess() const { return mStatus == Status::Success; }

    /// @return the core Status code associated withi this ClusterStatusCode.
    Status GetStatus() const { return mStatus; }

    /// @return true if a cluster-specific code is associated with the ClusterStatusCode.
    bool HasClusterSpecificCode() const { return mClusterSpecificCode.HasValue(); }

    /// @return the cluster-specific code associated with this ClusterStatusCode or chip::NullOptional if none is associated.
    chip::Optional<ClusterStatus> GetClusterSpecificCode() const
    {
        if ((mStatus != Status::Failure) && (mStatus != Status::Success))
        {
            return chip::NullOptional;
        }
        return mClusterSpecificCode;
    }

private:
    ClusterStatusCode() = delete;
    ClusterStatusCode(Status status, ClusterStatus cluster_specific_code) :
        mStatus(status), mClusterSpecificCode(chip::MakeOptional(cluster_specific_code))
    {}

    Status mStatus;
    chip::Optional<ClusterStatus> mClusterSpecificCode;
};

static_assert(sizeof(ClusterStatusCode) <= sizeof(uint32_t), "ClusterStatusCode must not grow to be larger than a uint32_t");

} // namespace InteractionModel
} // namespace Protocols
} // namespace chip
