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
/**
 *    @file
 *      This file defines Status Information Block in Interaction Model
 *
 */

#pragma once

#include "StructBuilder.h"
#include "StructParser.h"

#include <app/AppConfig.h>
#include <app/util/basic-types.h>
#include <lib/core/CHIPCore.h>
#include <lib/core/Optional.h>
#include <lib/core/TLV.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/logging/CHIPLogging.h>
#include <protocols/interaction_model/Constants.h>
#include <protocols/interaction_model/StatusCode.h>
#include <protocols/secure_channel/Constants.h>

namespace chip {
namespace app {
struct StatusIB
{
    StatusIB() = default;
    explicit StatusIB(Protocols::InteractionModel::Status imStatus) : mStatus(imStatus) {}

    explicit StatusIB(Protocols::InteractionModel::Status imStatus, ClusterStatus clusterStatus) :
        mStatus(imStatus), mClusterStatus(clusterStatus)
    {}

    explicit StatusIB(const Protocols::InteractionModel::ClusterStatusCode & statusCode) : mStatus(statusCode.GetStatus())
    {
        // NOTE: Cluster-specific codes are only valid on SUCCESS/FAILURE IM status (7.10.7. Status Codes)
        chip::Optional<ClusterStatus> clusterStatus = statusCode.GetClusterSpecificCode();
        if (clusterStatus.HasValue())
        {
            mStatus        = statusCode.IsSuccess() ? Protocols::InteractionModel::Status::Success
                                                    : Protocols::InteractionModel::Status::Failure;
            mClusterStatus = clusterStatus;
        }
    }

    explicit StatusIB(CHIP_ERROR error) : StatusIB(Protocols::InteractionModel::ClusterStatusCode(error)) {}

    enum class Tag : uint8_t
    {
        kStatus        = 0,
        kClusterStatus = 1,
    };

    class Parser : public StructParser
    {
    public:
#if CHIP_CONFIG_IM_PRETTY_PRINT
        CHIP_ERROR PrettyPrint() const;
#endif // CHIP_CONFIG_IM_PRETTY_PRINT
        /**
         * Decode the StatusIB
         *
         * @return       CHIP_ERROR codes returned by chip::TLV objects. CHIP_END_OF_TLV if either
         *               element is missing. CHIP_ERROR_WRONG_TLV_TYPE if the elements are of the wrong
         *               type.
         */
        CHIP_ERROR DecodeStatusIB(StatusIB & aStatusIB) const;
    };

    class Builder : public StructBuilder
    {
    public:
        /**
         * Write the StatusIB into TLV and close the container
         *
         * @return       CHIP_ERROR codes returned by chip::TLV objects. CHIP_END_OF_TLV if either
         *               element is missing. CHIP_ERROR_WRONG_TLV_TYPE if the elements are of the wrong
         *               type.
         */
        StatusIB::Builder & EncodeStatusIB(const StatusIB & aStatusIB);
    };

    /**
     * Encapsulate a StatusIB in a CHIP_ERROR.  This can be done for any
     * StatusIB, but will treat all success codes (including cluster-specific
     * ones) as CHIP_NO_ERROR.  The resulting CHIP_ERROR will either be
     * CHIP_NO_ERROR or test true for IsIMStatus().
     */
    CHIP_ERROR ToChipError() const;

    /**
     * Test whether this status is a success.
     */
    bool IsSuccess() const { return mStatus == Protocols::InteractionModel::Status::Success; }

    /**
     * Test whether this status is a failure.
     */
    bool IsFailure() const { return !IsSuccess(); }

    /**
     * Register the StatusIB error formatter.
     */
    static void RegisterErrorFormatter();

    Protocols::InteractionModel::Status mStatus = Protocols::InteractionModel::Status::Success;
    Optional<ClusterStatus> mClusterStatus      = Optional<ClusterStatus>::Missing();

}; // struct StatusIB

constexpr bool operator==(const StatusIB & one, const StatusIB & two)
{
    return one.mStatus == two.mStatus && one.mClusterStatus == two.mClusterStatus;
}

constexpr bool operator!=(const StatusIB & one, const StatusIB & two)
{
    return !(one == two);
}

}; // namespace app
}; // namespace chip
