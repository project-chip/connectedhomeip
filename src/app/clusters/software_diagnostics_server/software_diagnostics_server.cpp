/**
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

#include <app-common/zap-generated/attributes/Accessors.h>
#include <app-common/zap-generated/cluster-objects.h>
#include <app-common/zap-generated/ids/Attributes.h>
#include <app-common/zap-generated/ids/Clusters.h>
#include <app/AttributeAccessInterface.h>
#include <app/CommandHandler.h>
#include <app/ConcreteCommandPath.h>
#include <app/util/af.h>
#include <app/util/attribute-storage.h>
#include <lib/core/Optional.h>
#include <platform/PlatformManager.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::SoftwareDiagnostics;
using namespace chip::app::Clusters::SoftwareDiagnostics::Attributes;
using chip::DeviceLayer::PlatformManager;

namespace {

class SoftwareDiagosticsAttrAccess : public AttributeAccessInterface
{
public:
    // Register for the SoftwareDiagnostics cluster on all endpoints.
    SoftwareDiagosticsAttrAccess() : AttributeAccessInterface(Optional<EndpointId>::Missing(), SoftwareDiagnostics::Id) {}

    CHIP_ERROR Read(const ConcreteAttributePath & aPath, AttributeValueEncoder & aEncoder) override;

private:
    CHIP_ERROR ReadIfSupported(CHIP_ERROR (PlatformManager::*getter)(uint64_t &), AttributeValueEncoder & aEncoder);
    CHIP_ERROR ReadThreadMetrics(AttributeValueEncoder & aEncoder);
};

SoftwareDiagosticsAttrAccess gAttrAccess;

CHIP_ERROR SoftwareDiagosticsAttrAccess::Read(const ConcreteAttributePath & aPath, AttributeValueEncoder & aEncoder)
{
    if (aPath.mClusterId != SoftwareDiagnostics::Id)
    {
        // We shouldn't have been called at all.
        return CHIP_ERROR_INVALID_ARGUMENT;
    }

    switch (aPath.mAttributeId)
    {
    case CurrentHeapFree::Id: {
        return ReadIfSupported(&PlatformManager::GetCurrentHeapFree, aEncoder);
    }
    case CurrentHeapUsed::Id: {
        return ReadIfSupported(&PlatformManager::GetCurrentHeapUsed, aEncoder);
    }
    case CurrentHeapHighWatermark::Id: {
        return ReadIfSupported(&PlatformManager::GetCurrentHeapHighWatermark, aEncoder);
    }
    case ThreadMetrics::Id: {
        return ReadThreadMetrics(aEncoder);
    }
    default: {
        break;
    }
    }
    return CHIP_NO_ERROR;
}

CHIP_ERROR SoftwareDiagosticsAttrAccess::ReadIfSupported(CHIP_ERROR (PlatformManager::*getter)(uint64_t &),
                                                         AttributeValueEncoder & aEncoder)
{
    uint64_t data;
    CHIP_ERROR err = (DeviceLayer::PlatformMgr().*getter)(data);
    if (err == CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE)
    {
        data = 0;
    }
    else if (err != CHIP_NO_ERROR)
    {
        return err;
    }

    return aEncoder.Encode(data);
}

CHIP_ERROR SoftwareDiagosticsAttrAccess::ReadThreadMetrics(AttributeValueEncoder & aEncoder)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    DeviceLayer::ThreadMetrics * threadMetrics;

    if (DeviceLayer::PlatformMgr().GetThreadMetrics(&threadMetrics) == CHIP_NO_ERROR)
    {
        err = aEncoder.EncodeList([&threadMetrics](const TagBoundEncoder & encoder) -> CHIP_ERROR {
            for (DeviceLayer::ThreadMetrics * thread = threadMetrics; thread != nullptr; thread = thread->Next)
            {
                ReturnErrorOnFailure(encoder.Encode(*thread));
            }

            return CHIP_NO_ERROR;
        });

        DeviceLayer::PlatformMgr().ReleaseThreadMetrics(threadMetrics);
    }
    else
    {
        err = aEncoder.Encode(DataModel::List<EndpointId>());
    }

    return err;
}
} // anonymous namespace

bool emberAfSoftwareDiagnosticsClusterResetWatermarksCallback(app::CommandHandler * commandObj,
                                                              const app::ConcreteCommandPath & commandPath,
                                                              const Commands::ResetWatermarks::DecodableType & commandData)
{
    EndpointId endpoint = commandPath.mEndpointId;

    uint64_t currentHeapUsed;

    EmberAfStatus status = SoftwareDiagnostics::Attributes::CurrentHeapUsed::Get(endpoint, &currentHeapUsed);
    VerifyOrExit(status == EMBER_ZCL_STATUS_SUCCESS, ChipLogError(Zcl, "Failed to get the value of the CurrentHeapUsed attribute"));

    status = SoftwareDiagnostics::Attributes::CurrentHeapHighWatermark::Set(endpoint, currentHeapUsed);
    VerifyOrExit(
        status == EMBER_ZCL_STATUS_SUCCESS,
        ChipLogError(
            Zcl,
            "Failed to reset the value of the CurrentHeapHighWaterMark attribute to the value of the CurrentHeapUsed attribute"));

exit:
    emberAfSendImmediateDefaultResponse(status);

    return true;
}

void MatterSoftwareDiagnosticsPluginServerInitCallback()
{
    registerAttributeAccessOverride(&gAttrAccess);
}
