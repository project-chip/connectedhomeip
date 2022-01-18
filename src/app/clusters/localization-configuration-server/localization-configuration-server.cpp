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
    CHIP_ERROR Write(const ConcreteDataAttributePath & aPath, AttributeValueDecoder & aDecoder) override;

private:
    CHIP_ERROR ReadActiveLocale(EndpointId endpoint, AttributeValueEncoder & aEncoder);
    CHIP_ERROR WriteActiveLocale(EndpointId endpoint, AttributeValueDecoder & aDecoder);
    CHIP_ERROR ReadSupportedLocales(AttributeValueEncoder & aEncoder);
};

LocalizationConfigurationAttrAccess gAttrAccess;

CHIP_ERROR LocalizationConfigurationAttrAccess::ReadActiveLocale(EndpointId endpoint, AttributeValueEncoder & aEncoder)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    char activeLocale[DeviceLayer::ConfigurationManager::kMaxLanguageTagLength + 1];
    size_t codeLen = 0;

    if (DeviceLayer::ConfigurationMgr().GetActiveLocale(activeLocale, sizeof(activeLocale), codeLen) == CHIP_NO_ERROR)
    {
        err = aEncoder.Encode(chip::CharSpan(activeLocale, strlen(activeLocale)));
    }

    return err;
}

CHIP_ERROR LocalizationConfigurationAttrAccess::WriteActiveLocale(EndpointId endpoint, AttributeValueDecoder & aDecoder)
{
    DeviceLayer::AttributeList<chip::CharSpan, DeviceLayer::kMaxLanguageTags> supportedLocales;
    chip::CharSpan activeLocale;

    ReturnErrorOnFailure(aDecoder.Decode(activeLocale));
    VerifyOrReturnError(activeLocale.size() <= DeviceLayer::ConfigurationManager::kMaxLanguageTagLength,
                        CHIP_ERROR_INVALID_MESSAGE_LENGTH);

    return DeviceLayer::ConfigurationMgr().StoreActiveLocale(activeLocale.data(), activeLocale.size());
}

CHIP_ERROR LocalizationConfigurationAttrAccess::ReadSupportedLocales(AttributeValueEncoder & aEncoder)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    DeviceLayer::AttributeList<chip::CharSpan, DeviceLayer::kMaxLanguageTags> supportedLocales;

    if (DeviceLayer::PlatformMgr().GetSupportedLocales(supportedLocales) == CHIP_NO_ERROR)
    {
        err = aEncoder.EncodeList([&supportedLocales](const auto & encoder) -> CHIP_ERROR {
            for (auto locales : supportedLocales)
            {
                ReturnErrorOnFailure(encoder.Encode(locales));
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
    case ActiveLocale::Id:
        return ReadActiveLocale(aPath.mEndpointId, aEncoder);
    case SupportedLocales::Id:
        return ReadSupportedLocales(aEncoder);
    default:
        break;
    }
    return CHIP_NO_ERROR;
}

CHIP_ERROR LocalizationConfigurationAttrAccess::Write(const ConcreteDataAttributePath & aPath, AttributeValueDecoder & aDecoder)
{
    VerifyOrDie(aPath.mClusterId == LocalizationConfiguration::Id);

    switch (aPath.mAttributeId)
    {
    case ActiveLocale::Id:
        return WriteActiveLocale(aPath.mEndpointId, aDecoder);
    default:
        break;
    }
    return CHIP_NO_ERROR;
}

} // anonymous namespace

void MatterLocalizationConfigurationPluginServerInitCallback(void)
{
    registerAttributeAccessOverride(&gAttrAccess);
}
