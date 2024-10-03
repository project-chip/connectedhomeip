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

#include <app/AttributeAccessInterface.h>
#include <app/AttributeAccessInterfaceRegistry.h>
#include <app/CommandHandlerInterface.h>
#include <app/CommandHandlerInterfaceRegistry.h>
#include <app/ConcreteAttributePath.h>
#include <app/ConcreteClusterPath.h>
#include <app/InteractionModelEngine.h>
#include <app/SafeAttributePersistenceProvider.h>
#include <app/clusters/resource-monitoring-server/resource-monitoring-cluster-objects.h>
#include <app/clusters/resource-monitoring-server/resource-monitoring-server.h>
#include <app/data-model/Nullable.h>
#include <app/reporting/reporting.h>
#include <app/util/attribute-storage.h>
#include <lib/core/CHIPError.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/TypeTraits.h>
#include <protocols/interaction_model/StatusCode.h>

using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::ResourceMonitoring;
using chip::Protocols::InteractionModel::Status;

namespace chip {
namespace app {
namespace Clusters {
namespace ResourceMonitoring {

Instance::Instance(Delegate * aDelegate, EndpointId aEndpointId, ClusterId aClusterId, uint32_t aFeatureMap,
                   ResourceMonitoring::Attributes::DegradationDirection::TypeInfo::Type aDegradationDirection,
                   bool aResetConditionCommandSupported) :
    CommandHandlerInterface(Optional<EndpointId>(aEndpointId), aClusterId),
    AttributeAccessInterface(Optional<EndpointId>(aEndpointId), aClusterId), mDelegate(aDelegate), mEndpointId(aEndpointId),
    mClusterId(aClusterId), mDegradationDirection(aDegradationDirection), mFeatureMap(aFeatureMap),
    mResetConditionCommandSupported(aResetConditionCommandSupported)
{
    mDelegate->SetInstance(this);
};

Instance::~Instance()
{
    CommandHandlerInterfaceRegistry::Instance().UnregisterCommandHandler(this);
    AttributeAccessInterfaceRegistry::Instance().Unregister(this);
}

CHIP_ERROR Instance::Init()
{
    ChipLogDetail(Zcl, "ResourceMonitoring: Init");

    // Check if the cluster has been selected in zap
    VerifyOrDie(emberAfContainsServer(mEndpointId, mClusterId));

    LoadPersistentAttributes();

    ReturnErrorOnFailure(CommandHandlerInterfaceRegistry::Instance().RegisterCommandHandler(this));
    VerifyOrReturnError(AttributeAccessInterfaceRegistry::Instance().Register(this), CHIP_ERROR_INCORRECT_STATE);
    ChipLogDetail(Zcl, "ResourceMonitoring: calling mDelegate->Init()");
    ReturnErrorOnFailure(mDelegate->Init());

    return CHIP_NO_ERROR;
}

bool Instance::HasFeature(ResourceMonitoring::Feature aFeature) const
{
    return ((mFeatureMap & to_underlying(aFeature)) != 0);
}

chip::Protocols::InteractionModel::Status Instance::UpdateCondition(uint8_t aNewCondition)
{
    auto oldCondition = mCondition;
    mCondition        = aNewCondition;
    if (mCondition != oldCondition)
    {
        MatterReportingAttributeChangeCallback(mEndpointId, mClusterId, Attributes::Condition::Id);
    }
    return Protocols::InteractionModel::Status::Success;
}

chip::Protocols::InteractionModel::Status
Instance::UpdateChangeIndication(chip::app::Clusters::ResourceMonitoring::ChangeIndicationEnum aNewChangeIndication)
{
    if (aNewChangeIndication == chip::app::Clusters::ResourceMonitoring::ChangeIndicationEnum::kWarning)
    {
        if (!HasFeature(ResourceMonitoring::Feature::kWarning))
        {
            return Protocols::InteractionModel::Status::InvalidValue;
        }
    }
    auto oldChangeIndication = mChangeIndication;
    mChangeIndication        = aNewChangeIndication;
    if (mChangeIndication != oldChangeIndication)
    {
        MatterReportingAttributeChangeCallback(mEndpointId, mClusterId, Attributes::ChangeIndication::Id);
    }
    return Protocols::InteractionModel::Status::Success;
}

chip::Protocols::InteractionModel::Status Instance::UpdateInPlaceIndicator(bool aNewInPlaceIndicator)
{
    auto oldInPlaceIndicator = mInPlaceIndicator;
    mInPlaceIndicator        = aNewInPlaceIndicator;
    if (mInPlaceIndicator != oldInPlaceIndicator)
    {
        MatterReportingAttributeChangeCallback(mEndpointId, mClusterId, Attributes::InPlaceIndicator::Id);
    }
    return Protocols::InteractionModel::Status::Success;
}

chip::Protocols::InteractionModel::Status Instance::UpdateLastChangedTime(DataModel::Nullable<uint32_t> aNewLastChangedTime)
{
    auto oldLastchangedTime = mLastChangedTime;
    mLastChangedTime        = aNewLastChangedTime;
    if (mLastChangedTime != oldLastchangedTime)
    {
        chip::app::GetSafeAttributePersistenceProvider()->WriteScalarValue(
            ConcreteAttributePath(mEndpointId, mClusterId, Attributes::LastChangedTime::Id), mLastChangedTime);
        MatterReportingAttributeChangeCallback(mEndpointId, mClusterId, Attributes::LastChangedTime::Id);
    }
    return Protocols::InteractionModel::Status::Success;
}

void Instance::SetReplacementProductListManagerInstance(ReplacementProductListManager * aReplacementProductListManager)
{
    mReplacementProductListManager = aReplacementProductListManager;
}

uint8_t Instance::GetCondition() const
{
    return mCondition;
}
chip::app::Clusters::ResourceMonitoring::ChangeIndicationEnum Instance::GetChangeIndication() const
{
    return mChangeIndication;
}

chip::app::Clusters::ResourceMonitoring::DegradationDirectionEnum Instance::GetDegradationDirection() const
{
    return mDegradationDirection;
}

bool Instance::GetInPlaceIndicator() const
{
    return mInPlaceIndicator;
}

DataModel::Nullable<uint32_t> Instance::GetLastChangedTime() const
{
    return mLastChangedTime;
}

ReplacementProductListManager * Instance::GetReplacementProductListManagerInstance()
{
    return mReplacementProductListManager;
}

// This method is called by the interaction model engine when a command destined for this instance is received.
void Instance::InvokeCommand(HandlerContext & handlerContext)
{
    ChipLogDetail(Zcl, "ResourceMonitoring Instance::InvokeCommand");
    switch (handlerContext.mRequestPath.mCommandId)
    {
    case ResourceMonitoring::Commands::ResetCondition::Id:
        ChipLogDetail(Zcl, "ResourceMonitoring::Commands::ResetCondition");

        HandleCommand<ResourceMonitoring::Commands::ResetCondition::DecodableType>(
            handlerContext, [this](HandlerContext & ctx, const auto & commandData) { HandleResetCondition(ctx, commandData); });
        break;
    }
}

// List the commands supported by this instance.
CHIP_ERROR Instance::EnumerateAcceptedCommands(const ConcreteClusterPath & cluster,
                                               CommandHandlerInterface::CommandIdCallback callback, void * context)
{
    ChipLogDetail(Zcl, "resourcemonitoring: EnumerateAcceptedCommands");
    if (mResetConditionCommandSupported)
    {
        callback(ResourceMonitoring::Commands::ResetCondition::Id, context);
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR Instance::ReadReplaceableProductList(AttributeValueEncoder & aEncoder)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    if (HasFeature(ResourceMonitoring::Feature::kReplacementProductList))
    {
        ReplacementProductListManager * productListManagerInstance = GetReplacementProductListManagerInstance();
        if (nullptr == productListManagerInstance)
        {
            aEncoder.EncodeEmptyList();
            return CHIP_NO_ERROR;
        }

        productListManagerInstance->Reset();

        err = aEncoder.EncodeList([productListManagerInstance](const auto & encoder) -> CHIP_ERROR {
            ReplacementProductStruct replacementProductStruct;
            CHIP_ERROR iteratorError = productListManagerInstance->Next(replacementProductStruct);

            while (CHIP_NO_ERROR == iteratorError)
            {
                ReturnErrorOnFailure(encoder.Encode(replacementProductStruct));
                iteratorError = productListManagerInstance->Next(replacementProductStruct);
            }
            return (CHIP_ERROR_PROVIDER_LIST_EXHAUSTED == iteratorError) ? CHIP_NO_ERROR : iteratorError;
        });
    }
    return err;
}

// Implements the read functionality for non-standard attributes.
CHIP_ERROR Instance::Read(const ConcreteReadAttributePath & aPath, AttributeValueEncoder & aEncoder)
{
    switch (aPath.mAttributeId)
    {
    case Attributes::Condition::Id: {
        ReturnErrorOnFailure(aEncoder.Encode(mCondition));
        break;
    }
    case Attributes::FeatureMap::Id: {
        ReturnErrorOnFailure(aEncoder.Encode(mFeatureMap));
        break;
    }
    case Attributes::DegradationDirection::Id: {
        ReturnErrorOnFailure(aEncoder.Encode(mDegradationDirection));
        break;
    }
    case Attributes::ChangeIndication::Id: {
        ReturnErrorOnFailure(aEncoder.Encode(mChangeIndication));
        break;
    }
    case Attributes::InPlaceIndicator::Id: {
        ReturnErrorOnFailure(aEncoder.Encode(mInPlaceIndicator));
        break;
    }
    case Attributes::LastChangedTime::Id: {
        ReturnErrorOnFailure(aEncoder.Encode(mLastChangedTime));
        break;
    }
    case Attributes::ReplacementProductList::Id: {
        return ReadReplaceableProductList(aEncoder);
        break;
    }
    }
    return CHIP_NO_ERROR;
}

// Implements checking before attribute writes.
CHIP_ERROR Instance::Write(const ConcreteDataAttributePath & aPath, AttributeValueDecoder & aDecoder)
{
    switch (aPath.mAttributeId)
    {
    case Attributes::LastChangedTime::Id: {
        DataModel::Nullable<uint32_t> newLastChangedTime;
        ReturnErrorOnFailure(aDecoder.Decode(newLastChangedTime));
        UpdateLastChangedTime(newLastChangedTime);
        break;
    }
    }
    return CHIP_NO_ERROR;
}

template <typename RequestT, typename FuncT>
void Instance::HandleCommand(HandlerContext & handlerContext, FuncT func)
{
    ChipLogDetail(Zcl, "ResourceMonitoring: HandleCommand");
    if (handlerContext.mCommandHandled || (handlerContext.mRequestPath.mCommandId != RequestT::GetCommandId()))
    {
        return;
    }

    RequestT requestPayload;

    // If the command matches what the caller is looking for, let's mark this as being handled
    // even if errors happen after this. This ensures that we don't execute any fall-back strategies
    // to handle this command since at this point, the caller is taking responsibility for handling
    // the command in its entirety, warts and all.
    handlerContext.SetCommandHandled();

    if (DataModel::Decode(handlerContext.mPayload, requestPayload) != CHIP_NO_ERROR)
    {
        handlerContext.mCommandHandler.AddStatus(handlerContext.mRequestPath, Protocols::InteractionModel::Status::InvalidCommand);
        return;
    }

    func(handlerContext, requestPayload);
}

void Instance::LoadPersistentAttributes()
{
    CHIP_ERROR err = chip::app::GetSafeAttributePersistenceProvider()->ReadScalarValue(
        ConcreteAttributePath(mEndpointId, mClusterId, Attributes::LastChangedTime::Id), mLastChangedTime);
    if (err == CHIP_NO_ERROR)
    {
        if (mLastChangedTime.IsNull())
        {
            ChipLogDetail(Zcl, "ResourceMonitoring: Loaded LastChangedTime as null");
        }
        else
        {
            ChipLogDetail(Zcl, "ResourceMonitoring: Loaded LastChangedTime as %lu",
                          (long unsigned int) mLastChangedTime.Value()); // on some platforms uint32_t is a long, cast it to
                                                                         // unsigned long on all platforms to prevent CI errors
        }
    }
    else
    {
        // If we cannot find the previous LastChangedTime, we will assume it to be null.
        ChipLogDetail(Zcl, "ResourceMonitoring: Unable to load the LastChangedTime from the KVS. Assuming null");
    }
}

void Instance::HandleResetCondition(HandlerContext & ctx,
                                    const ResourceMonitoring::Commands::ResetCondition::DecodableType & commandData)
{

    Status resetConditionStatus = mDelegate->OnResetCondition();
    ctx.mCommandHandler.AddStatus(ctx.mRequestPath, resetConditionStatus);
}

Status Delegate::OnResetCondition()
{
    ChipLogDetail(Zcl, "ResourceMonitoringServer::OnResetCondition()");

    // call application specific pre reset logic,
    // anything other than Success will cause the command to fail, and not do any of the resets
    auto status = PreResetCondition();
    if (status != Status::Success)
    {
        return status;
    }
    // Handle the reset of the condition attribute, if supported
    if (emberAfContainsAttribute(mInstance->GetEndpointId(), mInstance->GetClusterId(), Attributes::Condition::Id))
    {
        if (mInstance->GetDegradationDirection() == DegradationDirectionEnum::kDown)
        {
            mInstance->UpdateCondition(100);
        }
        else if (mInstance->GetDegradationDirection() == DegradationDirectionEnum::kUp)
        {
            mInstance->UpdateCondition(0);
        }
    }

    // handle the reset of the ChangeIndication attribute, mandatory
    mInstance->UpdateChangeIndication(ChangeIndicationEnum::kOk);

    // Handle the reset of the LastChangedTime attribute, if supported
    if (emberAfContainsAttribute(mInstance->GetEndpointId(), mInstance->GetClusterId(), Attributes::LastChangedTime::Id))
    {
        System::Clock::Milliseconds64 currentUnixTimeMS;
        CHIP_ERROR err = System::SystemClock().GetClock_RealTimeMS(currentUnixTimeMS);
        if (err == CHIP_NO_ERROR)
        {
            System::Clock::Seconds32 currentUnixTime = std::chrono::duration_cast<System::Clock::Seconds32>(currentUnixTimeMS);
            mInstance->UpdateLastChangedTime(DataModel::MakeNullable(currentUnixTime.count()));
        }
    }

    // call application specific post reset logic
    status = PostResetCondition();
    return status;
}

Status Delegate::PreResetCondition()
{
    ChipLogDetail(Zcl, "ResourceMonitoringServer::PreResetCondition()");
    return Status::Success;
}

Status Delegate::PostResetCondition()
{
    ChipLogDetail(Zcl, "ResourceMonitoringServer::PostResetCondition()");
    return Status::Success;
}

} // namespace ResourceMonitoring
} // namespace Clusters
} // namespace app
} // namespace chip
