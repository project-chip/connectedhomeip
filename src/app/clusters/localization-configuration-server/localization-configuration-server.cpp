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
#include <protocols/interaction_model/StatusCode.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::LocalizationConfiguration;
using namespace chip::app::Clusters::LocalizationConfiguration::Attributes;

using chip::Protocols::InteractionModel::Status;

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

    DeviceLayer::DeviceInfoProvider * provider = DeviceLayer::GetDeviceInfoProvider();

    if (provider)
    {
        DeviceLayer::DeviceInfoProvider::SupportedLocalesIterator * it = provider->IterateSupportedLocales();

        if (it)
        {
            err = aEncoder.EncodeList([&it](const auto & encoder) -> CHIP_ERROR {
                CharSpan activeLocale;

                while (it->Next(activeLocale))
                {
                    ReturnErrorOnFailure(encoder.Encode(activeLocale));
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

using Status = Protocols::InteractionModel::Status;

static Protocols::InteractionModel::Status emberAfPluginLocalizationConfigurationOnActiveLocaleChange(EndpointId EndpointId,
                                                                                                      CharSpan newLangtag)
{
    DeviceLayer::DeviceInfoProvider * provider = DeviceLayer::GetDeviceInfoProvider();
    DeviceLayer::DeviceInfoProvider::SupportedLocalesIterator * it;

    if (provider && (it = provider->IterateSupportedLocales()))
    {
        CharSpan outLocale;

        while (it->Next(outLocale))
        {
            if (outLocale.data_equal(newLangtag))
            {
                it->Release();
                return Status::Success;
            }
        }

        it->Release();
    }

    return Status::InvalidValue;
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
        break;
    }
    default:
        res = Status::Success;
        break;
    }

    return res;
}

void emberAfLocalizationConfigurationClusterServerInitCallback(EndpointId endpoint)
{
    char outBuf[Attributes::ActiveLocale::TypeInfo::MaxLength()];
    MutableCharSpan activeLocale(outBuf);
    Status status = ActiveLocale::Get(endpoint, activeLocale);

    VerifyOrReturn(Status::Success == status,
                   ChipLogError(Zcl, "Failed to read ActiveLocale with error: 0x%02x", to_underlying(status)));

    DeviceLayer::DeviceInfoProvider * provider = DeviceLayer::GetDeviceInfoProvider();

    VerifyOrReturn(provider != nullptr, ChipLogError(Zcl, "DeviceInfoProvider is not registered"));

    DeviceLayer::DeviceInfoProvider::SupportedLocalesIterator * it = provider->IterateSupportedLocales();

    if (it)
    {
        CHIP_ERROR err = CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND;

        char tempBuf[Attributes::ActiveLocale::TypeInfo::MaxLength()];
        MutableCharSpan validLocale(tempBuf);
        CharSpan outLocale;
        bool validLocaleCached = false;

        while (it->Next(outLocale))
        {
            if (outLocale.data_equal(activeLocale))
            {
                err = CHIP_NO_ERROR;
                break;
            }

            if (!validLocaleCached)
            {
                if (CopyCharSpanToMutableCharSpan(outLocale, validLocale) != CHIP_NO_ERROR)
                {
                    err = CHIP_ERROR_WRITE_FAILED;
                    break;
                }

                validLocaleCached = true;
            }
        }

        it->Release();

        if (err == CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND)
        {
            // If initial value is not one of the allowed values, write the valid value it.
            status = ActiveLocale::Set(endpoint, validLocale);
            VerifyOrReturn(Status::Success == status,
                           ChipLogError(Zcl, "Failed to write active locale with error: 0x%02x", to_underlying(status)));
        }
    }
}

void MatterLocalizationConfigurationPluginServerInitCallback()
{
    registerAttributeAccessOverride(&gAttrAccess);
}
