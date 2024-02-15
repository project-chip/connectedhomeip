/*
 *
 *    Copyright (c) 2023 Project CHIP Authors
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

#include <app-common/zap-generated/cluster-objects.h>

#include <lib/support/CommonIterator.h>

namespace chip {
namespace app {
namespace Clusters {
namespace OperationalState {

inline constexpr size_t kOperationalStateLabelMaxSize   = 64u;
inline constexpr size_t kOperationalErrorLabelMaxSize   = 64u;
inline constexpr size_t kOperationalErrorDetailsMaxSize = 64u;
inline constexpr size_t kOperationalPhaseNameMaxSize    = 64u;

/**
 * A class which represents the operational state of an Operational State cluster derivation instance.
 */
struct GenericOperationalState : public app::Clusters::detail::Structs::OperationalStateStruct::Type
{
    GenericOperationalState(uint8_t state = to_underlying(OperationalStateEnum::kStopped), Optional<CharSpan> label = NullOptional)
    {
        Set(state, label);
    }

    GenericOperationalState(const GenericOperationalState & op) { *this = op; }

    GenericOperationalState & operator=(const GenericOperationalState & op)
    {
        Set(op.operationalStateID, op.operationalStateLabel);
        return *this;
    }

    void Set(uint8_t state, Optional<CharSpan> label = NullOptional)
    {
        operationalStateID = state;
        if (label.HasValue())
        {
            memset(mOperationalStateLabelBuffer, 0, sizeof(mOperationalStateLabelBuffer));
            if (label.Value().size() > sizeof(mOperationalStateLabelBuffer))
            {
                memcpy(mOperationalStateLabelBuffer, label.Value().data(), sizeof(mOperationalStateLabelBuffer));
                operationalStateLabel.SetValue(CharSpan(mOperationalStateLabelBuffer, sizeof(mOperationalStateLabelBuffer)));
            }
            else
            {
                memcpy(mOperationalStateLabelBuffer, label.Value().data(), label.Value().size());
                operationalStateLabel.SetValue(CharSpan(mOperationalStateLabelBuffer, label.Value().size()));
            }
        }
        else
        {
            operationalStateLabel = NullOptional;
        }
    }

private:
    char mOperationalStateLabelBuffer[kOperationalStateLabelMaxSize];
};

/**
 * A class which represents the operational error of an Operational State cluster derivation instance.
 */
struct GenericOperationalError : public app::Clusters::detail::Structs::ErrorStateStruct::Type
{
    GenericOperationalError(uint8_t state, Optional<chip::CharSpan> label = NullOptional,
                            Optional<chip::CharSpan> details = NullOptional)
    {
        Set(state, label, details);
    }

    GenericOperationalError(const GenericOperationalError & error) { *this = error; }

    GenericOperationalError & operator=(const GenericOperationalError & error)
    {
        Set(error.errorStateID, error.errorStateLabel, error.errorStateDetails);
        return *this;
    }

    void Set(uint8_t state, Optional<chip::CharSpan> label = NullOptional, Optional<chip::CharSpan> details = NullOptional)
    {
        errorStateID = state;
        if (label.HasValue())
        {
            memset(mErrorStateLabelBuffer, 0, sizeof(mErrorStateLabelBuffer));
            if (label.Value().size() > sizeof(mErrorStateLabelBuffer))
            {
                memcpy(mErrorStateLabelBuffer, label.Value().data(), sizeof(mErrorStateLabelBuffer));
                errorStateLabel.SetValue(CharSpan(mErrorStateLabelBuffer, sizeof(mErrorStateLabelBuffer)));
            }
            else
            {
                memcpy(mErrorStateLabelBuffer, label.Value().data(), label.Value().size());
                errorStateLabel.SetValue(CharSpan(mErrorStateLabelBuffer, label.Value().size()));
            }
        }
        else
        {
            errorStateLabel = NullOptional;
        }

        if (details.HasValue())
        {
            memset(mErrorStateDetailsBuffer, 0, sizeof(mErrorStateDetailsBuffer));
            if (details.Value().size() > sizeof(mErrorStateDetailsBuffer))
            {
                memcpy(mErrorStateDetailsBuffer, details.Value().data(), sizeof(mErrorStateDetailsBuffer));
                errorStateDetails.SetValue(CharSpan(mErrorStateDetailsBuffer, sizeof(mErrorStateDetailsBuffer)));
            }
            else
            {
                memcpy(mErrorStateDetailsBuffer, details.Value().data(), details.Value().size());
                errorStateDetails.SetValue(CharSpan(mErrorStateDetailsBuffer, details.Value().size()));
            }
        }
        else
        {
            errorStateDetails = NullOptional;
        }
    }

    bool IsEqual(const Structs::ErrorStateStruct::Type & rhs) const
    {
        if (errorStateID != rhs.errorStateID)
        {
            return false;
        }
        if (errorStateLabel.HasValue() != rhs.errorStateLabel.HasValue() ||
            errorStateDetails.HasValue() != rhs.errorStateDetails.HasValue())
        {
            return false;
        }
        if (errorStateLabel.HasValue())
        {
            if (!errorStateLabel.Value().data_equal(rhs.errorStateLabel.Value()))
            {
                return false;
            }
        }
        if (errorStateDetails.HasValue())
        {
            if (!errorStateDetails.Value().data_equal(rhs.errorStateDetails.Value()))
            {
                return false;
            }
        }

        return true;
    }

private:
    char mErrorStateLabelBuffer[kOperationalErrorLabelMaxSize];
    char mErrorStateDetailsBuffer[kOperationalErrorDetailsMaxSize];
};

/**
 * A class which represents the operational error event of an Operational State cluster derivation instance.
 */
class GenericErrorEvent : private app::Clusters::OperationalState::Events::OperationalError::Type
{
    using super = app::Clusters::OperationalState::Events::OperationalError::Type;

public:
    GenericErrorEvent(ClusterId aClusterId, const Structs::ErrorStateStruct::Type & aError) : mClusterId(aClusterId)
    {
        errorState = aError;
    }
    using super::GetEventId;
    using super::GetPriorityLevel;
    ClusterId GetClusterId() const { return mClusterId; }
    using super::Encode;
    using super::kIsFabricScoped;

private:
    ClusterId mClusterId;
};

/**
 * A class which represents the operational completion event of an Operational State cluster derivation instance.
 */
class GenericOperationCompletionEvent : private app::Clusters::OperationalState::Events::OperationCompletion::Type
{
    using super = app::Clusters::OperationalState::Events::OperationCompletion::Type;

public:
    GenericOperationCompletionEvent(ClusterId aClusterId, uint8_t aCompletionErrorCode,
                                    const Optional<DataModel::Nullable<uint32_t>> & aTotalOperationalTime = NullOptional,
                                    const Optional<DataModel::Nullable<uint32_t>> & aPausedTime           = NullOptional) :
        mClusterId(aClusterId)
    {
        completionErrorCode  = aCompletionErrorCode;
        totalOperationalTime = aTotalOperationalTime;
        pausedTime           = aPausedTime;
    }
    using super::GetEventId;
    using super::GetPriorityLevel;
    ClusterId GetClusterId() const { return mClusterId; }
    using super::Encode;
    using super::kIsFabricScoped;

private:
    ClusterId mClusterId;
};

} // namespace OperationalState
} // namespace Clusters
} // namespace app
} // namespace chip
