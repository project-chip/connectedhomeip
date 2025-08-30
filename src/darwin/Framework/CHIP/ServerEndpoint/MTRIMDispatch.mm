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
#include <lib/core/Global.h>
#include <lib/core/Optional.h>
#include <lib/core/TLVReader.h>
#include <platform/LockTracker.h>
#include <protocols/interaction_model/StatusCode.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;

namespace {

// Code assumes there is only a single OTA provider and it lives on EP0
constexpr EndpointId kOtaProviderEndpointId = 0;

/**
 * A OTAProviderServer with a fixed endpoint id, so that it
 * can be used as a Global<> (i.e. supports a constructor without arguments)
 */
class FixedOtaProviderServer : public OtaProviderServer {
public:
    FixedOtaProviderServer()
        : OtaProviderServer(kOtaProviderEndpointId)
    {
    }
};

Global<FixedOtaProviderServer> gOtaProviderServer;

} // anonymous namespace

namespace chip::app::Clusters::OTAProvider {

void SetDelegate(EndpointId endpointId, OTAProviderDelegate * delegate)
{
    if (endpointId != kOtaProviderEndpointId) {
        ChipLogError(AppServer, "Cannot set OTA provider for endpoint %d: not a valid OTA provider endpoint.", endpointId);
        return;
    }
    gOtaProviderServer->SetDelegate(delegate);
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
        // TODO: Consider having MTRServerCluster register a
        // ServerClusterInterface for command dispatch.
        // But OTA would need some special-casing in any case, to call into the
        // existing cluster implementation.
        Access::SubjectDescriptor subjectDescriptor = aCommandObj->GetSubjectDescriptor();

        DataModel::InvokeRequest invokeRequest;
        invokeRequest.path = aPath;
        invokeRequest.subjectDescriptor = &subjectDescriptor;

        std::optional<DataModel::ActionReturnStatus> result = gOtaProviderServer->InvokeCommand(
            invokeRequest, aReader, aCommandObj);

        if (result.has_value()) {
            // The cluster indicates that handler status or data was already set (or will be set asynchronously) by
            // returning std::nullopt. If any other value is returned, it is requesting that a status is set. This
            // includes CHIP_NO_ERROR: in this case CHIP_NO_ERROR would mean set a `status success on the command`
            aCommandObj->AddStatus(aPath, result->GetStatusCode());
        }
    }

} // namespace app
} // namespace chip
