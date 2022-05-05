/*
 *
 *    Copyright (c) 2022 Project CHIP Authors
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
