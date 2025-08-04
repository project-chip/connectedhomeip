/*
 *    Copyright (c) 2024 Project CHIP Authors
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

#include "OperationalKeystore.h"

chip::Crypto::OperationalKeystore * chip::NXP::App::OperationalKeystore::GetInstance()
{
    // Default implementation returns a null pointer.
    // Applications that have a specific operational keystore defined should implement
    // this function and return a pointer to that instance.
    return nullptr;
}

CHIP_ERROR chip::NXP::App::OperationalKeystore::Init(PersistentStorageDelegate * delegate)
{
    return CHIP_NO_ERROR;
}
