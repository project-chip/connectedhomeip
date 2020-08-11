/*
 *
 *    Copyright (c) 2013-2017 Nest Labs, Inc.
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
 *    @file
 *      Utility functions for generating verification hashes of provisioning information
 *
 */

#include <core/CHIPCore.h>
#include <support/CodeUtils.h>
#include <support/crypto/HashAlgos.h>
#include <support/Base64.h>

namespace chip {
namespace Profiles {
namespace Security {

enum {
    kSHA256Base64Length                 = (((chip::Platform::Security::SHA256::kHashLength + 2) / 3) * 4),

    kChipProvisioningHashLength        = kSHA256Base64Length,

    kDeviceCredentialHashLength         = kSHA256Base64Length
};

DLL_EXPORT CHIP_ERROR MakeChipProvisioningHash(uint64_t nodeId,
                                                    const char *weaveCert, size_t weaveCertLen,
                                                    const char *weavePrivKey, size_t weavePrivKeyLen,
                                                    const char *pairingCode, size_t weavePairingCodeLen,
                                                    char *hashBuf, size_t hashBufSize);

DLL_EXPORT CHIP_ERROR MakeDeviceCredentialHash(const char *serialNum, size_t serialNumLen,
                                                   const char *deviceId, size_t deviceIdLen,
                                                   const char *deviceSecret, size_t deviceSecretLen,
                                                   char *hashBuf, size_t hashBufSize);

} // namespace Security
} // namespace Profiles
} // namespace chip
