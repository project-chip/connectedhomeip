#include "ProvisionStorage.h"
#include "ProvisionProtocol.h"
#include <CHIPProjectConfig.h>
#include <lib/core/CHIPEncoding.h>
#include <lib/support/Base64.h>
#include <lib/support/BytesToHex.h>
#include <crypto/CHIPCryptoPAL.h>
#include <platform/CHIPDeviceConfig.h>
#include <platform/CHIPDeviceError.h>
#include <setup_payload/Base38Encode.h>
#include <setup_payload/SetupPayload.h>

#include <nvm3.h>
#include <nvm3_default.h>
#include <nvm3_hal_flash.h>

using namespace chip::Encoding;
using namespace chip::DeviceLayer::Silabs::Provision::Parameters;

namespace chip {
namespace DeviceLayer {
namespace Silabs {
namespace Provision {

namespace {
constexpr uint8_t kDefaultCommissioningFlow = static_cast<uint8_t>(chip::CommissioningFlow::kCustom);
constexpr uint8_t kDefaultRendezvousFlags = static_cast<uint8_t>(chip::RendezvousInformationFlag::kSoftAP) |  static_cast<uint8_t>(chip::RendezvousInformationFlag::kOnNetwork);
} // namespace

uint8_t Storage::aux_buffer[Storage::kArgumentSizeMax];


size_t WriteQR(uint8_t bits[], size_t offset, uint32_t input, size_t numberOfBits, size_t totalPayloadSizeInBits)
{
    VerifyOrReturnError((offset + numberOfBits) <= totalPayloadSizeInBits, 0);
    size_t index = offset;
    offset += numberOfBits;
    while (input != 0)
    {
        if (input & 1)
        {
            bits[int(index / 8)] |= (1 << (index % 8));
        }
        index += 1;
        input >>= 1;
    }
    return offset;
}

void GenerateQR(uint8_t bits[], uint32_t vendor_id, uint32_t product_id,
        uint32_t commissioning_flow, uint32_t rendezvous_flag, uint32_t discriminator, uint32_t passcode)
{
    size_t offset = 0;

    offset = WriteQR(bits, offset, 0, kVersionFieldLengthInBits, kTotalPayloadDataSizeInBits);
    offset = WriteQR(bits, offset, vendor_id, kVendorIDFieldLengthInBits, kTotalPayloadDataSizeInBits);
    offset = WriteQR(bits, offset, product_id, kProductIDFieldLengthInBits, kTotalPayloadDataSizeInBits);
    offset = WriteQR(bits, offset, commissioning_flow, kCommissioningFlowFieldLengthInBits, kTotalPayloadDataSizeInBits);
    offset = WriteQR(bits, offset, rendezvous_flag, kRendezvousInfoFieldLengthInBits, kTotalPayloadDataSizeInBits);
    offset = WriteQR(bits, offset, discriminator, kPayloadDiscriminatorFieldLengthInBits, kTotalPayloadDataSizeInBits);
    offset = WriteQR(bits, offset, passcode, kSetupPINCodeFieldLengthInBits, kTotalPayloadDataSizeInBits);
    offset = WriteQR(bits, offset, 0, kPaddingFieldLengthInBits, kTotalPayloadDataSizeInBits);
}

//
// Generic Interface
//

CHIP_ERROR Storage::Set(uint16_t id, const uint8_t *value)
{
    switch(id)
    {
    case Parameters::ID::kCommissioningFlow:
        mCommissioningFlow = (nullptr == value) ? kDefaultCommissioningFlow : *value;
        break;
    case Parameters::ID::kRendezvousFlags:
        mRendezvousFlags = (nullptr == value) ? kDefaultRendezvousFlags : *value;
        break;
    default:
        return CHIP_ERROR_UNKNOWN_RESOURCE_ID;
    }
    return CHIP_NO_ERROR;
}

CHIP_ERROR Storage::Get(uint16_t id, uint8_t &value)
{
    switch(id)
    {
    case Parameters::ID::kCommissioningFlow:
        value = mCommissioningFlow;
        break;
    case Parameters::ID::kRendezvousFlags:
        value = mRendezvousFlags;
        break;
    default:
        return CHIP_ERROR_UNKNOWN_RESOURCE_ID;
    }
    return CHIP_NO_ERROR;
}


CHIP_ERROR Storage::Set(uint16_t id, const uint16_t *value)
{
    switch(id)
    {
    case Parameters::ID::kVendorId:
        // Vendor ID is mandatory
        VerifyOrReturnError(nullptr != value, CHIP_ERROR_INVALID_ARGUMENT);
        mVendorId = *value;
        return SetVendorId(mVendorId);

    case Parameters::ID::kProductId:
        // Product ID is mandatory
        VerifyOrReturnError(nullptr != value, CHIP_ERROR_INVALID_ARGUMENT);
        mProductId = *value;
        return SetProductId(mProductId);

    case Parameters::ID::kHwVersion:
        mHwVersion = (nullptr == value) ? 0 : *value;
        return SetHardwareVersion(mHwVersion);

    case Parameters::ID::kDiscriminator:
        if((nullptr == value))
        {
            // Auto-generate
            do
            {
                chip::Crypto::DRBG_get_bytes((uint8_t *) &mDiscriminator, sizeof(mDiscriminator));
            }
            while(mDiscriminator > 0x0fff);
        }
        else
        {
            VerifyOrReturnError(*value <= 0xffff, CHIP_ERROR_INVALID_ARGUMENT);
            mDiscriminator = *value;
        }
        return SetSetupDiscriminator(mDiscriminator);
    }
    return CHIP_ERROR_UNKNOWN_RESOURCE_ID;
}

CHIP_ERROR Storage::Get(uint16_t id, uint16_t &value)
{
    switch(id)
    {
    case Parameters::ID::kVendorId:
        return GetVendorId(value);

    case Parameters::ID::kProductId:
        return GetProductId(value);

    case Parameters::ID::kHwVersion:
        return GetHardwareVersion(value);

    case Parameters::ID::kDiscriminator:
        return GetSetupDiscriminator(value);
    }
    return CHIP_ERROR_UNKNOWN_RESOURCE_ID;
}


CHIP_ERROR Storage::Set(uint16_t id, const uint32_t *value)
{
    switch(id)
    {
    case Parameters::ID::kSpake2pIterations:
        VerifyOrReturnError(nullptr != value, CHIP_ERROR_INVALID_ARGUMENT);
        VerifyOrReturnError(*value >= chip::Crypto::kSpake2p_Min_PBKDF_Iterations, CHIP_ERROR_INVALID_ARGUMENT);
        VerifyOrReturnError(*value <= chip::Crypto::kSpake2p_Max_PBKDF_Iterations, CHIP_ERROR_INVALID_ARGUMENT);
        return SetSpake2pIterationCount(*value);

    case Parameters::ID::kSpake2pPasscode:
        // Since the verifier is calculated by the script, the passcode must come from the script too
        mPasscode = (nullptr == value) ? 0 : *value;
        return CHIP_NO_ERROR;

    case Parameters::ID::kKeyId:
        mKeyId = (nullptr == value) ? 0 : *value;
        return CHIP_NO_ERROR;
    }
    return CHIP_ERROR_UNKNOWN_RESOURCE_ID;
}


CHIP_ERROR Storage::Get(uint16_t id, uint32_t &value)
{
    switch(id)
    {
    case Parameters::ID::kSpake2pIterations:
        return GetSpake2pIterationCount(value);

    case Parameters::ID::kSpake2pPasscode:
        value = mPasscode;
        break;
    default:
        return CHIP_ERROR_UNKNOWN_RESOURCE_ID;
    }
    return CHIP_NO_ERROR;
}


CHIP_ERROR Storage::Set(uint16_t id, const uint8_t *value, size_t size)
{
    switch(id)
    {
    case Parameters::ID::kVersion:
        return SetProvisionVersion((const char*)value, size);

    case Parameters::ID::kSerialNumber:
        if((nullptr == value) || (0 == size))
        {
            return SetSerialNumber("", 0);
        }
        else
        {
            return SetSerialNumber((const char*)value, size);
        }

    case Parameters::ID::kVendorName:
        if((nullptr == value) || (0 == size))
        {
            return SetVendorName("", 0);
        }
        else
        {
            return SetVendorName((const char*)value, size);
        }

    case Parameters::ID::kProductName:
        if((nullptr == value) || (0 == size))
        {
            return SetProductName("", 0);
        }
        else
        {
            return SetProductName((const char*)value, size);
        }

    case Parameters::ID::kProductLabel:
        if((nullptr == value) || (0 == size))
        {
            return SetProductLabel("", 0);
        }
        else
        {
            return SetProductLabel((const char*)value, size);
        }

    case Parameters::ID::kProductUrl:
        if((nullptr == value) || (0 == size))
        {
            return SetProductURL("", 0);
        }
        else
        {
            return SetProductURL((const char*)value, size);
        }

    case Parameters::ID::kPartNumber:
        if((nullptr == value) || (0 == size))
        {
            return SetPartNumber("", 0);
        }
        else
        {
            return SetPartNumber((const char*)value, size);
        }

    case Parameters::ID::kHwVersionStr:
        if((nullptr == value) || (0 == size))
        {
            char temp[kHardwareVersionStrLengthMax] = { 0 };
            size_t temp_size = 0;
            temp_size = snprintf((char*)temp, sizeof(temp), "%d", mHwVersion);
            return SetHardwareVersionString(temp, temp_size);
        }
        else
        {
            return SetHardwareVersionString((const char*)value, size);
        }

    case Parameters::ID::kManufacturingDate:
        if((nullptr == value) || (0 == size))
        {
            return SetManufacturingDate("", 0);
        }
        else
        {
            return SetManufacturingDate((const char*)value, size);
        }

    case Parameters::ID::kUniqueId:
    {
        char hex[2*kUniqueIdLengthMax + 1];
        if((nullptr == value) || (0 == size))
        {
            uint8_t temp[kUniqueIdLengthMax];
            chip::Crypto::DRBG_get_bytes(temp, sizeof(temp));
            ReturnErrorOnFailure(chip::Encoding::BytesToLowercaseHexString(temp, sizeof(temp), hex, sizeof(hex))) ;
        }
        else
        {
            ReturnErrorCodeIf(size > kUniqueIdLengthMax, CHIP_ERROR_INVALID_ARGUMENT);
            ReturnErrorOnFailure(chip::Encoding::BytesToLowercaseHexString(value, size, hex, sizeof(hex))) ;
        }
        return SetUniqueId((uint8_t*)hex, std::min(strlen(hex), 2*kUniqueIdLengthMax)); // Do not store the ending zero
    }

    case Parameters::ID::kSpake2pSalt:
        // Since the verifier is calculated by the script, the salt must come from the script too
        VerifyOrReturnError((nullptr != value) && (size > 0), CHIP_ERROR_INVALID_ARGUMENT);
        return SetSpake2pSalt((const char*)value, size);
        break;

    case Parameters::ID::kSpake2pVerifier:
        // Verifier is mandatory, this call will return error if value is null
        return SetSpake2pVerifier((const char*)value, size);

    case Parameters::ID::kSetupPayload:
        if((nullptr == value) || (0 == size))
        {
            uint8_t setup_payload[kTotalPayloadDataSize] = { 0 };
            GenerateQR(setup_payload, mVendorId, mProductId, mCommissioningFlow, mRendezvousFlags, mDiscriminator, mPasscode);
            return SetSetupPayload(setup_payload, kTotalPayloadDataSize);
        }
        else
        {
            return SetSetupPayload(value, size);

        }

    case Parameters::ID::kFirmwareInfo:
        if((nullptr == value) || (0 == size))
        {
            uint8_t temp[1] = { 0 };
            return SetFirmwareInformation(ByteSpan(temp, 0));
        }
        else
        {
            return SetFirmwareInformation(ByteSpan(value, size));
        }

    case Parameters::ID::kCertification:
        return SetCertificationDeclaration(ByteSpan(value, size));

    case Parameters::ID::kPaiCert:
        return SetProductAttestationIntermediateCert(ByteSpan(value, size));

    case Parameters::ID::kDacCert:
        return SetDeviceAttestationCert(ByteSpan(value, size));

    case Parameters::ID::kDacKey:
        return SetDeviceAttestationKey(ByteSpan(value, size));

    case Parameters::ID::kCommonName:
        if(nullptr == value)
        {
            memset(mCommonName, 0x00, sizeof(mCommonName));
        }
        else
        {
            VerifyOrReturnError(size < sizeof(mCommonName), CHIP_ERROR_INVALID_ARGUMENT);
            memcpy(mCommonName, value, size);
            mCommonName[size] = 0;
        }
        return CHIP_NO_ERROR;
#if OTA_ENCRYPTION_ENABLE
    case Parameters::ID::kOtaTlvEncryptionKey:
        if(value != nullptr) {
            return SetOtaTlvEncryptionKey(ByteSpan(value, size));
        }
#endif
    }
    return CHIP_ERROR_UNKNOWN_RESOURCE_ID;
}

CHIP_ERROR Storage::Set(uint16_t id, const uint64_t *value)
{
    return CHIP_ERROR_UNKNOWN_RESOURCE_ID;
}

CHIP_ERROR Storage::Get(uint16_t id, uint64_t &value)
{
    return CHIP_ERROR_UNKNOWN_RESOURCE_ID;
}

CHIP_ERROR Storage::Get(uint16_t id, uint8_t *value, size_t max_size, size_t &size)
{
    switch(id)
    {
    case Parameters::ID::kVersion:
        ReturnErrorOnFailure(GetProvisionVersion((char*)value, max_size, size));
        break;

    case Parameters::ID::kCsrFile:
    {
        CharSpan cn(mCommonName, strlen(mCommonName) + 1);
        MutableCharSpan csr((char *)value, max_size);
        ReturnErrorOnFailure(GetDeviceAttestationCSR(mVendorId, mProductId, cn, csr));
        size = csr.size();
        break;
    }
    case Parameters::ID::kSerialNumber:
        ReturnErrorOnFailure(GetSerialNumber((char*)value, max_size));
        size = strlen((char*)value);
        break;
    case Parameters::ID::kVendorName:
        ReturnErrorOnFailure(GetVendorName((char*)value, max_size));
        size = strlen((char*)value);
        break;
    case Parameters::ID::kProductName:
        ReturnErrorOnFailure(GetProductName((char*)value, max_size));
        size = strlen((char*)value);
        break;
    case Parameters::ID::kProductLabel:
        ReturnErrorOnFailure(GetProductLabel((char*)value, max_size));
        size = strlen((char*)value);
        break;
    case Parameters::ID::kProductUrl:
        ReturnErrorOnFailure(GetProductURL((char*)value, max_size));
        size = strlen((char*)value);
        break;
    case Parameters::ID::kPartNumber:
        ReturnErrorOnFailure(GetPartNumber((char*)value, max_size));
        size = strlen((char*)value);
        break;
    case Parameters::ID::kHwVersionStr:
        ReturnErrorOnFailure(GetHardwareVersionString((char*)value, max_size));
        size = strlen((char*)value);
        break;
    case Parameters::ID::kManufacturingDate:
        ReturnErrorOnFailure(GetManufacturingDate(value, max_size, size));
        break;
    case Parameters::ID::kUniqueId:
    {
        char hex[2*kUniqueIdLengthMax + 1];
        MutableByteSpan span((uint8_t*)hex, sizeof(hex));
        ReturnErrorOnFailure(GetRotatingDeviceIdUniqueId(span));
        size = HexToBytes(hex, 2*kUniqueIdLengthMax, value, max_size);
        break;
    }
    case Parameters::ID::kSpake2pSalt:
        ReturnErrorOnFailure(GetSpake2pSalt((char*)value, max_size, size));
        break;
    case Parameters::ID::kSpake2pVerifier:
        ReturnErrorOnFailure(GetSpake2pVerifier((char*)value, max_size, size));
        break;
    case Parameters::ID::kSetupPayload:
        ReturnErrorOnFailure(GetSetupPayload(value, max_size, size));
        break;
    case Parameters::ID::kCertification:
    {
        MutableByteSpan span(value, max_size);
        ReturnErrorOnFailure(GetCertificationDeclaration(span));
        size = span.size();
        break;
    }
    case Parameters::ID::kPaiCert:
    {
        MutableByteSpan span(value, max_size);
        ReturnErrorOnFailure(GetProductAttestationIntermediateCert(span));
        size = span.size();
        break;
    }
    case Parameters::ID::kDacCert:
    {
        MutableByteSpan span(value, max_size);
        ReturnErrorOnFailure(GetDeviceAttestationCert(span));
        size = span.size();
        break;
    }
    default:
        return CHIP_ERROR_UNKNOWN_RESOURCE_ID;
    }

    return CHIP_NO_ERROR;
}


//
// DeviceInstanceInfoProvider
//

CHIP_ERROR Storage::GetManufacturingDate(uint16_t & year, uint8_t & month, uint8_t & day)
{
    constexpr uint8_t kDateStringLength = 10; // YYYY-MM-DD
    char date[kDateStringLength + 1] = { 0 };
    size_t date_len = 0;
    char * parse_end = nullptr;

    CHIP_ERROR err = GetManufacturingDate((uint8_t*)date, sizeof(date), date_len);
    SuccessOrExit(err);

    VerifyOrExit(date_len == kDateStringLength, err = CHIP_ERROR_INVALID_ARGUMENT);

    // Cast does not lose information, because we then check that we only parsed
    // 4 digits, so our number can't be bigger than 9999.
    year = static_cast<uint16_t>(strtoul(date, &parse_end, 10));
    VerifyOrExit(parse_end == date + 4, err = CHIP_ERROR_INVALID_ARGUMENT);

    // Cast does not lose information, because we then check that we only parsed
    // 2 digits, so our number can't be bigger than 99.
    month = static_cast<uint8_t>(strtoul(date + 5, &parse_end, 10));
    VerifyOrExit(parse_end == date + 7, err = CHIP_ERROR_INVALID_ARGUMENT);

    // Cast does not lose information, because we then check that we only parsed
    // 2 digits, so our number can't be bigger than 99.
    day = static_cast<uint8_t>(strtoul(date + 8, &parse_end, 10));
    VerifyOrExit(parse_end == date + 10, err = CHIP_ERROR_INVALID_ARGUMENT);

exit:
    if (err != CHIP_NO_ERROR && err != CHIP_DEVICE_ERROR_CONFIG_NOT_FOUND)
    {
        ChipLogError(DeviceLayer, "Invalid manufacturing date: %s", date);
    }
    return err;
}

CHIP_ERROR Storage::GetRotatingDeviceIdUniqueId(MutableByteSpan & value)
{
    size_t size = 0;

    CHIP_ERROR err = GetUniqueId(value.data(), value.size(), size);
#ifdef CHIP_DEVICE_CONFIG_ROTATING_DEVICE_ID_UNIQUE_ID
    if (CHIP_DEVICE_ERROR_CONFIG_NOT_FOUND == err)
    {
        constexpr uint8_t unique_id[] = CHIP_DEVICE_CONFIG_ROTATING_DEVICE_ID_UNIQUE_ID;
        ReturnErrorCodeIf(sizeof(unique_id) > value.size(), CHIP_ERROR_BUFFER_TOO_SMALL);
        memcpy(value.data(), unique_id, sizeof(unique_id));
        size = sizeof(unique_id);
        err = CHIP_NO_ERROR;
    }
#endif
    ReturnErrorOnFailure(err);
    value.reduce_size(size);
    return CHIP_NO_ERROR;
}


//
// CommissionableDataProvider
//

CHIP_ERROR Storage::GetSpake2pSalt(MutableByteSpan & value)
{
    // Base64
    char salt_b64[kSpake2pSaltB64LengthMax + 1] = { 0 };
    size_t size_b64                       = 0;

    CHIP_ERROR err = this->GetSpake2pSalt(salt_b64, sizeof(salt_b64), size_b64);
#if defined(CHIP_DEVICE_CONFIG_USE_TEST_SPAKE2P_SALT)
    if (CHIP_DEVICE_ERROR_CONFIG_NOT_FOUND == err)
    {
        size_b64 = strlen(CHIP_DEVICE_CONFIG_USE_TEST_SPAKE2P_SALT);
        ReturnErrorCodeIf(size_b64 > sizeof(salt_b64), CHIP_ERROR_BUFFER_TOO_SMALL);
        memcpy(salt_b64, CHIP_DEVICE_CONFIG_USE_TEST_SPAKE2P_SALT, size_b64);
        err = CHIP_NO_ERROR;
    }
#endif
    ReturnErrorOnFailure(err);

    // Decode
    uint8_t salt[chip::Crypto::kSpake2p_Max_PBKDF_Salt_Length] = { 0 };
    size_t size = chip::Base64Decode32(salt_b64, size_b64, salt);
    ReturnErrorCodeIf(size > value.size(), CHIP_ERROR_BUFFER_TOO_SMALL);

    // Copy
    memcpy(value.data(), salt, size);
    value.reduce_size(size);
    return CHIP_NO_ERROR;
}

CHIP_ERROR Storage::GetSpake2pVerifier(MutableByteSpan & out_value, size_t & out_size)
{
    ReturnErrorCodeIf(out_value.size() < chip::Crypto::kSpake2p_VerifierSerialized_Length, CHIP_ERROR_BUFFER_TOO_SMALL);

    // Base64
    char verifier_b64[kSpake2pVerifierB64LengthMax + 1] = { 0 };
    size_t size_b64                                     = 0;

    CHIP_ERROR err = this->GetSpake2pVerifier(verifier_b64, sizeof(verifier_b64), size_b64);
#if defined(CHIP_DEVICE_CONFIG_USE_TEST_SPAKE2P_VERIFIER)
    if (CHIP_DEVICE_ERROR_CONFIG_NOT_FOUND == err)
    {
        size_b64 = strlen(CHIP_DEVICE_CONFIG_USE_TEST_SPAKE2P_VERIFIER);
        ReturnErrorCodeIf(size_b64 > sizeof(verifier_b64), CHIP_ERROR_BUFFER_TOO_SMALL);
        memcpy(verifier_b64, CHIP_DEVICE_CONFIG_USE_TEST_SPAKE2P_VERIFIER, size_b64);
        err = CHIP_NO_ERROR;
    }
#endif
    ReturnErrorOnFailure(err);

    // Decode
    out_size = chip::Base64Decode32(verifier_b64, size_b64, out_value.data());
    out_value.reduce_size(out_size);
    return CHIP_NO_ERROR;
}

//
// Other
//

CHIP_ERROR Storage::GetSetupPayload(chip::MutableCharSpan &value)
{
    uint8_t payload[kSetupPayloadSizeMax];
    size_t size = 0;

    // Setup bits
    size_t prefix_len = strlen(kQRCodePrefix);
    VerifyOrReturnError(value.size() > prefix_len, CHIP_ERROR_BUFFER_TOO_SMALL);

    CHIP_ERROR err = this->GetSetupPayload(payload, sizeof(payload), size);
#if defined(CHIP_DEVICE_CONFIG_USE_TEST_SETUP_PIN_CODE) && CHIP_DEVICE_CONFIG_USE_TEST_SETUP_PIN_CODE
    if (CHIP_DEVICE_ERROR_CONFIG_NOT_FOUND == err)
    {
        static constexpr uint8_t kTestSetupPayloadBitset[] = { 0x88, 0xFF, 0x2F, 0x00, 0x44, 0x00, 0xE0, 0x4B, 0x84, 0x68, 0x02 };
        size = sizeof(kTestSetupPayloadBitset);
        ReturnErrorCodeIf(size > kTotalPayloadDataSizeInBytes, CHIP_ERROR_BUFFER_TOO_SMALL);
        memcpy(payload, kTestSetupPayloadBitset, size);
        err = CHIP_NO_ERROR;
    }
#endif
    ReturnErrorOnFailure(err);
    VerifyOrReturnError(size > 0, CHIP_ERROR_NOT_FOUND);
    char *data = value.data();
    // Prefix
    memcpy(data, kQRCodePrefix, prefix_len);
    // Base38
    MutableCharSpan qr_code(data + prefix_len, value.size() - prefix_len);
    ReturnErrorOnFailure(base38Encode(ByteSpan(payload, size), qr_code));
    value.reduce_size(prefix_len + qr_code.size());
    return CHIP_NO_ERROR;
}

} // namespace Provision
} // namespace Silabs
} // namespace DeviceLayer
} // namespace chip
