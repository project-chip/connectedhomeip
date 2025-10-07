/*
 *    Copyright (c) 2025 Project CHIP Authors
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

#include "electrical-grid-conditions-server.h"

#include <app/AttributeAccessInterface.h>
#include <app/AttributeAccessInterfaceRegistry.h>
#include <app/CommandHandlerInterfaceRegistry.h>
#include <app/ConcreteAttributePath.h>
#include <app/EventLogging.h>
#include <app/InteractionModelEngine.h>
#include <app/reporting/reporting.h>
#include <lib/support/ScopedBuffer.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::DataModel;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::ElectricalGridConditions;
using namespace chip::app::Clusters::ElectricalGridConditions::Attributes;
using namespace chip::app::Clusters::ElectricalGridConditions::Structs;

using chip::Protocols::InteractionModel::Status;

namespace chip {
namespace app {
namespace Clusters {
namespace ElectricalGridConditions {

CHIP_ERROR Instance::Init()
{
    VerifyOrReturnError(AttributeAccessInterfaceRegistry::Instance().Register(this), CHIP_ERROR_INCORRECT_STATE);

    return CHIP_NO_ERROR;
}

void Instance::Shutdown()
{
    AttributeAccessInterfaceRegistry::Instance().Unregister(this);

    // Reset forecast conditions to not point to mForecastConditionsStorage after freeing
    mForecastConditions = DataModel::List<const Structs::ElectricalGridConditionsStruct::Type>();
    mForecastConditionsStorage.Free();
}

bool Instance::HasFeature(Feature aFeature) const
{
    return mFeatures.Has(aFeature);
}

// AttributeAccessInterface
CHIP_ERROR Instance::Read(const ConcreteReadAttributePath & aPath, AttributeValueEncoder & aEncoder)
{
    switch (aPath.mAttributeId)
    {
    case LocalGenerationAvailable::Id:
        return aEncoder.Encode(mLocalGenerationAvailable);
    case CurrentConditions::Id:
        return aEncoder.Encode(mCurrentConditions);
    case ForecastConditions::Id:
        /* FORE - Forecasting */
        if (!HasFeature(Feature::kForecasting))
        {
            return CHIP_IM_GLOBAL_STATUS(UnsupportedAttribute);
        }

        return aEncoder.EncodeList([this](const auto & encoder) -> CHIP_ERROR {
            for (auto const & condition : mForecastConditions)
            {
                ReturnErrorOnFailure(encoder.Encode(condition));
            }
            return CHIP_NO_ERROR;
        });

    /* FeatureMap - is held locally */
    case FeatureMap::Id:
        return aEncoder.Encode(mFeatures);
    }

    /* Allow all other unhandled attributes to fall through to Ember */
    return CHIP_NO_ERROR;
}

