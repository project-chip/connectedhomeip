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

/**
 *    @file
 *          Platform-specific implementation of the persistent operational keystore for rw61x
 */

#pragma once

#include <crypto/OperationalKeystore.h>
#include <crypto/PersistentStorageOperationalKeystore.h>
#include <lib/core/CHIPError.h>
#include <lib/core/CHIPPersistentStorageDelegate.h>
#include <lib/core/DataModelTypes.h>
#include <lib/support/CHIPMem.h>
#include <lib/support/CodeUtils.h>

#include <platform/nxp/common/crypto/PersistentStorageOpKeystoreBase.h>

namespace chip {

/**
 * @brief OperationalKeystore implementation making use of PersistentStorageDelegate
 *        to load/store keypairs. This is the legacy behavior of `FabricTable` prior
 *        to refactors to use `OperationalKeystore` and exists as a baseline example
 *        of how to use the interface.
 *
 */
class PersistentStorageOpKeystoreS50 : public PersistentStorageOpKeystoreNXP
{
public:
    PersistentStorageOpKeystoreS50() = default;
    virtual ~PersistentStorageOpKeystoreS50() { Finish(); }

    // Non-copyable
    PersistentStorageOpKeystoreS50(PersistentStorageOpKeystoreS50 const &) = delete;
    void operator=(PersistentStorageOpKeystoreS50 const &)                 = delete;

    CHIP_ERROR SignWithOpKeypair(FabricIndex fabricIndex, const ByteSpan & message,
                                 Crypto::P256ECDSASignature & outSignature) const override;
};
} // namespace chip
