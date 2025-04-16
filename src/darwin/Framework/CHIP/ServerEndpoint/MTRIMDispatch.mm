/**
 *    Copyright (c) 2024 Project CHIP Authors
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

#include <app-common/zap-generated/cluster-objects.h>
#include <app/CommandHandler.h>
#include <app/ConcreteCommandPath.h>
#include <app/clusters/ota-provider/ota-provider-cluster.h>
#include <app/util/af-types.h>
#include <app/util/attribute-table.h>
#include <app/util/privilege-storage.h>
#include <lib/core/Optional.h>
#include <lib/core/TLVReader.h>
#include <platform/LockTracker.h>
#include <protocols/interaction_model/StatusCode.h>

#include <map>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;

namespace {

// Generally logic for what provides have a delegate set on them.
// TODO: is a single OTA provider reasonable?
std::map<EndpointId, OtaProviderLogic> gOtaProviders;

}

namespace chip::app::Clusters::OTAProvider {

void SetDelegate(chip::EndpointId endpointId, OTAProviderDelegate * delegate)
{
    if (gOtaProviders.find(endpointId) == gOtaProviders.end()) {
        gOtaProviders[endpointId] = {};
        ChipLogProgress(AppServer, "New OTA provider set up on endpoint %d", endpointId);
    }
    gOtaProviders[endpointId].SetDelegate(delegate);
}

} // namespace chip::app::Clusters::OTAProvider

void emberAfClusterInitCallback(EndpointId endpoint, ClusterId clusterId)
{
    assertChipStackLockedByCurrentThread();

    // No-op: Descriptor and OTA do not need this, and our client-defined
    // clusters dont use it.
}

Protocols::InteractionModel::Status emAfWriteAttributeExternal(const ConcreteAttributePath & path,
    const EmberAfWriteDataInput & input)
{
    assertChipStackLockedByCurrentThread();

    // All of our attributes are handled via AttributeAccessInterface, so this
    // should be unreached.
    return Protocols::InteractionModel::Status::UnsupportedAttribute;
}

Protocols::InteractionModel::Status emberAfWriteAttribute(const ConcreteAttributePath & path, const EmberAfWriteDataInput & input)
{
    assertChipStackLockedByCurrentThread();

    // All of our attributes are handled via AttributeAccessInterface, so this
    // should be unreached.
    return Protocols::InteractionModel::Status::UnsupportedAttribute;
}

namespace chip {
namespace app {

    void DispatchSingleClusterCommand(const ConcreteCommandPath & aPath, TLV::TLVReader & aReader, CommandHandler * aCommandObj)
    {
        using Protocols::InteractionModel::Status;

        if (aPath.mClusterId != OtaSoftwareUpdateProvider::Id) {
            aCommandObj->AddStatus(aPath, Status::UnsupportedCluster);
            return;
        }

        if (gOtaProviders.find(aPath.mEndpointId) == gOtaProviders.end()) {
            // Error here is unclear: we have no delegate, so just give up
            aCommandObj->AddStatus(aPath, Status::InvalidCommand);
            return;
        }

        OtaProviderLogic & logic = gOtaProviders[aPath.mEndpointId];

        // This command passed ServerClusterCommandExists so we know it's one of our
        // supported commands.
        using namespace OtaSoftwareUpdateProvider::Commands;

        std::optional<DataModel::ActionReturnStatus> result;
        CHIP_ERROR err = CHIP_NO_ERROR;

        switch (aPath.mCommandId) {
        case QueryImage::Id: {
            QueryImage::DecodableType commandData;
            err = DataModel::Decode(aReader, commandData);
            if (err == CHIP_NO_ERROR) {
                result = logic.QueryImage(aPath, commandData, aCommandObj);
            }
            break;
        }
        case ApplyUpdateRequest::Id: {
            ApplyUpdateRequest::DecodableType commandData;
            err = DataModel::Decode(aReader, commandData);
            if (err == CHIP_NO_ERROR) {
                result = logic.ApplyUpdateRequest(aPath, commandData, aCommandObj);
            }
            break;
        }
        case NotifyUpdateApplied::Id: {
            NotifyUpdateApplied::DecodableType commandData;
            err = DataModel::Decode(aReader, commandData);
            if (err == CHIP_NO_ERROR) {
                result = logic.NotifyUpdateApplied(aPath, commandData, aCommandObj);
            }
            break;
        }
        default:
            break;
        }

        if (CHIP_NO_ERROR != err) {
            aCommandObj->AddStatus(aPath, Status::InvalidCommand);
        } else if (result.has_value()) {
            // Provider indicates that handler status or data was already set (or will be set asynchronously) by
            // returning std::nullopt. If any other value is returned, it is requesting that a status is set. This
            // includes CHIP_NO_ERROR: in this case CHIP_NO_ERROR would mean set a `status success on the command`
            aCommandObj->AddStatus(aPath, result->GetStatusCode());
        }
    }

} // namespace app
} // namespace chip
