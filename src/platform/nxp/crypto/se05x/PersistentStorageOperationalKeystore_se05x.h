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
 *          Platform-specific implementation of the persistent operational key store for SE05x
 */

#pragma once

#include <crypto/PersistentStorageOperationalKeystore.h>

namespace chip {

/**
 * @brief PersistentStorageOpKeystorese05x implementation making use of PersistentStorageOperationalKeystore
 *        to create/remove node operational key pairs in SE05x secure element.
 *
 */
class PersistentStorageOpKeystorese05x : public chip::PersistentStorageOperationalKeystore
{
public:
    CHIP_ERROR NewOpKeypairForFabric(FabricIndex fabricIndex, MutableByteSpan & outCertificateSigningRequest) override;
    CHIP_ERROR RemoveOpKeypairForFabric(FabricIndex fabricIndex) override;
};

} // namespace chip
