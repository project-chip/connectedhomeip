/*
 *    Copyright (c) 2025 Project CHIP Authors
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
#include <app/clusters/localization-configuration-server/localization-configuration-cluster.h>
#include <app/static-cluster-config/LocalizationConfiguration.h>
#include <data-model-providers/codegen/CodegenDataModelProvider.h>
#include <lib/support/CodeUtils.h>
#include <protocols/interaction_model/StatusCode.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::LocalizationConfiguration;
using namespace chip::app::Clusters::LocalizationConfiguration::Attributes;
using chip::Protocols::InteractionModel::Status;

// for fixed endpoint, this file is ever only included IF software diagnostics is enabled and that MUST happen only on endpoint 0
// the static assert is skipped in case of dynamic endpoints.
static_assert((LocalizationConfiguration::StaticApplicationConfig::kFixedClusterConfig.size() == 1 &&
               LocalizationConfiguration::StaticApplicationConfig::kFixedClusterConfig[0].endpointNumber == kRootEndpointId) ||
              LocalizationConfiguration::StaticApplicationConfig::kFixedClusterConfig.size() == 0);

namespace {

LazyRegisteredServerCluster<LocalizationConfigurationCluster> gServer;

} // namespace

Status MatterLocalizationConfigurationClusterServerPreAttributeChangedCallback(const ConcreteAttributePath & attributePath,
                                                                               EmberAfAttributeType attributeType, uint16_t size,
                                                                               uint8_t * value)
{
    Status res = Status::Success;

    switch (attributePath.mAttributeId)
    {
    case ActiveLocale::Id: {
        // TODO:: allow fromZclString for CharSpan as well and use that here
        auto langtag = CharSpan(Uint8::to_char(&value[1]), static_cast<size_t>(value[0]));
        char tempBuf[Attributes::ActiveLocale::TypeInfo::MaxLength()];
        MutableCharSpan validLocale(tempBuf);
        CHIP_ERROR error = gServer.Cluster().GetLogic().IsSupportedLocale(langtag, validLocale);
        if (error != CHIP_NO_ERROR)
        {
            res = Status::InvalidValue;
        }
        break;
    }
    default:
        break;
    }
    return res;
}

void emberAfLocalizationConfigurationClusterServerInitCallback(EndpointId endpointId)
{
    VerifyOrReturn(endpointId == kRootEndpointId);
    char outBuf[Attributes::ActiveLocale::TypeInfo::MaxLength()];
    MutableCharSpan activeLocale(outBuf);
    ActiveLocale::Get(endpointId, activeLocale);

    gServer.Create(activeLocale);

    CHIP_ERROR err = CodegenDataModelProvider::Instance().Registry().Register(gServer.Registration());
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(AppServer, "Failed to register LocalizationConfiguration on endpoint %u: %" CHIP_ERROR_FORMAT, endpointId,
                     err.Format());
    }

    char tempBuf[Attributes::ActiveLocale::TypeInfo::MaxLength()];
    MutableCharSpan validLocale(tempBuf);

    CHIP_ERROR error = gServer.Cluster().GetLogic().IsSupportedLocale(activeLocale, validLocale);

    if (error != CHIP_NO_ERROR)
    {
        gServer.Cluster().GetLogic().SetActiveLocale(validLocale);
    }
}

void MatterLocalizationConfigurationClusterServerShutdownCallback(EndpointId endpointId)
{
    VerifyOrReturn(endpointId == kRootEndpointId);
    gServer.Destroy();

    CHIP_ERROR err = CodegenDataModelProvider::Instance().Registry().Unregister(&gServer.Cluster());
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(AppServer, "Failed to unregister LocalizationConfiguration on endpoint %u: %" CHIP_ERROR_FORMAT, endpointId,
                     err.Format());
    }

    gServer.Destroy();
}

void MatterLocalizationConfigurationPluginServerInitCallback() {}

void MatterLocalizationConfigurationPluginServerShutdownCallback() {}
