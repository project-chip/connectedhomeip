/*
 *
 *    Copyright (c) 2024 Project CHIP Authors
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

#include <esp_err.h>
#include <esp_secure_cert_tlv_config.h>
#include <esp_secure_cert_tlv_read.h>

#include <lib/core/CHIPEncoding.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/Span.h>
#include <lib/support/logging/CHIPLogging.h>
#include <platform/ESP32/ESP32SecureCertDataProvider.h>

namespace chip {
namespace DeviceLayer {

namespace {
// Define Matter-specific TLV subtypes for fields in the secure cert partition
enum class MatterTLVSubType : uint8_t
{
    kSetupDiscriminator       = 0,
    kSpake2pVerifier          = 1,
    kSpake2pSalt              = 2,
    kSpake2pIterationCount    = 3,
    kRotatingDeviceIdUniqueId = 4,
};

// Scoped wrapper class for handling TLV data retrieval from secure cert partition
class ScopedTLVInfo
{
public:
    ScopedTLVInfo(MatterTLVSubType subType) : mTLVConfig(GetMatterTLVConfig(subType)) {}

    ~ScopedTLVInfo() { esp_secure_cert_free_tlv_info(&mTLVInfo); }

    CHIP_ERROR GetValue(MutableByteSpan & span)
    {
        ReturnErrorOnFailure(PopulateTLVInfo());
        VerifyOrReturnError(mTLVInfo.length <= span.size(), CHIP_ERROR_BUFFER_TOO_SMALL);

        memcpy(span.data(), mTLVInfo.data, mTLVInfo.length);
        span.reduce_size(mTLVInfo.length);

        return CHIP_NO_ERROR;
    }

    CHIP_ERROR GetValue(uint16_t & value)
    {
        ReturnErrorOnFailure(PopulateTLVInfo());
        VerifyOrReturnError(mTLVInfo.length == sizeof(uint16_t), CHIP_ERROR_INTERNAL,
                            ChipLogError(DeviceLayer, "secure cert tlv size mismatch"));

        value = Encoding::LittleEndian::Get16(reinterpret_cast<const uint8_t *>(mTLVInfo.data));
        ChipLogDetail(DeviceLayer, "secure cert tlv value:%u", value);
        return CHIP_NO_ERROR;
    }

    CHIP_ERROR GetValue(uint32_t & value)
    {
        ReturnErrorOnFailure(PopulateTLVInfo());
        VerifyOrReturnError(mTLVInfo.length == sizeof(uint32_t), CHIP_ERROR_INTERNAL,
                            ChipLogError(DeviceLayer, "secure cert tlv size mismatch"));

        value = Encoding::LittleEndian::Get32(reinterpret_cast<const uint8_t *>(mTLVInfo.data));
        ChipLogDetail(DeviceLayer, "secure cert tlv value:%" PRIu32, value);
        return CHIP_NO_ERROR;
    }

private:
    esp_secure_cert_tlv_info_t mTLVInfo;
    esp_secure_cert_tlv_config_t mTLVConfig;

    esp_secure_cert_tlv_config_t GetMatterTLVConfig(MatterTLVSubType subType)
    {
        return { .type = ESP_SECURE_CERT_MATTER_TLV_1, .subtype = static_cast<esp_secure_cert_tlv_subtype_t>(subType) };
    }

    CHIP_ERROR PopulateTLVInfo()
    {
        esp_err_t err = esp_secure_cert_get_tlv_info(&mTLVConfig, &mTLVInfo);
        VerifyOrReturnError(err == ESP_OK, CHIP_ERROR_INTERNAL,
                            ChipLogError(DeviceLayer, "esp_secure_cert_get_tlv_info failed: type:%u, subtype:%u, err:%d",
                                         mTLVConfig.type, mTLVConfig.subtype, err));

        ChipLogDetail(DeviceLayer, "secure cert tlv info type:%u subtype:%u length:%" PRIu32 " flags:%u", mTLVInfo.type,
                      mTLVInfo.subtype, mTLVInfo.length, mTLVInfo.flags);

        return CHIP_NO_ERROR;
    }
};

} // anonymous namespace

CHIP_ERROR ESP32SecureCertDataProvider::GetSetupDiscriminator(uint16_t & setupDiscriminator)
{
    ScopedTLVInfo tlvInfo(MatterTLVSubType::kSetupDiscriminator);
    return tlvInfo.GetValue(setupDiscriminator);
}

CHIP_ERROR ESP32SecureCertDataProvider::GetSpake2pIterationCount(uint32_t & iterationCount)
{
    ScopedTLVInfo tlvInfo(MatterTLVSubType::kSpake2pIterationCount);
    return tlvInfo.GetValue(iterationCount);
}

CHIP_ERROR ESP32SecureCertDataProvider::GetSpake2pSalt(MutableByteSpan & saltBuf)
{
    ScopedTLVInfo tlvInfo(MatterTLVSubType::kSpake2pSalt);
    return tlvInfo.GetValue(saltBuf);
}

CHIP_ERROR ESP32SecureCertDataProvider::GetSpake2pVerifier(MutableByteSpan & verifierBuf, size_t & verifierLen)
{
    ScopedTLVInfo tlvInfo(MatterTLVSubType::kSpake2pVerifier);
    ReturnErrorOnFailure(tlvInfo.GetValue(verifierBuf));
    verifierLen = verifierBuf.size();
    return CHIP_NO_ERROR;
}

CHIP_ERROR ESP32SecureCertDataProvider::GetRotatingDeviceIdUniqueId(MutableByteSpan & uniqueIdSpan)
{
#if CHIP_ENABLE_ROTATING_DEVICE_ID
    ScopedTLVInfo tlvInfo(MatterTLVSubType::kRotatingDeviceIdUniqueId);
    return tlvInfo.GetValue(uniqueIdSpan);
#else
    return CHIP_ERROR_NOT_IMPLEMENTED;
#endif // CHIP_ENABLE_ROTATING_DEVICE_ID
}

} // namespace DeviceLayer
} // namespace chip
