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
#include <app-common/zap-generated/ids/Attributes.h>
#include <app-common/zap-generated/ids/Clusters.h>
#include <app/AttributeAccessInterface.h>
#include <app/CommandHandler.h>
#include <app/MessageDef/AttributeDataElement.h>
#include <app/util/af.h>
#include <app/util/attribute-storage.h>
#include <lib/core/CHIPEncoding.h>
#include <lib/core/CHIPTLVTypes.h>
#include <lib/core/Optional.h>
#include <lib/support/CHIPPlatformMemory.h>
#include <platform/CHIPDeviceLayer.h>
#include <platform/ConnectivityManager.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::ThreadNetworkDiagnostics::Attributes;
using namespace chip::DeviceLayer;

namespace {

class ThreadDiagosticsAttrAccess : public AttributeAccessInterface
{
public:
    // Register for the ThreadNetworkDiagnostics cluster on all endpoints.
    ThreadDiagosticsAttrAccess() : AttributeAccessInterface(Optional<EndpointId>::Missing(), ThreadNetworkDiagnostics::Id) {}

    CHIP_ERROR Read(ClusterInfo & aClusterInfo, const AttributeValueEncoder & aEncoder, bool * aDataRead) override;
};

ThreadDiagosticsAttrAccess gAttrAccess;

CHIP_ERROR ThreadDiagosticsAttrAccess::Read(ClusterInfo & aClusterInfo, const AttributeValueEncoder & aEncoder, bool * aDataRead)
{
    if (aClusterInfo.mClusterId != ThreadNetworkDiagnostics::Id)
    {
        // We shouldn't have been called at all.
        return CHIP_ERROR_INVALID_ARGUMENT;
    }

    CHIP_ERROR err = ConnectivityMgr().WriteThreadNetworkDiagnosticAttributeToTlv(aClusterInfo.mFieldId, aEncoder);

    *aDataRead = true;

    // If it isn't a run time assigned attribute, e.j ClusterRevision, or if
    // not implemented, use standard read.
    // Clear error and no data read
    if (err == CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE || err == CHIP_ERROR_NOT_IMPLEMENTED)
    {
        err        = CHIP_NO_ERROR;
        *aDataRead = false;
    }

    return err;
}
} // anonymous namespace

bool emberAfThreadNetworkDiagnosticsClusterResetCountsCallback(EndpointId endpoint, app::CommandHandler * commandObj)
{
    EmberAfStatus status = ThreadNetworkDiagnostics::Attributes::OverrunCount::Set(endpoint, 0);
    if (status != EMBER_ZCL_STATUS_SUCCESS)
    {
        ChipLogError(Zcl, "Failed to reset OverrunCount attribute");
    }

    ConnectivityMgr().ResetThreadNetworkDiagnosticsCounts();

    emberAfSendImmediateDefaultResponse(status);
    return true;
}

void emberAfThreadNetworkDiagnosticsClusterServerInitCallback(EndpointId endpoint)
{
    static bool attrAccessRegistered = false;
    if (!attrAccessRegistered)
    {
        registerAttributeAccessOverride(&gAttrAccess);
        attrAccessRegistered = true;
    }
}
