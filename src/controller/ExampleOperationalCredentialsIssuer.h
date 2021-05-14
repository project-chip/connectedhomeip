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

/**
 *  @file
 *    This file contains class definition of an example operational certificate
 *    issuer for CHIP devices. The class can be used as a guideline on how to
 *    construct your own certificate issuer. It can also be used in tests and tools
 *    if a specific signing authority is not required.
 */

#pragma once

#include <controller/OperationalCredentialsDelegate.h>
#include <core/CHIPError.h>
#include <core/CHIPPersistentStorageDelegate.h>
#include <crypto/CHIPCryptoPAL.h>
#include <support/CodeUtils.h>

namespace chip {
namespace Controller {

class DLL_EXPORT ExampleOperationalCredentialsIssuer : public OperationalCredentialsDelegate
{
public:
    virtual ~ExampleOperationalCredentialsIssuer() {}

    CHIP_ERROR GenerateNodeOperationalCertificate(const PeerId & peerId, const ByteSpan & csr, int64_t serialNumber,
                                                  uint8_t * certBuf, uint32_t certBufSize, uint32_t & outCertLen) override;

    CHIP_ERROR GetRootCACertificate(FabricId fabricId, uint8_t * certBuf, uint32_t certBufSize, uint32_t & outCertLen) override;

    /**
     * @brief Initialize the issuer with the keypair in the storage.
     *        If the storage doesn't have one, it'll create one, and it to the storage.
     *
     * @param[in] storage  A reference to the storage, where the keypair is stored.
     *                     The object of ExampleOperationalCredentialsIssuer doesn't hold
     *                     on the reference of storage.
     *
     * @return Returns a CHIP_ERROR on error, CHIP_NO_ERROR otherwise
     **/
    CHIP_ERROR Initialize(PersistentStorageDelegate & storage);

    void SetIssuerId(uint32_t id) { mIssuerId = id; }

    void SetCurrentEpoch(uint32_t epoch) { mNow = epoch; }

    void SetCertificateValidityPeriod(uint32_t validity) { mValidity = validity; }

private:
    Crypto::P256Keypair mIssuer;
    bool mInitialized  = false;
    uint32_t mIssuerId = 0;
    uint32_t mNow      = 0;

    // By default, let's set validity to 10 years
    uint32_t mValidity = 365 * 24 * 60 * 60 * 10;
};

} // namespace Controller
} // namespace chip
