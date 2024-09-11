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
#include <platform/nxp/common/crypto/PersistentStorageOpKeystoreS200.h>

static chip::PersistentStorageOpKeystoreS200 sInstance;

chip::Crypto::OperationalKeystore * chip::NXP::App::OperationalKeystore::GetInstance()
{
    return &sInstance;
}

CHIP_ERROR chip::NXP::App::OperationalKeystore::Init(PersistentStorageDelegate * delegate)
{
    VerifyOrReturnError(delegate != nullptr, CHIP_ERROR_INTERNAL);
    sInstance.Init(delegate);

    return CHIP_NO_ERROR;
}
