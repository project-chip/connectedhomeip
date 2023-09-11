/*
 *
 * SPDX-FileCopyrightText: 2022 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "DeviceInstanceInfoProviderImpl.h"

#include "PosixConfig.h"

namespace chip {
namespace DeviceLayer {

CHIP_ERROR DeviceInstanceInfoProviderImpl::GetVendorId(uint16_t & vendorId)
{
    return Internal::PosixConfig::ReadConfigValue(Internal::PosixConfig::kConfigKey_VendorId, vendorId);
}

CHIP_ERROR DeviceInstanceInfoProviderImpl::GetProductId(uint16_t & productId)
{
    return Internal::PosixConfig::ReadConfigValue(Internal::PosixConfig::kConfigKey_ProductId, productId);
}

} // namespace DeviceLayer
} // namespace chip
