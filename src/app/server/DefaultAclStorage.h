/*
 *
 * SPDX-FileCopyrightText: 2022 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <app/server/AclStorage.h>

namespace chip {
namespace app {

class DefaultAclStorage : public AclStorage
{
public:
    /**
     * Initialize must be called. It loads ACL entries for all fabrics from persistent storage,
     * then installs a listener for the access control system module to maintain ACL entries in
     * persistent storage so they remain in sync with entries in the access control system module.
     */
    CHIP_ERROR Init(PersistentStorageDelegate & persistentStorage, ConstFabricIterator first, ConstFabricIterator last) override;
};

} // namespace app
} // namespace chip
