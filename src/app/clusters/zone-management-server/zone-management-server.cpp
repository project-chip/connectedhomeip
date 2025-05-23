/**
 *
 *    Copyright (c) 2025 Project CHIP Authors
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
 *
 */

#include <app/AttributeAccessInterface.h>
#include <app/AttributeAccessInterfaceRegistry.h>
#include <app/CommandHandlerInterfaceRegistry.h>
#include <app/InteractionModelEngine.h>
#include <app/clusters/zone-management-server/zone-management-server.h>
#include <app/reporting/reporting.h>
#include <app/util/attribute-storage.h>
#include <app/util/util.h>
#include <lib/core/CHIPSafeCasts.h>
#include <lib/support/DefaultStorageKeyAllocator.h>
#include <protocols/interaction_model/StatusCode.h>

#include <cmath>
#include <cstring>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::ZoneManagement;
using namespace chip::app::Clusters::ZoneManagement::Structs;
using namespace chip::app::Clusters::ZoneManagement::Attributes;
using namespace Protocols::InteractionModel;

namespace chip {
namespace app {
namespace Clusters {
namespace ZoneManagement {

ZoneManagementServer::ZoneManagementServer(ZoneManagementDelegate & aDelegate, EndpointId aEndpointId, const BitFlags<Feature> aFeatures,
                        const BitFlags<OptionalAttribute> aOptionalAttrs, uint8_t aMaxUserDefinedZones,
                        uint8_t aMaxZones, uint8_t aSensitivityMax, const TwoDCartesianVertexStruct & aTwoDCartesianMax) :
    CommandHandlerInterface(MakeOptional(aEndpointId), ZoneManagement::Id),
    AttributeAccessInterface(MakeOptional(aEndpointId), ZoneManagement::Id), mDelegate(aDelegate),
    mEndpointId(aEndpointId), mFeatures(aFeatures), mOptionalAttrs(aOptionalAttrs), mMaxUserDefinedZones(aMaxUserDefinedZones),
    mMaxZones(aMaxZones), mSensitivityMax(aSensitivityMax), mTwoDCartesianMax(aTwoDCartesianMax)
{
    mDelegate.SetZoneManagementServer(this);
}

ZoneManagementServer::~ZoneManagementServer()
{
    // Explicitly set the ZoneManagementServer pointer in the Delegate to
    // null.
    mDelegate.SetZoneManagementServer(nullptr);

    // Unregister command handler and attribute access interfaces
    CommandHandlerInterfaceRegistry::Instance().UnregisterCommandHandler(this);
    AttributeAccessInterfaceRegistry::Instance().Unregister(this);
}

CHIP_ERROR ZoneManagementServer::Init()
{
    // Perform constraint checks

    VerifyOrReturnError(AttributeAccessInterfaceRegistry::Instance().Register(this), CHIP_ERROR_INTERNAL);
    ReturnErrorOnFailure(CommandHandlerInterfaceRegistry::Instance().RegisterCommandHandler(this));
    return CHIP_NO_ERROR;
}

bool ZoneManagementServer::HasFeature(Feature feature) const
{
    return mFeatures.Has(feature);
}

bool ZoneManagementServer::SupportsOptAttr(OptionalAttribute aOptionalAttr) const
{
    return mOptionalAttrs.Has(aOptionalAttr);
}


// AttributeAccessInterface
CHIP_ERROR CameraAVStreamMgmtServer::Read(const ConcreteReadAttributePath & aPath, AttributeValueEncoder & aEncoder)
{
    VerifyOrDie(aPath.mClusterId == CameraAvStreamManagement::Id);
    ChipLogProgress(Zcl, "Camera AVStream Management[ep=%d]: Reading", mEndpointId);

    switch (aPath.mAttributeId)
    {
    case FeatureMap::Id:
        ReturnErrorOnFailure(aEncoder.Encode(mFeatures));
        break;
    }

    return CHIP_NO_ERROR;
}
} // namespace ZoneManagement
} // namespace Clusters
} // namespace app
} // namespace chip

/** @brief Zone Management Cluster Server Init
 *
 * Server Init
 *
 */
void MatterZoneManagementPluginServerInitCallback() {}
void MatterZoneManagementPluginServerShutdownCallback() {}