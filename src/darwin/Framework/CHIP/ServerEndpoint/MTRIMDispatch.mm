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

#include <app-common/zap-generated/callback.h>
#include <app-common/zap-generated/cluster-objects.h>
#include <app/CommandHandler.h>
#include <app/ConcreteCommandPath.h>
#include <app/util/attribute-table.h>

#include <app/util/af-types.h>
#include <app/util/privilege-storage.h>
#include <lib/core/Optional.h>
#include <lib/core/TLVReader.h>
#include <platform/LockTracker.h>
#include <protocols/interaction_model/StatusCode.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;

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
        // TODO: Consider having MTRServerCluster register a
        // CommandHandlerInterface for command dispatch.  But OTA would need
        // some special-casing in any case, to call into the existing cluster
        // implementation.
        using Protocols::InteractionModel::Status;
        // This command passed ServerClusterCommandExists so we know it's one of our
        // supported commands.
        using namespace OtaSoftwareUpdateProvider::Commands;

        bool wasHandled = false;
        CHIP_ERROR err = CHIP_NO_ERROR;

        switch (aPath.mCommandId) {
        case QueryImage::Id: {
            QueryImage::DecodableType commandData;
            err = DataModel::Decode(aReader, commandData);
            if (err == CHIP_NO_ERROR) {
                wasHandled = emberAfOtaSoftwareUpdateProviderClusterQueryImageCallback(aCommandObj, aPath, commandData);
            }
            break;
        }
        case ApplyUpdateRequest::Id: {
            ApplyUpdateRequest::DecodableType commandData;
            err = DataModel::Decode(aReader, commandData);
            if (err == CHIP_NO_ERROR) {
                wasHandled = emberAfOtaSoftwareUpdateProviderClusterApplyUpdateRequestCallback(aCommandObj, aPath, commandData);
            }
            break;
        }
        case NotifyUpdateApplied::Id: {
            NotifyUpdateApplied::DecodableType commandData;
            err = DataModel::Decode(aReader, commandData);
            if (err == CHIP_NO_ERROR) {
                wasHandled = emberAfOtaSoftwareUpdateProviderClusterNotifyUpdateAppliedCallback(aCommandObj, aPath, commandData);
            }
            break;
        }
        default:
            break;
        }

        if (CHIP_NO_ERROR != err || !wasHandled) {
            aCommandObj->AddStatus(aPath, Status::InvalidCommand);
        }
    }

} // namespace app
} // namespace chip
