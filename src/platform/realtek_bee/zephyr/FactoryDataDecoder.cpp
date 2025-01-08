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

#include "FactoryDataDecoder.h"
#include <platform/internal/CHIPDeviceLayerInternal.h>

// #include "chip_porting.h"
// #include "matter_utils.h"

namespace chip {
namespace DeviceLayer {

extern "C"
{

CHIP_ERROR FactoryDataDecoder::ReadFactoryData(uint8_t * buffer, uint16_t * pfactorydata_len)
{
    uint32_t ret = 0;
    //ret          = ReadFactory(buffer, pfactorydata_len);
    if (ret != 0)
        return CHIP_ERROR_INTERNAL;

    return CHIP_NO_ERROR;
}

CHIP_ERROR FactoryDataDecoder::DecodeFactoryData(uint8_t * buffer, FactoryData * fdata, uint16_t factorydata_len)
{
    uint32_t ret = 0;
    //ret          = DecodeFactory(buffer, fdata, factorydata_len);
    if (ret != 0)
        return CHIP_ERROR_INTERNAL;

    return CHIP_NO_ERROR;
}

}
} // namespace DeviceLayer
} // namespace chip
