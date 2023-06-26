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
#include <protocols/interaction_model/StatusCode.h>

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
    GenericOperationalState(uint8_t state, Optional<CharSpan> label = NullOptional)
    {
        Set(state, label);
    }

    GenericOperationalState(const GenericOperationalState & op)
    {
        *this = op;
    }

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
 * A class which holds the operational state list of an Operational State cluster derivation.
 */
struct GenericOperationalStateList : public GenericOperationalState
{
    GenericOperationalStateList(uint8_t state, Optional<CharSpan> label = NullOptional) :
        GenericOperationalState(state, label)
    {}
    GenericOperationalStateList * next = nullptr;
};

/**
 * A class which represents the operational error of an Operational State cluster derivation instance.
 */
struct GenericOperationalError : public app::Clusters::detail::Structs::ErrorStateStruct::Type
{
    GenericOperationalError(uint8_t state, Optional<chip::CharSpan> label = NullOptional, Optional<chip::CharSpan> details = NullOptional)
    {
        Set(state, label, details);
    }

    GenericOperationalError(const GenericOperationalError & error)
    {
        *this = error;
    }

    GenericOperationalError & operator=(const GenericOperationalError & error)
    {
        Set(error.errorStateID, error.errorStateLabel,
            error.errorStateDetails);
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
 * A class presents the phase of operational state cluster
 */
struct GenericOperationalPhase
{
    char PhaseName[kOperationalPhaseNameMaxSize];
    app::DataModel::Nullable<CharSpan> phaseName;

    GenericOperationalPhase(const char * name = nullptr, size_t nameLen = 0) { set(name, nameLen); }

    GenericOperationalPhase(const GenericOperationalPhase & ph)
    {
        if (ph.isNullable())
        {
            set();
        }
        else
        {
            set(ph.PhaseName, sizeof(ph.PhaseName));
        }
    }

    GenericOperationalPhase & operator=(const GenericOperationalPhase & ph)
    {
        if (ph.isNullable())
        {
            set();
        }
        else
        {
            set(ph.PhaseName, sizeof(ph.PhaseName));
        }
        return *this;
    }

    void set(const char * name = nullptr, size_t nameLen = 0)
    {
        if (name == nullptr)
        {
            phaseName.SetNull();
        }
        else
        {
            memset(PhaseName, 0, sizeof(PhaseName));
            if (nameLen > kOperationalPhaseNameMaxSize)
            {
                memcpy(PhaseName, name, kOperationalPhaseNameMaxSize);
            }
            else
            {
                memcpy(PhaseName, name, nameLen);
            }
            phaseName = app::DataModel::Nullable<CharSpan>(CharSpan(PhaseName, sizeof(PhaseName)));
        }
    }
    bool isNullable() const { return phaseName.IsNull(); }
};

/**
 * A class hold the phase list of operational state cluster
 */
struct GenericOperationalPhaseList : public GenericOperationalPhase
{
    GenericOperationalPhaseList(const char * name = nullptr, size_t nameLen = 0) : GenericOperationalPhase(name, nameLen) {}
    GenericOperationalPhaseList * next = nullptr;
};

/**
 * A class wrap the operation completion of operational state cluster
 */
struct GenericOperationCompletion : public app::Clusters::OperationalState::Events::OperationCompletion::Type
{
    GenericOperationCompletion(uint8_t aCompletionErrorCode)
    {

        app::DataModel::Nullable<uint32_t> __totalOperationalTime;
        app::DataModel::Nullable<uint32_t> __pausedTime;

        completionErrorCode = aCompletionErrorCode;

        __totalOperationalTime.SetNull();
        totalOperationalTime.SetValue(__totalOperationalTime);

        __pausedTime.SetNull();
        pausedTime.SetValue(__pausedTime);
    }

    GenericOperationCompletion(uint8_t aCompletionErrorCode, uint32_t aTotalOperationalTime)
    {
        app::DataModel::Nullable<uint32_t> __pausedTime;

        completionErrorCode = aCompletionErrorCode;
        totalOperationalTime.SetValue(app::DataModel::Nullable<uint32_t>(aTotalOperationalTime));
        __pausedTime.SetNull();
        pausedTime.SetValue(__pausedTime);
    }

    GenericOperationCompletion(uint8_t aCompletionErrorCode, uint32_t aTotalOperationalTime, uint32_t aPausedTime)
    {
        completionErrorCode = aCompletionErrorCode;
        totalOperationalTime.SetValue(app::DataModel::Nullable<uint32_t>(aTotalOperationalTime));
        pausedTime.SetValue(app::DataModel::Nullable<uint32_t>(aPausedTime));
    }
};

/**
 * A delegate to handle application logic of the Operational State aliased Cluster.
 */
class Delegate
{
public:
    /**
     * Get operational state.
     * @param void.
     * @return the const reference of operational state.
     */
    virtual const GenericOperationalState & GetOperationalState() const = 0;

    /**
     * Get operational state list.
     * @param operationalStateList The pointer to operational state list.
     * After a successful return the caller is responsible for calling ReleaseOperationalStateList on the outparam.
     */
    virtual CHIP_ERROR GetOperationalStateList(GenericOperationalStateList ** operationalStateList, size_t & size) = 0;

    /**
     * Release operational state list
     * @param operationalStateList The pointer for which to clear the OperationalStateStructDynamicList.
     * @return void
     */
    virtual void ReleaseOperationalStateList(GenericOperationalStateList * operationalStateList) = 0;

    /**
     * Get operational phase list.
     * @param operationalPhaseList The pointer to operational phase list.
     * After a successful return the caller is responsible for calling ReleaseOperationalPhaseList on the outparam.
     */
    virtual CHIP_ERROR GetOperationalPhaseList(GenericOperationalPhaseList ** operationalPhaseList, size_t & size) = 0;

    /**
     * Release operational phase list
     * @param operationalStateList The pointer for which to clear the GenericOperationalPhaseList.
     * @return void
     */
    virtual void ReleaseOperationalPhaseList(GenericOperationalPhaseList * operationalPhaseList) = 0;

    /**
     * Get operational error.
     * @param void.
     * @return the const reference of operational error.
     */
    virtual const GenericOperationalError GetOperationalError() const = 0;

    /**
     * Set operational state.
     * @param opState The operational state for which to set.
     * @return CHIP_ERROR CHIP_NO_ERROR on success, or corresponding error code.
     */
    virtual CHIP_ERROR SetOperationalState(const GenericOperationalState & opState) = 0;

    /**
     * Set operational error.
     * @param opErrState The reference of operational error.
     * @return CHIP_ERROR CHIP_NO_ERROR on success, or corresponding error code.
     */
    virtual CHIP_ERROR SetOperationalError(const GenericOperationalError & opErrState) = 0;

    // command callback
    /**
     * Handle Command Callback in application: Pause
     * @param[out] get operational error after callback.
     * @return operational error after callback
     */
    virtual GenericOperationalError & HandlePauseStateCallback(GenericOperationalError & err) = 0;

    /**
     * Handle Command Callback in application: Resume
     * @param[out] get operational error after callback.
     * @return operational error after callback
     */
    virtual GenericOperationalError & HandleResumeStateCallback(GenericOperationalError & err) = 0;

    /**
     * Handle Command Callback in application: Start
     * @param[out] get operational error after callback.
     * @return operational error after callback
     */
    virtual GenericOperationalError & HandleStartStateCallback(GenericOperationalError & err) = 0;

    /**
     * Handle Command Callback in application: Stop
     * @param[out] get operational error after callback.
     * @return operational error after callback
     */
    virtual GenericOperationalError & HandleStopStateCallback(GenericOperationalError & err) = 0;

    /**
     * Send OperationalError Event
     * @param[in] set the operational error to event.
     * @return true: send event success; fail : send event fail.
     */
    virtual bool sendOperationalErrorEvent(const GenericOperationalError & err) = 0;

    /**
     * Send OperationCompletion Event
     * @param[in] set the operation comletion to event.
     * @return true: send event success; fail : send event fail.
     */
    virtual bool sendOperationCompletion(const GenericOperationCompletion & op) = 0;

    Delegate(EndpointId aEndpointId, ClusterId aClusterId) : mEndpointId(aEndpointId), mClusterId(aClusterId) {}

    virtual ~Delegate() = default;

protected:
    EndpointId mEndpointId;
    ClusterId mClusterId;
};

Delegate * GetOperationalStateDelegate(EndpointId endpointId, ClusterId clusterId);

} // namespace OperationalState
} // namespace Clusters
} // namespace app
} // namespace chip
