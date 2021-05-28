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

#include <controller/ExampleOperationalCredentialsIssuer.h>
#include <credentials/CHIPCert.h>

namespace chip {
namespace Controller {

constexpr const char kOperationalCredentialsIssuerKeypairStorage[] = "ExampleOpCredsCAKey";

using namespace Credentials;
using namespace Crypto;

CHIP_ERROR ExampleOperationalCredentialsIssuer::Initialize(PersistentStorageDelegate & storage)
{
    Crypto::P256SerializedKeypair serializedKey;
    uint16_t keySize = static_cast<uint16_t>(sizeof(serializedKey));

    if (storage.SyncGetKeyValue(kOperationalCredentialsIssuerKeypairStorage, &serializedKey, keySize) != CHIP_NO_ERROR)
    {
        // Storage doesn't have an existing keypair. Let's create one and add it to the storage.
        ReturnErrorOnFailure(mIssuer.Initialize());
        ReturnErrorOnFailure(mIssuer.Serialize(serializedKey));

        keySize = static_cast<uint16_t>(sizeof(serializedKey));
        ReturnErrorOnFailure(storage.SyncSetKeyValue(kOperationalCredentialsIssuerKeypairStorage, &serializedKey, keySize));
    }
    else
    {
        // Use the keypair from the storage
        ReturnErrorOnFailure(mIssuer.Deserialize(serializedKey));
    }

    mInitialized = true;
    return CHIP_NO_ERROR;
}

CHIP_ERROR ExampleOperationalCredentialsIssuer::GenerateNodeOperationalCertificate(const PeerId & peerId, const ByteSpan & csr,
                                                                                   int64_t serialNumber, uint8_t * certBuf,
                                                                                   uint32_t certBufSize, uint32_t & outCertLen)
{
    VerifyOrReturnError(mInitialized, CHIP_ERROR_INCORRECT_STATE);
    X509CertRequestParams request = { serialNumber, mIssuerId,         mNow, mNow + mValidity, true, peerId.GetFabricId(),
                                      true,         peerId.GetNodeId() };

    P256PublicKey pubkey;
    ReturnErrorOnFailure(VerifyCertificateSigningRequest(csr.data(), csr.size(), pubkey));
    return NewNodeOperationalX509Cert(request, CertificateIssuerLevel::kIssuerIsRootCA, pubkey, mIssuer, certBuf, certBufSize,
                                      outCertLen);
}

CHIP_ERROR ExampleOperationalCredentialsIssuer::GetRootCACertificate(FabricId fabricId, uint8_t * certBuf, uint32_t certBufSize,
                                                                     uint32_t & outCertLen)
{
    VerifyOrReturnError(mInitialized, CHIP_ERROR_INCORRECT_STATE);
    X509CertRequestParams request = { 0, mIssuerId, mNow, mNow + mValidity, true, fabricId, false, 0 };
    return NewRootX509Cert(request, mIssuer, certBuf, certBufSize, outCertLen);
}

} // namespace Controller
} // namespace chip
