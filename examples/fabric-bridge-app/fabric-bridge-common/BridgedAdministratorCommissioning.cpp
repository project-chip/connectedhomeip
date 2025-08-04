/*
 *    Copyright (c) 2024 Project CHIP Authors
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

#include "BridgedAdministratorCommissioning.h"

#include "BridgedDevice.h"
#include "BridgedDeviceManager.h"
#include <app/AttributeAccessInterfaceRegistry.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::AdministratorCommissioning;

namespace bridge {

BridgedAdministratorCommissioning::BridgedAdministratorCommissioning(BridgedDevice & device) :
    AttributeAccessInterface(MakeOptional(device.GetEndpointId()), chip::app::Clusters::AdministratorCommissioning::Id),
    mDevice(device)
{
    VerifyOrDie(AttributeAccessInterfaceRegistry::Instance().Register(this));
}

BridgedAdministratorCommissioning::~BridgedAdministratorCommissioning()
{
    AttributeAccessInterfaceRegistry::Instance().Unregister(this);
}

CHIP_ERROR BridgedAdministratorCommissioning::Read(const ConcreteReadAttributePath & aPath, AttributeValueEncoder & aEncoder)
{
    VerifyOrDie(aPath.mClusterId == Clusters::AdministratorCommissioning::Id);
    VerifyOrDie(aPath.mEndpointId == mDevice.GetEndpointId());
    auto attr = mDevice.GetAdminCommissioningAttributes();

    switch (aPath.mAttributeId)
    {
    case Attributes::WindowStatus::Id: {
        return aEncoder.Encode(attr.commissioningWindowStatus);
    }
    case Attributes::AdminFabricIndex::Id: {
        DataModel::Nullable<FabricIndex> encodeableFabricIndex = DataModel::NullNullable;
        if (attr.openerFabricIndex.has_value())
        {
            encodeableFabricIndex.SetNonNull(attr.openerFabricIndex.value());
        }
        return aEncoder.Encode(encodeableFabricIndex);
    }
    case Attributes::AdminVendorId::Id: {
        DataModel::Nullable<VendorId> encodeableVendorId = DataModel::NullNullable;
        if (attr.openerVendorId.has_value())
        {
            encodeableVendorId.SetNonNull(attr.openerVendorId.value());
        }
        return aEncoder.Encode(encodeableVendorId);
    }
    default:
        break;
    }

    return CHIP_NO_ERROR;
}

} // namespace bridge
