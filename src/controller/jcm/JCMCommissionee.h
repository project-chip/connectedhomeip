/*
 *
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

#pragma once

#include "TrustVerification.h"

#include <app/CommandHandlerInterface.h>
#include <lib/core/CHIPCallback.h>
#include <lib/core/CHIPCore.h>
#include <lib/core/CHIPError.h>
#include <lib/core/DataModelTypes.h>

#include <functional>
#include <optional>

using namespace ::chip;
using namespace ::chip::app;

namespace chip {

namespace Controller {

namespace JCM {

/*
 * JCMCommissionee is a class that handles the Joint Commissioning Management (JCM) process for Joint Fabric Administrator devices
 * being commissioned in a CHIP network. It implements the JCM trust verification process.
 */
class JCMCommissionee : public VendorIdVerificationClient
{
public:
    JCMCommissionee() {}
    ~JCMCommissionee() {}

    using OnCompletionFunc = std::function<void(CHIP_ERROR)>;

    /*
     * StartJCMTrustVerification is a method that initiates the JCM trust verification process for the device.
     * It is called by the commissionee to start the trust verification process as part of HandleAnnounceJointFabricAdministrator.
     * The method will return an error if the inputs are invalid or if the trust verification process fails.
     *
     * @return CHIP_ERROR indicating success or failure of the operation.
     */
    CHIP_ERROR StartJCMTrustVerification(CommandHandlerInterface::HandlerContext & ctx, EndpointId endpointId,
                                         OnCompletionFunc onCompletion);

protected:
    CHIP_ERROR OnLookupOperationalTrustAnchor(VendorId vendorID, CertificateKeyId & subjectKeyId,
                                              ByteSpan & globallyTrustedRootSpan) override;
    void OnVendorIdVerficationComplete(const CHIP_ERROR & err) override;

private:
    std::optional<OnCompletionFunc> mOnCompletion = std::nullopt;
};

} // namespace JCM
} // namespace Controller
} // namespace chip
