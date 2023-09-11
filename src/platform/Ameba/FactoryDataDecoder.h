/*
 *
 * SPDX-FileCopyrightText: 2022 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <platform/internal/CHIPDeviceLayerInternal.h>

namespace chip {
namespace DeviceLayer {

class FactoryDataDecoder
{
public:
    CHIP_ERROR ReadFactoryData(uint8_t * buffer, uint16_t * pfactorydata_len);
    CHIP_ERROR DecodeFactoryData(uint8_t * buffer, FactoryData * fdata, uint16_t factorydata_len);
    static FactoryDataDecoder & GetInstance()
    {
        static FactoryDataDecoder instance;
        return instance;
    }
};

} // namespace DeviceLayer
} // namespace chip
