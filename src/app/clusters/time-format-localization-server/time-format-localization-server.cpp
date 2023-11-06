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
#include <app-common/zap-generated/cluster-objects.h>
#include <app-common/zap-generated/ids/Attributes.h>
#include <app-common/zap-generated/ids/Clusters.h>
#include <app/AttributeAccessInterface.h>
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

TimeFormatLocalizationAttrAccess gAttrAccess;

CHIP_ERROR TimeFormatLocalizationAttrAccess::ReadSupportedCalendarTypes(AttributeValueEncoder & aEncoder)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    DeviceLayer::DeviceInfoProvider * provider = DeviceLayer::GetDeviceInfoProvider();

    if (provider)
    {
        DeviceLayer::DeviceInfoProvider::SupportedCalendarTypesIterator * it = provider->IterateSupportedCalendarTypes();

        if (it)
        {
            err = aEncoder.EncodeList([&it](const auto & encoder) -> CHIP_ERROR {
                CalendarTypeEnum type;

                while (it->Next(type))
                {
                    ReturnErrorOnFailure(encoder.Encode(type));
                }

                return CHIP_NO_ERROR;
            });

            it->Release();
        }
        else
        {
            err = aEncoder.EncodeEmptyList();
        }
    }
    else
    {
        err = aEncoder.EncodeEmptyList();
    }

    return err;
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

    if (provider)
    {
        DeviceLayer::DeviceInfoProvider::SupportedCalendarTypesIterator * it = provider->IterateSupportedCalendarTypes();

        if (it)
        {
            CalendarTypeEnum type;

            while (it->Next(type))
            {
                validType = type;

                if (validType == newType)
                {
                    it->Release();
                    return true;
                }
            }

            it->Release();
        }
    }

    return false;
}

} // anonymous namespace

// =============================================================================
// Pre-change callbacks for cluster attributes
// =============================================================================

static Protocols::InteractionModel::Status emberAfPluginTimeFormatLocalizationOnCalendarTypeChange(EndpointId EndpointId,
                                                                                                   CalendarTypeEnum newType)
{
    Protocols::InteractionModel::Status res;
    CalendarTypeEnum validType = CalendarTypeEnum::kBuddhist;

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

static Protocols::InteractionModel::Status
emberAfPluginTimeFormatLocalizationOnUnhandledAttributeChange(EndpointId EndpointId, EmberAfAttributeType attrType,
                                                              uint16_t attrSize, uint8_t * attrValue)
{
    return Protocols::InteractionModel::Status::Success;
}

Protocols::InteractionModel::Status MatterTimeFormatLocalizationClusterServerPreAttributeChangedCallback(
    const ConcreteAttributePath & attributePath, EmberAfAttributeType attributeType, uint16_t size, uint8_t * value)
{
    Protocols::InteractionModel::Status res;

    switch (attributePath.mAttributeId)
    {
    case ActiveCalendarType::Id:
        if (sizeof(uint8_t) == size)
        {
            res = emberAfPluginTimeFormatLocalizationOnCalendarTypeChange(attributePath.mEndpointId,
                                                                          static_cast<CalendarTypeEnum>(*value));
        }
        else
        {
            res = Protocols::InteractionModel::Status::InvalidValue;
        }
        break;

    default:
        res = emberAfPluginTimeFormatLocalizationOnUnhandledAttributeChange(attributePath.mEndpointId, attributeType, size, value);
        break;
    }

    return res;
}

void emberAfTimeFormatLocalizationClusterServerInitCallback(EndpointId endpoint)
{
    CalendarTypeEnum calendarType;
    CalendarTypeEnum validType;
    EmberAfStatus status = ActiveCalendarType::Get(endpoint, &calendarType);

    VerifyOrReturn(EMBER_ZCL_STATUS_SUCCESS == status,
                   ChipLogError(Zcl, "Failed to read calendar type with error: 0x%02x", status));

    // We could have an invalid calendar type value if an OTA update removed support for the value we were using.
    // If initial value is not one of the allowed values, pick one valid value and write it.
    if (!IsSupportedCalendarType(calendarType, validType))
    {
        status = ActiveCalendarType::Set(endpoint, validType);
        VerifyOrReturn(EMBER_ZCL_STATUS_SUCCESS == status,
                       ChipLogError(Zcl, "Failed to write calendar type with error: 0x%02x", status));
    }
}

void MatterTimeFormatLocalizationPluginServerInitCallback()
{
    registerAttributeAccessOverride(&gAttrAccess);
}
