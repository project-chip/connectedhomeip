/*
 *
 * SPDX-FileCopyrightText: 2022 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
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

} // namespace DeviceLayer
} // namespace chip
