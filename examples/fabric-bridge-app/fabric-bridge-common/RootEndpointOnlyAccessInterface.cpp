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

#include "RootEndpointOnlyAccessInterface.h"

using namespace chip;
using namespace chip::app;

namespace bridge {

CHIP_ERROR RootEndpointOnlyAccessInterface::Init()
{
    AttributeAccessInterface * oldInterface = AttributeAccessInterfaceRegistry::Instance().Get(kRootEndpointId, mClusterId);
    VerifyOrDie(oldInterface != this);
    VerifyOrReturnError(oldInterface != nullptr, CHIP_ERROR_INCORRECT_STATE);
    VerifyOrReturnError(mOriginalAttributeInterface == nullptr, CHIP_ERROR_INCORRECT_STATE);

    mOriginalAttributeInterface = oldInterface;
    AttributeAccessInterfaceRegistry::Instance().Unregister(mOriginalAttributeInterface);
    VerifyOrDie(AttributeAccessInterfaceRegistry::Instance().Register(this));

    return CHIP_NO_ERROR;
}

CHIP_ERROR RootEndpointOnlyAccessInterface::Shutdown()
{
    VerifyOrReturnError(mOriginalAttributeInterface != nullptr, CHIP_ERROR_INCORRECT_STATE);
    VerifyOrReturnError(AttributeAccessInterfaceRegistry::Instance().Get(kRootEndpointId, mClusterId) == this,
                        CHIP_ERROR_INCORRECT_STATE);

    AttributeAccessInterfaceRegistry::Instance().Unregister(this);

    // Existing applications do not clean up extra dynamic endpoints, so this
    // registration may fail (as it can be on a wildcard endpoint and if existing
    // endpoints already contain this cluster, the register fails)
    //
    // Concrete example:
    //   - AdministratorCommissioningCluster default instance is on * endpoint
    //   - devices register this cluster on explicit endpoints
    if (!AttributeAccessInterfaceRegistry::Instance().Register(mOriginalAttributeInterface))
    {
        ChipLogError(AppServer,
                     "Failed to re-register the default AttributeAccessInterface for cluster " ChipLogFormatMEI " for endpoint 0",
                     ChipLogValueMEI(mClusterId));
    }

    mOriginalAttributeInterface = nullptr;

    return CHIP_NO_ERROR;
}

} // namespace bridge
