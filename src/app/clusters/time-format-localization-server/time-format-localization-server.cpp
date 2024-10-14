/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
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

/****************************************************************************
 * @file
 * @brief Implementation for the Time Format Localization Server Cluster
 ***************************************************************************/

#include <app-common/zap-generated/attributes/Accessors.h>
#include <app-common/zap-generated/cluster-enums-check.h>
#include <app-common/zap-generated/cluster-objects.h>
#include <app-common/zap-generated/ids/Attributes.h>
#include <app-common/zap-generated/ids/Clusters.h>
#include <app/AttributeAccessInterface.h>
#include <app/AttributeAccessInterfaceRegistry.h>
#include <app/util/attribute-storage.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/logging/CHIPLogging.h>
#include <platform/DeviceInfoProvider.h>
#include <platform/PlatformManager.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::TimeFormatLocalization;
using namespace chip::app::Clusters::TimeFormatLocalization::Attributes;
using namespace chip::DeviceLayer;

using chip::Protocols::InteractionModel::Status;

namespace {

class TimeFormatLocalizationAttrAccess : public AttributeAccessInterface
{
public:
    // Register for the Time Format Localization cluster on all endpoints.
    TimeFormatLocalizationAttrAccess() : AttributeAccessInterface(Optional<EndpointId>::Missing(), TimeFormatLocalization::Id) {}

    CHIP_ERROR Read(const ConcreteReadAttributePath & aPath, AttributeValueEncoder & aEncoder) override;

private:
    CHIP_ERROR ReadSupportedCalendarTypes(AttributeValueEncoder & aEncoder);
};

class AutoReleaseIterator
{
public:
    using Iterator = DeviceLayer::DeviceInfoProvider::SupportedCalendarTypesIterator;

    AutoReleaseIterator(Iterator * value) : mIterator(value) {}
    ~AutoReleaseIterator()
    {
        if (mIterator != nullptr)
        {
            mIterator->Release();
        }
    }

    bool IsValid() const { return mIterator != nullptr; }
    bool Next(CalendarTypeEnum & value) { return (mIterator == nullptr) ? false : mIterator->Next(value); }

private:
    Iterator * mIterator;
};

TimeFormatLocalizationAttrAccess gAttrAccess;

bool HasFeature(EndpointId endpoint, Feature feature)
{
    uint32_t featureMap;
    return FeatureMap::Get(endpoint, &featureMap) == Status::Success ? (featureMap & to_underlying(feature)) : false;
}

CHIP_ERROR TimeFormatLocalizationAttrAccess::ReadSupportedCalendarTypes(AttributeValueEncoder & aEncoder)
{
    DeviceLayer::DeviceInfoProvider * provider = DeviceLayer::GetDeviceInfoProvider();
    VerifyOrReturnValue(provider != nullptr, aEncoder.EncodeEmptyList());

    AutoReleaseIterator it(provider->IterateSupportedCalendarTypes());
    VerifyOrReturnValue(it.IsValid(), aEncoder.EncodeEmptyList());

    return aEncoder.EncodeList([&it](const auto & encoder) -> CHIP_ERROR {
        CalendarTypeEnum type;

        while (it.Next(type))
        {
            ReturnErrorOnFailure(encoder.Encode(type));
        }

        return CHIP_NO_ERROR;
    });
}

CHIP_ERROR TimeFormatLocalizationAttrAccess::Read(const ConcreteReadAttributePath & aPath, AttributeValueEncoder & aEncoder)
{
    VerifyOrDie(aPath.mClusterId == TimeFormatLocalization::Id);

    switch (aPath.mAttributeId)
    {
    case SupportedCalendarTypes::Id:
        return ReadSupportedCalendarTypes(aEncoder);
    default:
        break;
    }
    return CHIP_NO_ERROR;
}

// Returns whether newType is a valid calendar type.  If it's not, validType is set to a valid calendar type,
// if there are any, and to kBuddhist if there are not.
bool IsSupportedCalendarType(CalendarTypeEnum newType, CalendarTypeEnum & validType)
{
    // Reset valid type if no supported calendar types found.
    validType = CalendarTypeEnum::kBuddhist;

    DeviceLayer::DeviceInfoProvider * provider = DeviceLayer::GetDeviceInfoProvider();
    VerifyOrReturnValue(provider != nullptr, false);

    AutoReleaseIterator it(provider->IterateSupportedCalendarTypes());
    VerifyOrReturnValue(it.IsValid(), false);

    CalendarTypeEnum type;
    while (it.Next(type))
    {
        validType = type;
        if (validType == newType)
        {
            return true;
        }
    }

    return false;
}

template <typename E>
Optional<E> SafeCast(uint8_t value)
{
    E val = static_cast<E>(value);
    if (EnsureKnownEnumValue(val) == E::kUnknownEnumValue)
    {
        return NullOptional;
    }
    return MakeOptional(val);
}

} // anonymous namespace

