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
#include <crypto/CHIPCryptoPAL.h>
#include <support/DLLUtil.h>
#include <support/Span.h>
#include <transport/raw/MessageHeader.h>

namespace chip {
namespace Controller {

/// Callbacks for CHIP operational credentials generation
class DLL_EXPORT OperationalCredentialsDelegate
{
public:
    virtual ~OperationalCredentialsDelegate() {}

    /**
     * @brief
     *   This function generates an operational certificate for the given node.
     *   The API generates the certificate in X.509 DER format.
     *
     *   The delegate is expected to use the certificate authority whose certificate
     *   is returned in `GetIntermediateCACertificate()` or `GetRootCACertificate()`
     *   API calls.
     *
     * @param[in] nodeId       Node ID of the target device.
     * @param[in] fabricId     Fabric ID for which the credentials are being generated.
     * @param[in] csr          Certificate Signing Request from the node in DER format.
     * @param[in] serialNumber Serial number to assign to the new certificate.
     * @param[in] certBuf      The API will fill in the generated cert in this buffer. The buffer is allocated by the caller.
     * @param[in] certBufSize  The size of certBuf buffer.
     * @param[out] outCertLen  The size of the actual certificate that was written in the certBuf.
     *
     * @return CHIP_ERROR CHIP_NO_ERROR on success, or corresponding error code.
     */
    virtual CHIP_ERROR GenerateNodeOperationalCertificate(NodeId nodeId, FabricId fabricId, const ByteSpan & csr,
                                                          int64_t serialNumber, uint8_t * certBuf, uint32_t certBufSize,
                                                          uint32_t & outCertLen) = 0;

    /**
     * @brief
     *   This function returns the intermediate certificate authority (ICA) certificate corresponding to the
     *   provided fabric ID. Intermediate certificate authority is optional. If the controller
     *   application does not require ICA, this API call will return `CHIP_ERROR_NOT_IMPLEMENTED`.
     *
     *   The returned certificate is in X.509 DER format.
     *
     * @param[in] fabricId    Fabric ID for which the certificate is being requested.
     * @param[in] certBuf     The API will fill in the cert in this buffer. The buffer is allocated by the caller.
     * @param[in] certBufSize The size of certBuf buffer.
     * @param[out] outCertLen The size of the actual certificate that was written in the certBuf.
     *
     * @return CHIP_ERROR CHIP_NO_ERROR on success, or corresponding error code.
     *         CHIP_ERROR_NOT_IMPLEMENTED is not a critical error. It indicates that ICA is not needed.
     */
    virtual CHIP_ERROR GetIntermediateCACertificate(FabricId fabricId, uint8_t * certBuf, uint32_t certBufSize,
                                                    uint32_t & outCertLen)
    {
        return CHIP_ERROR_NOT_IMPLEMENTED;
    }

    /**
     * @brief
     *   This function returns the root certificate authority (root CA) certificate corresponding to the
     *   provided fabric ID.
     *
     *   The returned certificate is in X.509 DER format.
     *
     * @param[in] fabricId    Fabric ID for which the certificate is being requested.
     * @param[in] certBuf     The API will fill in the cert in this buffer. The buffer is allocated by the caller.
     * @param[in] certBufSize The size of certBuf buffer.
     * @param[out] outCertLen The size of the actual certificate that was written in the certBuf.
     *
     * @return CHIP_ERROR CHIP_NO_ERROR on success, or corresponding error code.
     */
    virtual CHIP_ERROR GetRootCACertificate(FabricId fabricId, uint8_t * certBuf, uint32_t certBufSize, uint32_t & outCertLen) = 0;
};

} // namespace Controller
} // namespace chip
