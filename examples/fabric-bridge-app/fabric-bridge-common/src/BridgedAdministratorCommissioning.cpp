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

CHIP_ERROR BridgedAdministratorCommissioning::Init()
{
    // We expect initialization after emberAfInit(). This allows us to unregister the existing
    // AccessAttributeInterface for AdministratorCommissioning and register ourselves, ensuring we
    // get the callback for reading attribute. If the read is not intended for a bridged device we will
    // forward it to the original attribute interface that we are unregistering.
    mOriginalAttributeInterface = AttributeAccessInterfaceRegistry::Instance().Get(kRootEndpointId, AdministratorCommissioning::Id);
    VerifyOrReturnError(mOriginalAttributeInterface, CHIP_ERROR_INTERNAL);
    AttributeAccessInterfaceRegistry::Instance().Unregister(mOriginalAttributeInterface);
    VerifyOrDie(AttributeAccessInterfaceRegistry::Instance().Register(this));
    return CHIP_NO_ERROR;
}

CHIP_ERROR BridgedAdministratorCommissioning::Read(const ConcreteReadAttributePath & aPath, AttributeValueEncoder & aEncoder)
{
    VerifyOrDie(aPath.mClusterId == Clusters::AdministratorCommissioning::Id);
    EndpointId endpointId  = aPath.mEndpointId;
    BridgedDevice * device = BridgeDeviceMgr().GetDevice(endpointId);

    if (!device)
    {
        VerifyOrDie(mOriginalAttributeInterface);
        return mOriginalAttributeInterface->Read(aPath, aEncoder);
    }
    auto attr = device->GetAdminCommissioningAttributes();

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
