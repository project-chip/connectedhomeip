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
#include <protocols/interaction_model/StatusCode.h>
#include <lib/support/CommonIterator.h>

namespace chip {
namespace app {
namespace Clusters {
namespace OperationalState {

constexpr size_t kOperationalStateLabelMaxSize      = 64u;
constexpr size_t kOperationalErrorLabelMaxSize = 64u;
constexpr size_t kOperationalErrorDetailsMaxSize = 64u;
constexpr size_t kOperationalPhaseNameMaxSize = 64u;

/**
 * A class wrap the operational state of operational state cluster
 */
struct GenericOperationalState : public chip::app::Clusters::detail::Structs::OperationalStateStruct::Type
{
    GenericOperationalState(uint8_t state, const char * label = nullptr, size_t labelLen = 0)
    {
        set(state, label, labelLen);
    }

    GenericOperationalState(const GenericOperationalState & op)
    {
        set(op.operationalStateID,
                op.operationalStateLabel.HasValue() ? op.OperationalStateLabel : nullptr,
                op.operationalStateLabel.HasValue() ? sizeof(op.OperationalStateLabel) : 0);

    }

    GenericOperationalState& operator = (const GenericOperationalState& op)
    {
        set(op.operationalStateID,
                op.operationalStateLabel.HasValue() ? op.OperationalStateLabel : nullptr,
                op.operationalStateLabel.HasValue() ? sizeof(op.OperationalStateLabel) : 0);

        return *this;
    }

    void set(uint8_t state, const char * label = nullptr, size_t labelLen = 0)
    {
        operationalStateID = state;
        if (label == nullptr)
        {
           operationalStateLabel = Optional<chip::CharSpan>::Missing();
        }
        else
        {
            memset(OperationalStateLabel, 0, sizeof(OperationalStateLabel));
            if (labelLen > kOperationalStateLabelMaxSize)
            {
                memcpy(OperationalStateLabel, label, kOperationalStateLabelMaxSize);
            }
            else
            {
                memcpy(OperationalStateLabel, label, labelLen);
            }

            operationalStateLabel.SetValue(chip::CharSpan(OperationalStateLabel, sizeof(OperationalStateLabel)));
        }

    }
    uint8_t getStateID() const
    {
        return operationalStateID;
    }
    char OperationalStateLabel[kOperationalStateLabelMaxSize];
};

/**
 * A class hold the operational state list of operational state cluster
 */
struct GenericOperationalStateList : public GenericOperationalState
{
    GenericOperationalStateList(uint8_t state, const char * label = nullptr, size_t labelLen = 0) : GenericOperationalState(state, label, labelLen){}
    GenericOperationalStateList * next = nullptr;
};

/**
 * A class wrap the operational error of operational state cluster
 */
struct GenericOperationalError : public chip::app::Clusters::detail::Structs::ErrorStateStruct::Type
{
    GenericOperationalError(uint8_t state, const char * label = nullptr, size_t labelLen = 0, const char *details = nullptr, size_t detailsLen = 0)
    {
        set(state, label, labelLen, details, detailsLen);
    }

    GenericOperationalError(const GenericOperationalError & error)
    {
        set(error.errorStateID,
                error.errorStateLabel.HasValue() ? error.ErrorStateLabel : nullptr,
                error.errorStateLabel.HasValue() ? sizeof(error.ErrorStateLabel) : 0,
                error.errorStateDetails.HasValue() ? error.ErrorStateDetails : nullptr,
                error.errorStateDetails.HasValue() ? sizeof(error.ErrorStateDetails) : 0);
    }

    GenericOperationalError& operator = (const GenericOperationalError& error)
    {
        set(error.errorStateID,
                error.errorStateLabel.HasValue() ? error.ErrorStateLabel : nullptr,
                error.errorStateLabel.HasValue() ? sizeof(error.ErrorStateLabel) : 0,
                error.errorStateDetails.HasValue() ? error.ErrorStateDetails : nullptr,
                error.errorStateDetails.HasValue() ? sizeof(error.ErrorStateDetails) : 0);
        return *this;
    }

