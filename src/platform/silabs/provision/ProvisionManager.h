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
#pragma once

#include "ProvisionChannel.h"
#include "ProvisionProtocol.h"
#include "ProvisionStorage.h"
#include <lib/core/CHIPError.h>

namespace chip {
namespace DeviceLayer {
namespace Silabs {
namespace Provision {

class Manager
{
public:
    Manager() :
#ifdef SILABS_PROVISION_PROTOCOL_V1
        mProtocol1(mStore),
#endif
        mProtocol2(mStore)
    {}

    CHIP_ERROR Init();
    bool Step();
    bool IsProvisionRequired();
    CHIP_ERROR SetProvisionRequired(bool required);
    Storage & GetStorage() { return mStore; }
    static Manager & GetInstance();

private:
    bool ProcessCommand(ByteSpan & request, MutableByteSpan & response);

    Storage mStore;
    Channel mChannel;
#ifdef SILABS_PROVISION_PROTOCOL_V1
    Protocol1 mProtocol1;
#endif
    Protocol2 mProtocol2;
    bool mProvisionRequested = true;
};

} // namespace Provision
} // namespace Silabs
} // namespace DeviceLayer
} // namespace chip
