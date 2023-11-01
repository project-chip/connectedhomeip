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

#pragma once

#include "ICDStorageKeyDelegate.h"
#include <lib/core/CHIPCore.h>

#include <lib/core/TLV.h>
#include <lib/support/Pool.h>
#include <vector>

namespace chip {
namespace app {

class DefaultICDStorageKey : public ICDStorageKeyDelegate
{
public:
    void Init(size_t aMaxKeyCounter);

    StorageKeyName GetKey(size_t aIndex) override;

    size_t MaxKeyCounter() override { return mMaxKeyCounter; };

private:
    size_t mMaxKeyCounter = 0;
};
} // namespace app
} // namespace chip
