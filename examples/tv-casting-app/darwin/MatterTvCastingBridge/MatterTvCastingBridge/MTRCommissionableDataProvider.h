/*
 *
 * SPDX-FileCopyrightText: 2023 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#import "MTRDataSource.h"

#include <lib/core/CHIPError.h>
#include <lib/core/Optional.h>
#include <platform/CommissionableDataProvider.h>
#include <stdint.h>
#include <vector>

#ifndef MTRCommissionableDataProvider_h
#define MTRCommissionableDataProvider_h

namespace matter {
namespace casting {
namespace support {

class MTRCommissionableDataProvider : public chip::DeviceLayer::CommissionableDataProvider
{
public:
    CHIP_ERROR Initialize(id<MTRDataSource> dataSource);
    CHIP_ERROR GetSetupDiscriminator(uint16_t & setupDiscriminator) override;
    CHIP_ERROR SetSetupDiscriminator(uint16_t setupDiscriminator) override
    {
        // We don't support overriding the discriminator post-init (it is deprecated!)
        return CHIP_ERROR_NOT_IMPLEMENTED;
    }
    CHIP_ERROR GetSpake2pIterationCount(uint32_t & iterationCount) override;
    CHIP_ERROR GetSpake2pSalt(chip::MutableByteSpan & saltBuf) override;
    CHIP_ERROR GetSpake2pVerifier(chip::MutableByteSpan & verifierBuf, size_t & outVerifierLen) override;
    CHIP_ERROR GetSetupPasscode(uint32_t & setupPasscode) override;
    CHIP_ERROR SetSetupPasscode(uint32_t setupPasscode) override
    {
        // We don't support overriding the passcode post-init (it is deprecated!)
        return CHIP_ERROR_NOT_IMPLEMENTED;
    }

private:
    id<MTRDataSource> mDataSource = nullptr;

    bool mFirstUpdated = false;
    std::vector<uint8_t> mSerializedPaseVerifier;
    std::vector<uint8_t> mPaseSalt;
    uint32_t mPaseIterationCount = 0;
    chip::Optional<uint32_t> mSetupPasscode;
    uint16_t mDiscriminator = 0;
};

}; // namespace support
}; // namespace casting
}; // namespace matter

#endif /* MTRCommissionableDataProvider_h */
