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
#include "chip_porting.h"
#include <platform/Ameba/AmebaUtils.h>
#include <platform/internal/CHIPDeviceLayerInternal.h>

using namespace chip::DeviceLayer::Internal;

namespace chip {
namespace DeviceLayer {

CHIP_ERROR FactoryDataDecoder::ReadFactoryData(uint8_t * buffer, uint16_t * pfactorydata_len)
{
    int32_t error  = ReadFactory(buffer, pfactorydata_len);
    CHIP_ERROR err = AmebaUtils::MapError(error, AmebaErrorType::kFlashError);

    return err;
}

CHIP_ERROR FactoryDataDecoder::DecodeFactoryData(uint8_t * buffer, FactoryData * fdata, uint16_t factorydata_len)
{
    int32_t error  = DecodeFactory(buffer, fdata, factorydata_len);
    CHIP_ERROR err = AmebaUtils::MapError(error, AmebaErrorType::kFlashError);

    return err;
}

#if CONFIG_ENABLE_AMEBA_CRYPTO
CHIP_ERROR FactoryDataDecoder::GetSign(uint8_t * PublicKeyData, size_t PublicKeySize, const unsigned char * MessageData,
                                       size_t MessageSize, unsigned char * Signature)
{
    int32_t error  = matter_get_signature(PublicKeyData, PublicKeySize, MessageData, MessageSize, Signature);
    CHIP_ERROR err = CHIP_NO_ERROR;

    return err;
}
#endif

} // namespace DeviceLayer
} // namespace chip
