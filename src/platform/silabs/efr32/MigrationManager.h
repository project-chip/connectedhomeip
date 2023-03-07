/*
 *    Copyright (c) 2023 Project CHIP Authors
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

#include <stdint.h>
#include <stdio.h>

namespace chip {
namespace DeviceLayer {
namespace EFR32 {

class EFR32Migration
{
    friend class KeyValueStoreManagerImpl;

public:
    /**
     * The EFR32 migration manager is implemented as a singleton
     * User should get the object from this getter.
     */
    static EFR32Migration & GetMigrationManager();
    static void applyMigrations();

private:
    EFR32Migration(){};
    ~EFR32Migration(){};
};

} // namespace EFR32
} // namespace DeviceLayer
} // namespace chip
