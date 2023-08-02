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
#include <app/util/af-enums.h>
#include <lib/support/CommonIterator.h>

namespace chip {
namespace app {
namespace Clusters {
namespace OperationalState {

constexpr size_t kOperationalStateLabelMaxSize   = 64u;
constexpr size_t kOperationalErrorLabelMaxSize   = 64u;
constexpr size_t kOperationalErrorDetailsMaxSize = 64u;
constexpr size_t kOperationalPhaseNameMaxSize    = 64u;

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

private:
    char mErrorStateLabelBuffer[kOperationalErrorLabelMaxSize];
    char mErrorStateDetailsBuffer[kOperationalErrorDetailsMaxSize];
};

/**
 * A class which represents the operational phase of an Operational State cluster derivation instance.
 */
struct GenericOperationalPhase
{
    GenericOperationalPhase(app::DataModel::Nullable<CharSpan> name) { Set(name); }

    GenericOperationalPhase(const GenericOperationalPhase & ph) { *this = ph; }

    GenericOperationalPhase & operator=(const GenericOperationalPhase & ph)
    {
        Set(ph.mPhaseName);
        return *this;
    }

    bool IsMissing() const { return mPhaseName.IsNull(); }
    app::DataModel::Nullable<CharSpan> mPhaseName;

private:
    void Set(app::DataModel::Nullable<CharSpan> name)
    {
        if (name.IsNull())
        {
            mPhaseName.SetNull();
        }
        else
        {
            memset(mPhaseNameBuffer, 0, sizeof(mPhaseNameBuffer));
            if (name.Value().size() > sizeof(mPhaseNameBuffer))
            {
                memcpy(mPhaseNameBuffer, name.Value().data(), sizeof(mPhaseNameBuffer));
                mPhaseName = app::DataModel::Nullable<CharSpan>(CharSpan(mPhaseNameBuffer, sizeof(mPhaseNameBuffer)));
            }
            else
            {
                memcpy(mPhaseNameBuffer, name.Value().data(), name.Value().size());
                mPhaseName = app::DataModel::Nullable<CharSpan>(CharSpan(mPhaseNameBuffer, name.Value().size()));
            }
        }
    }

    char mPhaseNameBuffer[kOperationalPhaseNameMaxSize];
};

/**
 * A delegate to handle application logic of the Operational State aliased Cluster.
 * The delegate API assumes there will be separate delegate objects for each cluster instance.
 * (i.e. each separate operational state cluster derivation, on each separate endpoint),
 * since the delegate methods are not handed the cluster id or endpoint.
 */
class Delegate
{
public:
    /**
     * Get the current operational state.
     * @return The current operational state value
     */
    virtual uint8_t GetCurrentOperationalState() = 0;

    /**
     * Get the list of supported operational states.
     * Fills in the provided GenericOperationalState with the state at index `index` if there is one,
     * or returns CHIP_ERROR_NOT_FOUND if the index is out of range for the list of states.
     * @param index The index of the state, with 0 representing the first state.
     * @param operationalState  The GenericOperationalState is filled.
     */
    virtual CHIP_ERROR GetOperationalStateAtIndex(size_t index, GenericOperationalState & operationalState) = 0;

    /**
     * Get the list of supported operational phases.
     * Fills in the provided GenericOperationalPhase with the phase at index `index` if there is one,
     * or returns CHIP_ERROR_NOT_FOUND if the index is out of range for the list of phases.
     * @param index The index of the phase, with 0 representing the first phase.
     * @param operationalPhase  The GenericOperationalPhase is filled.
     */
    virtual CHIP_ERROR GetOperationalPhaseAtIndex(size_t index, GenericOperationalPhase & operationalPhase) = 0;

    /**
     * Get current operational error.
     * @param error The GenericOperationalError to fill with the current operational error value
     */
    virtual void GetCurrentOperationalError(GenericOperationalError & error) = 0;

    /**
     * Get current phase
     * @param phase The app::DataModel::Nullable<uint8_t> to fill with the current phase value
     */
    virtual void GetCurrentPhase(app::DataModel::Nullable<uint8_t> & phase) = 0;

    /**
     * Get countdown time
     * @param time The app::DataModel::Nullable<uint32_t> to fill with the coutdown time value
     */
    virtual void GetCountdownTime(app::DataModel::Nullable<uint32_t> & time) = 0;

    /**
     * Set current operational state.
     * @param opState The operational state that should now be the current one.
     */
    virtual CHIP_ERROR SetOperationalState(uint8_t opState) = 0;

    /**
     * Set operational error.
     * @param opErrState The new operational error.
     */
    virtual CHIP_ERROR SetOperationalError(const GenericOperationalError & opErrState) = 0;

    /**
     * Set operational phase.
     * @param phase The operational phase that should now be the current one.
     */
    virtual CHIP_ERROR SetPhase(const app::DataModel::Nullable<uint8_t> & phase) = 0;

    /**
     * Set coutdown time.
     * @param time The coutdown time that should now be the current one.
     */
    virtual CHIP_ERROR SetCountdownTime(const app::DataModel::Nullable<uint32_t> & time) = 0;

    // command callback
    /**
     * Handle Command Callback in application: Pause
     * @param[out] get operational error after callback.
     */
    virtual void HandlePauseStateCallback(GenericOperationalError & err) = 0;

    /**
     * Handle Command Callback in application: Resume
     * @param[out] get operational error after callback.
     */
    virtual void HandleResumeStateCallback(GenericOperationalError & err) = 0;

    /**
     * Handle Command Callback in application: Start
     * @param[out] get operational error after callback.
     */
    virtual void HandleStartStateCallback(GenericOperationalError & err) = 0;

    /**
     * Handle Command Callback in application: Stop
     * @param[out] get operational error after callback.
     */
    virtual void HandleStopStateCallback(GenericOperationalError & err) = 0;

    Delegate() = default;

    virtual ~Delegate() = default;
};

// @brief Instance getter for the delegate for the given operational state alias cluster on the given endpoint.
// The delegate API assumes there will be separate delegate objects for each cluster instance.
// (i.e. each separate operational state cluster derivation, on each separate endpoint)
// @note This API should always be called prior to using the delegate and the return pointer should never be cached.
//   This should be implemented by the application.
// @return Default global delegate instance.
Delegate * GetOperationalStateDelegate(EndpointId endpointId, ClusterId clusterId);

} // namespace OperationalState
} // namespace Clusters
} // namespace app
} // namespace chip
