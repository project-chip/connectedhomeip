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
#include <app-common/zap-generated/attributes/Accessors.h>
#include <app/EventLogging.h>
#include <app/util/config.h>
#include <operational-state-delegate-impl.h>

namespace chip {
namespace app {
namespace Clusters {
namespace OperationalState {

using chip::Protocols::InteractionModel::Status;

constexpr const char * kWasherPreSoak = "pre-soak";
constexpr const char * kWasherRinse   = "rinse";
constexpr const char * kWasherSpin    = "spin";
/**
 * Template class to present Enquriy Table
 */
template <typename T>
struct EnquiryTable
{
    /**
     * Endpoint Id
     */
    EndpointId mEndpointId;
    /**
     * Cluster Id
     */
    ClusterId mClusterId;
    /**
     * point of Array(Items)
     */
    T * pItems;
    /**
     * ArraySize of Array(Items)
     */
    size_t numOfItems;
};

/**
 * Enquriy Table of Phase List
 * Note: User Define
 */
GenericOperationalPhase opPhaseList[] = {
    /**
     * Phase List is null
     */
    GenericOperationalPhase(DataModel::Nullable<CharSpan>()),
    /**
     * Phase List isn't null
     */
    // GenericOperationalPhase(kWasherPreSoak, strlen(kWasherPreSoak)),
    // GenericOperationalPhase(kWasherRinse, strlen(kWasherRinse)),
    // GenericOperationalPhase(kWasherSpin, strlen(kWasherSpin)),
};

/**
 * Enquriy Table of Phase List corresponding to endpointId and clusterId
 * Note: User Define
 */
constexpr EnquiryTable<GenericOperationalPhase> kPhaseListEnquiryTable[] = {
    // EndpointId, ClusterId, Array of phaseList, ArraySize of phaseList
    { 1, Clusters::OperationalState::Id, opPhaseList, ArraySize(opPhaseList) },
};

/**
 * Enquriy Table of Operational State List
 * Note: User Define
 */
GenericOperationalState opStateList[] = {
    GenericOperationalState(to_underlying(OperationalStateEnum::kStopped)),
    GenericOperationalState(to_underlying(OperationalStateEnum::kRunning)),
    GenericOperationalState(to_underlying(OperationalStateEnum::kPaused)),
    GenericOperationalState(to_underlying(OperationalStateEnum::kError)),
};

/**
 * Enquriy Table of Operational State List corresponding to endpointId and clusterId
 * Note: User Define
 */
constexpr EnquiryTable<GenericOperationalState> kOpStateListEnquiryTable[] = {
    // EndpointId, ClusterId, Array of Operational State List, ArraySize of Operational State List
    { 1, Clusters::OperationalState::Id, opStateList, ArraySize(opStateList) },
};

/**
 * Get the pointer of target Array(PhaseList) for target endpoint and cluster
 * @param[in] aEndpiontId The endpointId
 * @param[in] aClusterID  The clusterId
 * @param[out] size  The ArraySize of target Array(PhaseList)
 * @return the pointer of target Array(PhaseList)
 */
const GenericOperationalPhase * getGenericPhaseListTable(EndpointId aEndpointId, ClusterId aClusterId, size_t & size)
{
    for (size_t i = 0; i < ArraySize(kPhaseListEnquiryTable); ++i)
    {
        if (kPhaseListEnquiryTable[i].mEndpointId == aEndpointId && kPhaseListEnquiryTable[i].mClusterId == aClusterId)
        {
            size = kPhaseListEnquiryTable[i].numOfItems;
            return kPhaseListEnquiryTable[i].pItems;
        }
    }
    size = 0;
    return nullptr;
}

/**
 * Get the pointer of target Array(Operational State) for target endpoint and cluster
 * @param[in] aEndpiontId The endpointId
 * @param[in] aClusterID  The clusterId
 * @param[out] size  The ArraySize of target Array(Operational State)
 * @return the pointer of target Array(Operational State)
 */
const GenericOperationalState * getGenericOperationalStateTable(EndpointId aEndpointId, ClusterId aClusterId,
                                                                size_t & size)
{
    for (size_t i = 0; i < ArraySize(kOpStateListEnquiryTable); ++i)
    {
        if (kOpStateListEnquiryTable[i].mEndpointId == aEndpointId && kOpStateListEnquiryTable[i].mClusterId == aClusterId)
        {
            size = kOpStateListEnquiryTable[i].numOfItems;
            return kOpStateListEnquiryTable[i].pItems;
        }
    }
    size = 0;
    return nullptr;
}

CHIP_ERROR OperationalStateDelegate::SetOperationalState(const GenericOperationalState & opState)
{
    mOperationalState = opState;
    return CHIP_NO_ERROR;
}

void OperationalStateDelegate::GetOperationalState(GenericOperationalState & op)
{
    op = mOperationalState;
}

CHIP_ERROR OperationalStateDelegate::SetOperationalError(const GenericOperationalError & opErrState)
{
    mOperationalError = opErrState;
    return CHIP_NO_ERROR;
}

const GenericOperationalError OperationalStateDelegate::GetOperationalError() const
{
    return mOperationalError;
}

CHIP_ERROR OperationalStateDelegate::GetOperationalStateAtIndex(size_t index, GenericOperationalState & operationalState)
{
    size_t opStateListNumOfItems       = 0;

    const GenericOperationalState * src = getGenericOperationalStateTable(mEndpointId, mClusterId, opStateListNumOfItems);
    if (!src || !opStateListNumOfItems)
    {
        ChipLogError(Zcl, "Unable to find Operational State List for [ep=%d],[cid=%d]", mEndpointId, mClusterId);
        return CHIP_ERROR_NOT_FOUND;
    }

    if (index > opStateListNumOfItems - 1)
    {
        return CHIP_ERROR_NOT_FOUND;
    }
    else
    {
        operationalState = src[index];
    }
    return CHIP_NO_ERROR;
}

CHIP_ERROR OperationalStateDelegate::GetOperationalPhaseList(GenericOperationalPhaseList ** operationalPhaseList, size_t & size)
{
    CHIP_ERROR err                     = CHIP_ERROR_NO_MEMORY;
    size                               = 0;
    size_t i                           = 0;
    size_t phaseListNumOfItems         = 0;
    GenericOperationalPhaseList * head = nullptr;

    if (!operationalPhaseList)
    {
        return CHIP_ERROR_INVALID_ARGUMENT;
    }

    const GenericOperationalPhase * src = getGenericPhaseListTable(mEndpointId, mClusterId, phaseListNumOfItems);
    if (!src || !phaseListNumOfItems)
    {
        ChipLogError(Zcl, "Unable to find Phase List for [ep=%d],[cid=%d]", mEndpointId, mClusterId);
        return CHIP_ERROR_INVALID_ARGUMENT;
    }

    for (i = 0; i < phaseListNumOfItems; i++)
    {
        GenericOperationalPhaseList * des = Platform::New<GenericOperationalPhaseList>(src->mPhaseName);
        if (des == nullptr)
        {
            err = CHIP_ERROR_NO_MEMORY;
            ExitNow();
        }

        if (head == nullptr)
        {
            head = des;
        }
        else
        {
            GenericOperationalPhaseList * pList = head;
            while (pList->next != nullptr)
            {
                pList = pList->next;
            }
            pList->next = des;
        }
        src++;
    }
    size                  = i;
    *operationalPhaseList = head;
    return CHIP_NO_ERROR;
exit:
    ReleaseOperationalPhaseList(head);
    return err;
}

void OperationalStateDelegate::ReleaseOperationalPhaseList(GenericOperationalPhaseList * operationalPhaseList)
{
    while (operationalPhaseList)
    {
        GenericOperationalPhaseList * del = operationalPhaseList;
        operationalPhaseList              = operationalPhaseList->next;
        Platform::Delete(del);
    }
}

GenericOperationalError & OperationalStateDelegate::HandlePauseStateCallback(GenericOperationalError & err)
{
    if (mOperationalState.operationalStateID == to_underlying(ManufactureOperationalStateEnum::kChildSafetyLock))
    {
        err.Set(to_underlying(ErrorStateEnum::kCommandInvalidInState));
    }
    else
    {
        mOperationalState.Set(to_underlying(OperationalStateEnum::kPaused));
        err.Set(to_underlying(ErrorStateEnum::kNoError));
    }
    return err;
}

GenericOperationalError & OperationalStateDelegate::HandleResumeStateCallback(GenericOperationalError & err)
{
    if (mOperationalState.operationalStateID == to_underlying(ManufactureOperationalStateEnum::kChildSafetyLock))
    {
        err.Set(to_underlying(ErrorStateEnum::kUnableToStartOrResume));
    }
    else
    {
        mOperationalState.Set(to_underlying(OperationalStateEnum::kRunning));
        err.Set(to_underlying(ErrorStateEnum::kNoError));
    }
    return err;
}

GenericOperationalError & OperationalStateDelegate::HandleStartStateCallback(GenericOperationalError & err)
{
    if (mOperationalState.operationalStateID == to_underlying(ManufactureOperationalStateEnum::kChildSafetyLock))
    {
        err.Set(to_underlying(ErrorStateEnum::kUnableToStartOrResume));
    }
    else
    {
        mOperationalState.Set(to_underlying(OperationalStateEnum::kRunning));
        err.Set(to_underlying(ErrorStateEnum::kNoError));
    }
    return err;
}

GenericOperationalError & OperationalStateDelegate::HandleStopStateCallback(GenericOperationalError & err)
{
    if (mOperationalState.operationalStateID == to_underlying(ManufactureOperationalStateEnum::kChildSafetyLock))
    {
        err.Set(to_underlying(ErrorStateEnum::kCommandInvalidInState));
    }
    else
    {
        mOperationalState.Set(to_underlying(OperationalStateEnum::kStopped));
        err.Set(to_underlying(ErrorStateEnum::kNoError));
    }
    return err;
}

bool OperationalStateDelegate::sendOperationalErrorEvent(const GenericOperationalError & err)
{
    Events::OperationalError::Type event;
    EventNumber eventNumber;

    event.errorState = err;
    CHIP_ERROR error = app::LogEvent(event, mEndpointId, eventNumber);

    if (error != CHIP_NO_ERROR)
    {
        return false;
    }

    // set OperationalState attribute to Error
    mOperationalState.Set(to_underlying(OperationalStateEnum::kError));
    return true;
}

bool OperationalStateDelegate::sendOperationCompletion(const GenericOperationCompletion & op)
{
    Events::OperationCompletion::Type event;
    EventNumber eventNumber;
    event = op;

    CHIP_ERROR error = app::LogEvent(event, mEndpointId, eventNumber);

    if (error != CHIP_NO_ERROR)
    {
        return false;
    }

    return true;
}

} // namespace OperationalState
} // namespace Clusters
} // namespace app
} // namespace chip