    void set(uint8_t state, const char * label = nullptr, size_t labelLen = 0, const char *details = nullptr, size_t detailsLen = 0)
    {
        errorStateID = state;
        if (label == nullptr)
        {
           errorStateLabel = Optional<chip::CharSpan>::Missing();
        }
        else
        {
            memset(ErrorStateLabel, 0, sizeof(ErrorStateLabel));
            if (labelLen > kOperationalErrorLabelMaxSize)
            {
                memcpy(ErrorStateLabel, label, kOperationalErrorLabelMaxSize);
            }
            else
            {
                memcpy(ErrorStateLabel, label, labelLen);
            }

            errorStateLabel.SetValue(chip::CharSpan(ErrorStateLabel, sizeof(ErrorStateLabel)));
        }

        if (details == nullptr)
        {
           errorStateDetails = Optional<chip::CharSpan>::Missing();
        }
        else
        {
            memset(ErrorStateDetails, 0, sizeof(ErrorStateDetails));
            if (labelLen > kOperationalErrorDetailsMaxSize)
            {
                memcpy(ErrorStateDetails, details, kOperationalErrorDetailsMaxSize);
            }
            else
            {
                memcpy(ErrorStateDetails, details, detailsLen);
            }

            errorStateDetails.SetValue(chip::CharSpan(ErrorStateDetails, sizeof(ErrorStateDetails)));
        }

    }
    uint8_t getStateID() const
    {
        return errorStateID;
    }
    char ErrorStateLabel[kOperationalErrorLabelMaxSize];
    char ErrorStateDetails[kOperationalErrorDetailsMaxSize];
};

/**
 * A class presents the phase of operational state cluster
 */
struct GenericOperationalPhase
{
    char PhaseName[kOperationalPhaseNameMaxSize];
    chip::app::DataModel::Nullable<chip::CharSpan> phaseName;

    GenericOperationalPhase(const char *name = nullptr, size_t nameLen = 0)
    {
        set(name, nameLen);
    }

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

    GenericOperationalPhase& operator = (const GenericOperationalPhase& ph)
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

    void set(const char *name = nullptr, size_t nameLen = 0)
    {
        if(name == nullptr)
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
            phaseName = chip::app::DataModel::Nullable<chip::CharSpan>(chip::CharSpan(PhaseName, sizeof(PhaseName)));
        }
    }
    bool isNullable() const
    {
        return phaseName.IsNull();
    }
};

/**
 * A class hold the phase list of operational state cluster
 */
struct GenericOperationalPhaseList : public GenericOperationalPhase
{
    GenericOperationalPhaseList(const char *name = nullptr, size_t nameLen = 0) : GenericOperationalPhase(name, nameLen){}
    GenericOperationalPhaseList * next = nullptr;
};

/**
 * A class wrap the operation completion of operational state cluster
 */
struct GenericOperationCompletion : public chip::app::Clusters::OperationalState::Events::OperationCompletion::Type
{
    GenericOperationCompletion(uint8_t aCompletionErrorCode)
    {

        chip::app::DataModel::Nullable<uint32_t> __totalOperationalTime;
        chip::app::DataModel::Nullable<uint32_t> __pausedTime;

        completionErrorCode = aCompletionErrorCode;

        __totalOperationalTime.SetNull();
        totalOperationalTime.SetValue(__totalOperationalTime);

        __pausedTime.SetNull();
        pausedTime.SetValue(__pausedTime);
    }

    GenericOperationCompletion(uint8_t aCompletionErrorCode, uint32_t aTotalOperationalTime)
    {
        chip::app::DataModel::Nullable<uint32_t> __pausedTime;

        completionErrorCode = aCompletionErrorCode;
        totalOperationalTime.SetValue(chip::app::DataModel::Nullable<uint32_t>(aTotalOperationalTime));
        __pausedTime.SetNull();
        pausedTime.SetValue(__pausedTime);
    }

    GenericOperationCompletion(uint8_t aCompletionErrorCode, uint32_t aTotalOperationalTime, uint32_t aPausedTime)
    {
        completionErrorCode = aCompletionErrorCode;
        totalOperationalTime.SetValue(chip::app::DataModel::Nullable<uint32_t>(aTotalOperationalTime));
        pausedTime.SetValue(chip::app::DataModel::Nullable<uint32_t>(aPausedTime));
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

    //command callback
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
