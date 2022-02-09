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
 * @brief Implementation for the Localization Configuration Server Cluster
 ***************************************************************************/

#include <app-common/zap-generated/af-structs.h>
#include <app-common/zap-generated/attributes/Accessors.h>
#include <app-common/zap-generated/cluster-objects.h>
#include <app-common/zap-generated/ids/Attributes.h>
#include <app-common/zap-generated/ids/Clusters.h>
#include <app/AttributeAccessInterface.h>
#include <app/util/attribute-storage.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/logging/CHIPLogging.h>
#include <platform/PlatformManager.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::LocalizationConfiguration;
using namespace chip::app::Clusters::LocalizationConfiguration::Attributes;

namespace {

class LocalizationConfigurationAttrAccess : public AttributeAccessInterface
{
public:
    // Register for the Localization Configuration cluster on all endpoints.
    LocalizationConfigurationAttrAccess() : AttributeAccessInterface(Optional<EndpointId>::Missing(), LocalizationConfiguration::Id)
    {}

    CHIP_ERROR Read(const ConcreteReadAttributePath & aPath, AttributeValueEncoder & aEncoder) override;

private:
    CHIP_ERROR ReadSupportedLocales(AttributeValueEncoder & aEncoder);
};

LocalizationConfigurationAttrAccess gAttrAccess;

CHIP_ERROR LocalizationConfigurationAttrAccess::ReadSupportedLocales(AttributeValueEncoder & aEncoder)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    DeviceLayer::AttributeList<CharSpan, DeviceLayer::kMaxLanguageTags> supportedLocales;

    if (DeviceLayer::PlatformMgr().GetSupportedLocales(supportedLocales) == CHIP_NO_ERROR)
    {
        err = aEncoder.EncodeList([&supportedLocales](const auto & encoder) -> CHIP_ERROR {
            for (const Span<const char> & locale : supportedLocales)
            {
                ReturnErrorOnFailure(encoder.Encode(locale));
            }

            return CHIP_NO_ERROR;
        });
    }
    else
    {
        err = aEncoder.EncodeEmptyList();
    }

    return err;
}

CHIP_ERROR LocalizationConfigurationAttrAccess::Read(const ConcreteReadAttributePath & aPath, AttributeValueEncoder & aEncoder)
{
    VerifyOrDie(aPath.mClusterId == LocalizationConfiguration::Id);

    switch (aPath.mAttributeId)
    {
    case SupportedLocales::Id:
        return ReadSupportedLocales(aEncoder);
    default:
        break;
    }
    return CHIP_NO_ERROR;
}
} // anonymous namespace

// =============================================================================
// Pre-change callbacks for cluster attributes
// =============================================================================

static Protocols::InteractionModel::Status emberAfPluginLocalizationConfigurationOnActiveLocaleChange(EndpointId EndpointId,
                                                                                                      CharSpan newLangtag)
{
    DeviceLayer::AttributeList<CharSpan, DeviceLayer::kMaxLanguageTags> supportedLocales;

    if (DeviceLayer::PlatformMgr().GetSupportedLocales(supportedLocales) == CHIP_NO_ERROR)
    {
        for (const Span<const char> & locale : supportedLocales)
        {
            if (locale.data_equal(newLangtag))
            {
                return Protocols::InteractionModel::Status::Success;
            }
        }
    }

    return Protocols::InteractionModel::Status::InvalidValue;
}

static Protocols::InteractionModel::Status
emberAfPluginLocalizationConfigurationOnUnhandledAttributeChange(EndpointId EndpointId, EmberAfAttributeType attrType,
                                                                 uint16_t attrSize, uint8_t * attrValue)
{
    return Protocols::InteractionModel::Status::Success;
}

Protocols::InteractionModel::Status MatterLocalizationConfigurationClusterServerPreAttributeChangedCallback(
    const ConcreteAttributePath & attributePath, EmberAfAttributeType attributeType, uint16_t size, uint8_t * value)
{
    Protocols::InteractionModel::Status res;

    switch (attributePath.mAttributeId)
    {
    case ActiveLocale::Id: {
        // TODO:: allow fromZclString for CharSpan as well and use that here
        auto langtag = CharSpan(Uint8::to_char(&value[1]), static_cast<size_t>(value[0]));
        res          = emberAfPluginLocalizationConfigurationOnActiveLocaleChange(attributePath.mEndpointId, langtag);
    }
    break;

    default:
        res =
            emberAfPluginLocalizationConfigurationOnUnhandledAttributeChange(attributePath.mEndpointId, attributeType, size, value);
        break;
    }

    return res;
}

void emberAfLocalizationConfigurationClusterServerInitCallback(EndpointId endpoint)
{
    DeviceLayer::AttributeList<CharSpan, DeviceLayer::kMaxLanguageTags> supportedLocales;
    CharSpan validLocale;

    char outBuffer[Attributes::ActiveLocale::TypeInfo::MaxLength()];
    MutableCharSpan activeLocale(outBuffer);
    EmberAfStatus status = ActiveLocale::Get(endpoint, activeLocale);

    VerifyOrReturn(EMBER_ZCL_STATUS_SUCCESS == status, ChipLogError(Zcl, "Failed to read ActiveLocale with error: 0x%02x", status));

    // We could have an invalid ActiveLocale value if an OTA update removed support for the value we were using.
    if (DeviceLayer::PlatformMgr().GetSupportedLocales(supportedLocales) == CHIP_NO_ERROR)
    {
        for (const Span<const char> & locale : supportedLocales)
        {
            if (locale.data_equal(activeLocale))
            {
                return;
            }

            validLocale = locale;
        }

        // If initial value is not one of the allowed values, pick one valid value and write it.
        status = ActiveLocale::Set(endpoint, validLocale);
        VerifyOrReturn(EMBER_ZCL_STATUS_SUCCESS == status,
                       ChipLogError(Zcl, "Failed to write active locale with error: 0x%02x", status));
    }
}

void MatterLocalizationConfigurationPluginServerInitCallback(void)
{
    registerAttributeAccessOverride(&gAttrAccess);
}
