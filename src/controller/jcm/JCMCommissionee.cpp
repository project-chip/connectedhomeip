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

#include "JCMCommissionee.h"

#include <app-common/zap-generated/ids/Attributes.h>
#include <app-common/zap-generated/ids/Clusters.h>
#include <app/AttributePathParams.h>
#include <app/CommandHandlerInterface.h>
#include <app/InteractionModelEngine.h>
#include <app/ReadClient.h>
#include <app/ReadPrepareParams.h>
#include <app/server/Server.h>
#include <lib/core/CHIPCore.h>
#include <lib/core/CHIPError.h>

using namespace ::chip;
using namespace ::chip::app;

namespace chip {
namespace Controller {
namespace JCM {

/*
 * DeviceCommissioner public interface and override implementation
 */
CHIP_ERROR JCMCommissionee::StartJCMTrustVerification(CommandHandlerInterface::HandlerContext & ctx, EndpointId endpointId,
                                                      JCMCommissionee::OnCompletionFunc onCompletion)
{
    // TODO
    // FabricIndex accessingFabricIndex = ctx.mCommandHandler.GetAccessingFabricIndex();
    // app::CommandHandler::Handle handle(&ctx.mCommandHandler);
    // chip::Messaging::ExchangeManager * exchangeMgr = &chip::Server::GetInstance().GetExchangeManager();

    // Save the endpoint ID in the JF Administrator server
    Server::GetInstance().GetJointFabricAdministrator().SetPeerJFAdminClusterEndpointId(endpointId);

    mOnCompletion = onCompletion;

    // TODO: Check that the RootPublicKey and FabricID of the accessing fabric (found in the FabricDescriptorStruct) match the
    // RootPublicKey and FabricID of the Fabric indicated by AdministratorFabricIndex.

    // TODO: Figure out where I get the vendor ID, rcacSpan, icacSpan, and nocSpan

    // CHIP_ERROR err = VerifyVendorId(exchangeMgr, handle, accessingFabricIndex, vendorId, rcacSpan, icacSpan, nocSpan);

    return CHIP_NO_ERROR;
}

CHIP_ERROR JCMCommissionee::OnLookupOperationalTrustAnchor(VendorId vendorID, CertificateKeyId & subjectKeyId,
                                                           ByteSpan & globallyTrustedRootSpan)
{
    // TODO I don't think any action needs to be taken here

    return CHIP_NO_ERROR;
}

void JCMCommissionee::OnVendorIdVerficationComplete(const CHIP_ERROR & err)
{
    if (this->mOnCompletion.has_value())
    {
        (*(this->mOnCompletion))(err);
    }
}

} // namespace JCM
} // namespace Controller
} // namespace chip
