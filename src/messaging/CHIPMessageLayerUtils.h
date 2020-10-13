/*
 *    Copyright (c) 2020 Project CHIP Authors
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
 *      This file defines the utility class for the CHIP Message Layer.
 *
 */

#ifndef CHIP_MESSAGE_LAYER_UTILS_H
#define CHIP_MESSAGE_LAYER_UTILS_H

#include <core/CHIPError.h>
#include <support/DLLUtil.h>

namespace chip {

class DLL_EXPORT ChipMessageLayerUtils
{
public:
    static CHIP_ERROR FilterUDPSendError(CHIP_ERROR err, bool isMulticast);
    static bool IsIgnoredMulticastSendError(CHIP_ERROR err);
    static bool IsSendErrorNonCritical(CHIP_ERROR err);
};

} // namespace chip

#endif // CHIP_MESSAGE_LAYER_UTILS_H
