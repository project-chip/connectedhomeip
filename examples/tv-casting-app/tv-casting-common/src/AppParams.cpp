/*
 *
 *    Copyright (c) 2022 Project CHIP Authors
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

#include "AppParams.h"

#if CHIP_ENABLE_ROTATING_DEVICE_ID
AppParams::AppParams(chip::Optional<chip::ByteSpan> rotatingDeviceIdUniqueId)
{
    mRotatingDeviceIdUniqueId = rotatingDeviceIdUniqueId;
}

void AppParams::SetRotatingDeviceIdUniqueId(chip::Optional<chip::ByteSpan> rotatingDeviceIdUniqueId)
{
    mRotatingDeviceIdUniqueId = rotatingDeviceIdUniqueId;
}

chip::Optional<chip::ByteSpan> AppParams::GetRotatingDeviceIdUniqueId()
{
    return mRotatingDeviceIdUniqueId;
}
#endif // CHIP_ENABLE_ROTATING_DEVICE_ID