// --------------- Internal Attribute Set APIs
CHIP_ERROR Instance::SetLocalGenerationAvailable(DataModel::Nullable<bool> newValue)
{
    DataModel::Nullable<bool> oldValue = mLocalGenerationAvailable;

    mLocalGenerationAvailable = newValue;

    if (oldValue != newValue)
    {
        if (newValue.IsNull())
        {
            ChipLogDetail(AppServer, "Endpoint %d - mLocalGenerationAvailable updated to Null", mEndpointId);
        }
        else
        {
            ChipLogDetail(AppServer, "Endpoint %d - mLocalGenerationAvailable updated to %d", mEndpointId,
                          static_cast<int>(mLocalGenerationAvailable.Value()));
        }

        MatterReportingAttributeChangeCallback(mEndpointId, ElectricalGridConditions::Id, LocalGenerationAvailable::Id);
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR Instance::SetCurrentConditions(DataModel::Nullable<Structs::ElectricalGridConditionsStruct::Type> newValue)
{
    if (!newValue.IsNull())
    {
        if (EnsureKnownEnumValue(newValue.Value().gridCarbonLevel) == ThreeLevelEnum::kUnknownEnumValue)
        {
            return CHIP_IM_GLOBAL_STATUS(ConstraintError);
        }

        if (EnsureKnownEnumValue(newValue.Value().localCarbonLevel) == ThreeLevelEnum::kUnknownEnumValue)
        {
            return CHIP_IM_GLOBAL_STATUS(ConstraintError);
        }

        // Check PeriodStart  < PeriodEnd (if not null)
        if (!newValue.Value().periodEnd.IsNull() && (newValue.Value().periodStart > newValue.Value().periodEnd.Value()))
        {
            return CHIP_IM_GLOBAL_STATUS(ConstraintError);
        }
    }

    // Use simple comparison - operator!= handles deep comparison including null states
    if (mCurrentConditions != newValue)
    {
        mCurrentConditions = newValue;
        ChipLogDetail(AppServer, "Endpoint %d - mCurrentConditions updated", mEndpointId);
        MatterReportingAttributeChangeCallback(mEndpointId, ElectricalGridConditions::Id, CurrentConditions::Id);

        // generate a CurrentConditionsChanged Event
        GenerateCurrentConditionsChangedEvent();
    }

    return CHIP_NO_ERROR;
}

Status Instance::GenerateCurrentConditionsChangedEvent()
{

    Events::CurrentConditionsChanged::Type event;
    EventNumber eventNumber;

    event.currentConditions = mCurrentConditions;

    CHIP_ERROR err = LogEvent(event, mEndpointId, eventNumber);
    if (CHIP_NO_ERROR != err)
    {
        ChipLogError(AppServer, "Endpoint %d - Unable to send notify event: %" CHIP_ERROR_FORMAT, mEndpointId, err.Format());
        return Status::Failure;
    }
    return Status::Success;
}

CHIP_ERROR Instance::SetForecastConditions(const DataModel::List<const Structs::ElectricalGridConditionsStruct::Type> & newValue)
{
    // Validate input constraints
    if (newValue.size() > kMaxForecastEntries)
    {
        ChipLogError(AppServer, "Endpoint %d - ForecastConditions list size %u exceeds maximum %d", mEndpointId,
                     static_cast<unsigned int>(newValue.size()), kMaxForecastEntries);
        return CHIP_IM_GLOBAL_STATUS(ConstraintError);
    }

    // Validate each entry individually
    for (size_t i = 0; i < newValue.size(); i++)
    {
        const auto & condition = newValue[i];

        if (EnsureKnownEnumValue(condition.gridCarbonLevel) == ThreeLevelEnum::kUnknownEnumValue)
        {
            return CHIP_IM_GLOBAL_STATUS(ConstraintError);
        }

        if (EnsureKnownEnumValue(condition.localCarbonLevel) == ThreeLevelEnum::kUnknownEnumValue)
        {
            return CHIP_IM_GLOBAL_STATUS(ConstraintError);
        }

        // Check PeriodStart < PeriodEnd (if not null)
        if (!condition.periodEnd.IsNull() && (condition.periodStart > condition.periodEnd.Value()))
        {
            return CHIP_IM_GLOBAL_STATUS(ConstraintError);
        }
    }

    // Validate time order constraints per spec:
    // The list entries SHALL be in time order
    if (!newValue.empty())
    {
        for (size_t i = 0; i < newValue.size(); i++)
        {
            const auto & condition = newValue[i];

            // All entries except the last one SHALL have a non-null PeriodEnd
            if (i < newValue.size() - 1) // Not the last entry
            {
                if (condition.periodEnd.IsNull())
                {
                    ChipLogError(AppServer, "Endpoint %d - Entry %u: All entries except the last SHALL have a non-null PeriodEnd",
                                 mEndpointId, static_cast<unsigned int>(i));
                    return CHIP_IM_GLOBAL_STATUS(ConstraintError);
                }
            }

            // For all entries except the first one, PeriodStart SHALL be greater than the previous entry's PeriodEnd
            if (i > 0) // Not the first entry
            {
                const auto & previousCondition = newValue[i - 1];

                if (condition.periodStart <= previousCondition.periodEnd.Value())
                {
                    ChipLogError(AppServer,
                                 "Endpoint %d - Entry %u: PeriodStart (%u) SHALL be greater than previous PeriodEnd (%u)",
                                 mEndpointId, static_cast<unsigned int>(i), static_cast<unsigned int>(condition.periodStart),
                                 static_cast<unsigned int>(previousCondition.periodEnd.Value()));
                    return CHIP_IM_GLOBAL_STATUS(ConstraintError);
                }
            }
        }
    }

    // Do a deep copy of the newValue into mForecastConditions
    ReturnErrorOnFailure(CopyForecastConditions(newValue));

    ChipLogDetail(AppServer, "Endpoint %d - mForecastConditions updated with %u entries", mEndpointId,
                  static_cast<unsigned int>(mForecastConditionsStorage.AllocatedSize()));

    MatterReportingAttributeChangeCallback(mEndpointId, ElectricalGridConditions::Id, ForecastConditions::Id);

    return CHIP_NO_ERROR;
}

CHIP_ERROR Instance::CopyForecastConditions(const DataModel::List<const Structs::ElectricalGridConditionsStruct::Type> & src)
{
    // Convert DataModel::List to Span for CopyFromSpan
    chip::Span<const Structs::ElectricalGridConditionsStruct::Type> srcSpan(src.data(), src.size());

    // CopyFromSpan handles allocation, copying, and size tracking automatically
    mForecastConditionsStorage.CopyFromSpan(srcSpan);

    // Check if allocation succeeded (CopyFromSpan will have size 0 if allocation failed)
    if (src.size() > 0 && mForecastConditionsStorage.AllocatedSize() == 0)
    {
        ChipLogError(AppServer, "Endpoint %d - Memory allocation failed for forecast conditions storage", mEndpointId);
        return CHIP_ERROR_NO_MEMORY;
    }

    // Update the list to point to our managed storage (or empty if srcSpan was empty)
    auto storageSpan = mForecastConditionsStorage.Span();
    mForecastConditions =
        DataModel::List<const Structs::ElectricalGridConditionsStruct::Type>(storageSpan.data(), storageSpan.size());

    return CHIP_NO_ERROR;
}

} // namespace ElectricalGridConditions
} // namespace Clusters
} // namespace app
} // namespace chip
