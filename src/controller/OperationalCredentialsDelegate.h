/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
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

#include <app/util/basic-types.h>
#include <core/CHIPCallback.h>
#include <core/PeerId.h>
#include <crypto/CHIPCryptoPAL.h>
#include <support/DLLUtil.h>
#include <support/Span.h>
#include <transport/raw/MessageHeader.h>

namespace chip {
namespace Controller {

typedef void (*NOCGenerated)(void * context, const ByteSpan & noc);

constexpr uint32_t kMaxCHIPDERCertLength = 600;

/// Callbacks for CHIP operational credentials generation
class DLL_EXPORT OperationalCredentialsDelegate
{
public:
    virtual ~OperationalCredentialsDelegate() {}

    /**
     * @brief
     *   This function generates an operational certificate for the device.
     *   The API generates the certificate in X.509 DER format.
     *
     *   The delegate is expected to use the certificate authority whose certificate
     *   is returned in `GetIntermediateCACertificate()` or `GetRootCACertificate()`
     *   API calls.
     *
     *   The delegate will call `onNOCGenerated` when the NOC is ready.
     *
     * @param[in] nodeId          Optional node ID. If provided, the generated NOC must use the provided ID.
     *                            If ID is not provided, the delegate must generate one.
     * @param[in] fabricId        Fabric ID for which the certificate is being requested.
     * @param[in] csr             Certificate Signing Request from the node in DER format.
     * @param[in] DAC             Device attestation certificate received from the device being commissioned
     * @param[in] onNOCGenerated  Callback handler to provide generated NOC to the caller of GenerateNodeOperationalCertificate()
     *
     * @return CHIP_ERROR CHIP_NO_ERROR on success, or corresponding error code.
     */
    virtual CHIP_ERROR GenerateNodeOperationalCertificate(const Optional<NodeId> & nodeId, FabricId fabricId, const ByteSpan & csr,
                                                          const ByteSpan & DAC,
                                                          Callback::Callback<NOCGenerated> * onNOCGenerated) = 0;

    /**
     * @brief
     *   This function returns the intermediate certificate authority (ICA) certificate corresponding to the
     *   provided fabric ID. Intermediate certificate authority is optional. If the controller
     *   application does not require ICA, this API call will return `CHIP_ERROR_INTERMEDIATE_CA_NOT_REQUIRED`.
     *
     *   The returned certificate is in X.509 DER format.
     *
     * @param[in] fabricId    Fabric ID for which the certificate is being requested.
     * @param[in] outCert     The API will fill in the cert in this buffer. The buffer is allocated by the caller.
     *
     * @return CHIP_ERROR CHIP_NO_ERROR on success, or corresponding error code.
     *         CHIP_ERROR_INTERMEDIATE_CA_NOT_REQUIRED is not a critical error. It indicates that ICA is not needed.
     */
    virtual CHIP_ERROR GetIntermediateCACertificate(FabricId fabricId, MutableByteSpan & outCert)
    {
        // By default, let's return CHIP_ERROR_INTERMEDIATE_CA_NOT_REQUIRED status. It'll allow
        // commissioner applications to not implement GetIntermediateCACertificate() if they don't require an
        // intermediate CA.
        return CHIP_ERROR_INTERMEDIATE_CA_NOT_REQUIRED;
    }

    /**
     * @brief
     *   This function returns the root certificate authority (root CA) certificate corresponding to the
     *   provided fabric ID.
     *
     *   The returned certificate is in X.509 DER format.
     *
     * @param[in] fabricId    Fabric ID for which the certificate is being requested.
     * @param[in] outCert     The API will fill in the cert in this buffer. The buffer is allocated by the caller.
     *
     * @return CHIP_ERROR CHIP_NO_ERROR on success, or corresponding error code.
     */
    virtual CHIP_ERROR GetRootCACertificate(FabricId fabricId, MutableByteSpan & outCert) = 0;
};

} // namespace Controller
} // namespace chip