// =============================================================================
// Pre-change callbacks for cluster attributes
// =============================================================================

static Protocols::InteractionModel::Status emberAfPluginTimeFormatLocalizationOnCalendarTypeChange(EndpointId EndpointId,
                                                                                                   CalendarTypeEnum newType)
{
    Protocols::InteractionModel::Status res;
    CalendarTypeEnum validType = CalendarTypeEnum::kUseActiveLocale;

    if (IsSupportedCalendarType(newType, validType))
    {
        res = Protocols::InteractionModel::Status::Success;
    }
    else
    {
        res = Protocols::InteractionModel::Status::InvalidValue;
        ChipLogError(Zcl, "Trying to write invalid Calendar Type");
    }

    return res;
}

Protocols::InteractionModel::Status MatterTimeFormatLocalizationClusterServerPreAttributeChangedCallback(
    const ConcreteAttributePath & attributePath, EmberAfAttributeType attributeType, uint16_t size, uint8_t * value)
{
    switch (attributePath.mAttributeId)
    {
    case ActiveCalendarType::Id: {
        VerifyOrReturnValue(sizeof(uint8_t) == size, Protocols::InteractionModel::Status::InvalidValue);

        auto calendarType = SafeCast<CalendarTypeEnum>(*value);
        VerifyOrReturnValue(calendarType.HasValue(), Protocols::InteractionModel::Status::ConstraintError);

        return emberAfPluginTimeFormatLocalizationOnCalendarTypeChange(attributePath.mEndpointId, calendarType.Value());
    }
    case HourFormat::Id: {
        VerifyOrReturnValue(sizeof(uint8_t) == size, Protocols::InteractionModel::Status::InvalidValue);

        auto hourFormat = SafeCast<HourFormatEnum>(*value);
        VerifyOrReturnValue(hourFormat.HasValue(), Protocols::InteractionModel::Status::ConstraintError);

        return Protocols::InteractionModel::Status::Success;
    }
    default:
        return Protocols::InteractionModel::Status::Success;
    }
}

void emberAfTimeFormatLocalizationClusterServerInitCallback(EndpointId endpoint)
{
    if (!HasFeature(endpoint, Feature::kCalendarFormat))
    {
        return;
    }
    CalendarTypeEnum calendarType;
    CalendarTypeEnum validType;
    Status status = ActiveCalendarType::Get(endpoint, &calendarType);

    VerifyOrReturn(Status::Success == status,
                   ChipLogError(Zcl, "Failed to read calendar type with error: 0x%02x", to_underlying(status)));

    // We could have an invalid calendar type value if an OTA update removed support for the value we were using.
    // If initial value is not one of the allowed values, pick one valid value and write it.
    if (!IsSupportedCalendarType(calendarType, validType))
    {
        status = ActiveCalendarType::Set(endpoint, validType);
        VerifyOrReturn(Status::Success == status,
                       ChipLogError(Zcl, "Failed to write calendar type with error: 0x%02x", to_underlying(status)));
    }
}

void MatterTimeFormatLocalizationPluginServerInitCallback()
{
    AttributeAccessInterfaceRegistry::Instance().Register(&gAttrAccess);
}
