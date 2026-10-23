/*
 *    Copyright (c) 2026 Project CHIP Authors
 *    All rights reserved.
 */
#pragma once

#include <headers/ProvisionCryptoInterface.h>

namespace chip {
namespace DeviceLayer {
namespace Silabs {
namespace Provision {

class ProvisionCrypto final : public IProvisionCrypto
{
public:
    static ProvisionCrypto & GetInstance();

    ProvisionCrypto(const ProvisionCrypto &)             = delete;
    ProvisionCrypto & operator=(const ProvisionCrypto &) = delete;

    CHIP_ERROR GenerateRandom(MutableByteSpan & output) override;
    CHIP_ERROR Hash256(const ByteSpan & input, MutableByteSpan & output) override;
    CHIP_ERROR ImportDeviceAttestationKey(const ByteSpan & key) override;
    CHIP_ERROR GenerateDeviceAttestationCSR(uint16_t vid, uint16_t pid, const CharSpan & commonName,
                                            MutableCharSpan & csr) override;
    CHIP_ERROR SignWithDeviceAttestationKey(const ByteSpan & message, MutableByteSpan & signature) override;

private:
    ProvisionCrypto() = default;
};

} // namespace Provision
} // namespace Silabs
} // namespace DeviceLayer
} // namespace chip
