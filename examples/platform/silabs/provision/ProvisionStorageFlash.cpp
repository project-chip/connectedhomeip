#include "ProvisionStorageFlash.h"
#include "AttestationKeyMbed.h"
#include <lib/core/CHIPEncoding.h>
#include <lib/support/CodeUtils.h>

#include <em_msc.h>
#include <nvm3.h>
#include <nvm3_default.h>
#include <nvm3_hal_flash.h>
#include "silabs_creds.h"

extern uint8_t linker_nvm_end[];
static uint8_t * _base_address = (uint8_t *) linker_nvm_end;

namespace chip {
namespace DeviceLayer {
namespace Silabs {
namespace Provision {


enum FlashIds
{
    kFieldMap = 0,
    kSerialNumber,
    kVendorId,
    kVendorName,
    kProductId,
    kProductName,
    kProductLabel,
    kProductURL,
    kPartNumber,
    kHardwareVersion,
    kHardwareVersionStr,
    kManufacturingDate,
    kUniqueId,
    kDiscriminator,
    kSpake2pIterations,
    kSpake2pSalt,
    kSpake2pVerifier,
    kFirmwareInfo,
    // kBaseAddress,
    kCertificationDeclaration,
    kProductAttestationIntermediateCert,
    kDeviceAttestationCert,
    kDeviceAttestationKey,
    kSetupPayload, // b23: 0x800000
    kProvisionRequest,
    kFieldCount,
};
static_assert(kFieldCount < 32);

struct FlashEntry
{
    FlashIds id;
    uint32_t size;
    uint32_t offset;
};

class Flash
{
public:

    Flash()
    {
        Add(kFieldMap, sizeof(uint32_t));
        Add(kSerialNumber, kSerialNumberLengthMax, true);
        Add(kVendorId, sizeof(uint16_t));
        Add(kVendorName, kVendorNameLengthMax, true);
        Add(kProductId, sizeof(uint16_t));
        Add(kProductName, kProductNameLengthMax, true);
        Add(kProductLabel, kProductLabelLengthMax, true);
        Add(kProductURL, kProductUrlLengthMax, true);
        Add(kPartNumber, kPartNumberLengthMax, true);
        Add(kHardwareVersion, sizeof(uint16_t));
        Add(kHardwareVersionStr, kHardwareVersionStrLengthMax, true);
        Add(kManufacturingDate, kManufacturingDateLengthMax, true);
        Add(kUniqueId, kUniqueIdLengthMax, true);
        Add(kDiscriminator, sizeof(uint16_t));
        Add(kSpake2pIterations, sizeof(uint32_t));
        Add(kSpake2pSalt, kSpake2pSaltB64LengthMax, true);
        Add(kSpake2pVerifier, kSpake2pVerifierB64LengthMax, true);
        Add(kFirmwareInfo, kFirmwareInfoSizeMax, true);
        // Add(kBaseAddress, sizeof(uint32_t));
        Add(kCertificationDeclaration, kCertificationSizeMax, true);
        Add(kProductAttestationIntermediateCert, kCertificateSizeMax, true);
        Add(kDeviceAttestationCert, kCertificateSizeMax, true);
        Add(kDeviceAttestationKey, kDeviceAttestationKeySizeMax, true);
        Add(kSetupPayload, kSetupPayloadSizeMax, true);
        Add(kProvisionRequest, sizeof(uint8_t));
    }

    void Add(FlashIds id, uint32_t size, bool variable = false)
    {
        if(id < kFieldCount)
        {
            FlashEntry & e = _entries[id];
            e.id = id;
            e.size = size;
            e.offset = this->_offset;
            this->_offset += e.size;
            if(variable) {
                this->_offset += 2;
            }
        }
    }

    CHIP_ERROR Flag(FlashIds id, bool active)
    {
        VerifyOrReturnError(id < 32, CHIP_ERROR_NOT_FOUND);
        VerifyOrReturnError(id < kFieldCount, CHIP_ERROR_NOT_FOUND);
        VerifyOrReturnError(kFieldMap != id, CHIP_NO_ERROR); // Avoid recursion
        uint32_t map = 0;
        uint32_t mask = static_cast<uint32_t>(1 << id);
        Get(kFieldMap, map);
        if(active) {
            return Set(kFieldMap, map | mask);
        }
        else {
            return Set(kFieldMap, map & ~mask);
        }
    }

    bool IsSet(FlashIds id)
    {
        VerifyOrReturnError(id < 32, false);
        VerifyOrReturnError(id < kFieldCount, false);
        VerifyOrReturnError(kFieldMap != id, true); // Avoid recursion
        uint32_t map = 0;
        uint32_t mask = static_cast<uint32_t>(1 << id);
        Get(kFieldMap, map);
        return map & mask;
    }

    CHIP_ERROR Set(FlashIds id, const uint8_t *value, size_t size)
    {

        VerifyOrReturnError(id < kFieldCount, CHIP_ERROR_NOT_FOUND);
        FlashEntry & e = _entries[id];
        VerifyOrReturnError(e.size >= size, CHIP_ERROR_INTERNAL);
        uint8_t *p = _base_address + e.offset;
        chip::Encoding::BigEndian::Put16(p, (uint16_t) size);
        p += sizeof(uint16_t);
        memcpy(p, value, size);
        return Flag(id, true);
        // if(kSetupPayload == id) {
        //     uint32_t map = 0;
        //     Get(kFieldMap, map);
        //     return CHIP_ERROR(0x0a000000 + map);
        // }
        // return CHIP_NO_ERROR;
    }


    CHIP_ERROR Set(FlashIds id, const char *value, size_t size)
    {
        return Set(id, (const uint8_t *) value, size);
    }

    CHIP_ERROR Get(FlashIds id, uint8_t *value, size_t max, size_t & size)
    {
        VerifyOrReturnError(IsSet(id), CHIP_ERROR_NOT_FOUND);
        FlashEntry & e = _entries[id];
        VerifyOrReturnError(max >= e.size, CHIP_ERROR_INTERNAL);
        uint8_t *p = _base_address + e.offset;
        size = chip::Encoding::BigEndian::Get16(p);
        VerifyOrReturnError(e.size >= size, CHIP_ERROR_INTERNAL);
        p += sizeof(uint16_t);
        memcpy(value, p, size);
        return CHIP_NO_ERROR;
    }

    CHIP_ERROR Get(FlashIds id, char *value, size_t max, size_t & size)
    {
        VerifyOrReturnError(IsSet(id), CHIP_ERROR_NOT_FOUND);
        VerifyOrReturnError(max > 0, CHIP_ERROR_INTERNAL);
        ReturnErrorOnFailure(Get(id, (uint8_t *) value, max - 1, size));
        value[size] = 0;
        return CHIP_NO_ERROR;
    }

    CHIP_ERROR Set(FlashIds id, bool value)
    {
        VerifyOrReturnError(id < kFieldCount, CHIP_ERROR_NOT_FOUND);
        FlashEntry & e = _entries[id];
        VerifyOrReturnError(e.size > 0, CHIP_ERROR_INTERNAL);
        _page[e.offset] = (value ? 1 : 0);
        return Flag(id, true);
    }

    CHIP_ERROR Get(FlashIds id, bool & value)
    {
        VerifyOrReturnError(IsSet(id), CHIP_ERROR_NOT_FOUND);
        FlashEntry & e = _entries[id];
        VerifyOrReturnError(e.size > 0, CHIP_ERROR_INTERNAL);
        uint8_t *p = _base_address + e.offset;
        value = (*p > 0);
        return Flag(id, true);
    }

    CHIP_ERROR Set(FlashIds id, uint16_t value)
    {
        VerifyOrReturnError(id < kFieldCount, CHIP_ERROR_NOT_FOUND);
        FlashEntry & e = _entries[id];
        VerifyOrReturnError(e.size >= sizeof(uint16_t), CHIP_ERROR_INTERNAL);
        chip::Encoding::BigEndian::Put16(&_page[e.offset], value);
        return Flag(id, true);
    }

    CHIP_ERROR Get(FlashIds id, uint16_t & value)
    {
        VerifyOrReturnError(IsSet(id), CHIP_ERROR_NOT_FOUND);
        FlashEntry & e = _entries[id];
        VerifyOrReturnError(e.size >= sizeof(uint16_t), CHIP_ERROR_INTERNAL);
        uint8_t *p = _base_address + e.offset;
        value = chip::Encoding::BigEndian::Get16(p);
        return Flag(id, true);
    }

    CHIP_ERROR Set(FlashIds id, uint32_t value)
    {
        VerifyOrReturnError(id < kFieldCount, CHIP_ERROR_NOT_FOUND);
        FlashEntry & e = _entries[id];
        VerifyOrReturnError(e.size >= sizeof(uint32_t), CHIP_ERROR_INTERNAL);
        chip::Encoding::BigEndian::Put32(&_page[e.offset], value);
        return Flag(id, true);
    }

    CHIP_ERROR Get(FlashIds id, uint32_t & value)
    {
        VerifyOrReturnError(IsSet(id), CHIP_ERROR_NOT_FOUND);
        FlashEntry & e = _entries[id];
        VerifyOrReturnError(e.size >= sizeof(uint32_t), CHIP_ERROR_INTERNAL);
        uint8_t *p = _base_address + e.offset;
        value = chip::Encoding::BigEndian::Get32(p);
        return Flag(id, true);
    }

    CHIP_ERROR Commit()
    {
        // Get page address
        uint32_t base_addr = (uint32_t)_base_address;
        // ReturnErrorOnFailure(Get(FlashIds::kBaseAddress, base_addr));
#ifdef SIWX_917
        return CHIP_ERROR_NOT_IMPLEMENTED;
#else
        // Erase page
        MSC_ErasePage((uint32_t *)base_addr);
        // Write to flash
        MSC_WriteWord((uint32_t *)base_addr, _page, FLASH_PAGE_SIZE);
        return CHIP_NO_ERROR;
#endif
    }
    FlashEntry _entries[kFieldCount];
    uint8_t _page[FLASH_PAGE_SIZE] = { 0 };
    uint32_t _offset = 0;
};


Flash _flash;

CHIP_ERROR FlashStorage::Initialize(uint32_t flash_addr, uint32_t flash_size)
{
    // uint32_t base_addr = flash_addr + flash_size - FLASH_PAGE_SIZE;
    // return _flash.Set(FlashIds::kBaseAddress, base_addr);
    return CHIP_NO_ERROR;
}

CHIP_ERROR FlashStorage::GetBaseAddress(uint32_t & value)
{
    // return _flash.Set(FlashIds::kBaseAddress, value);
    value = 0;
    return CHIP_NO_ERROR;
}


//
// DeviceInstanceInfoProvider
//

CHIP_ERROR FlashStorage::SetSerialNumber(const char * value, size_t len)
{
    return _flash.Set(FlashIds::kSerialNumber, value, len);
}

CHIP_ERROR FlashStorage::GetSerialNumber(char * value, size_t max)
{
    size_t size = 0;
    return _flash.Get(FlashIds::kSerialNumber, value, max, size);
}

CHIP_ERROR FlashStorage::SetVendorId(uint16_t value)
{
    return _flash.Set(FlashIds::kVendorId, value);
}

CHIP_ERROR FlashStorage::GetVendorId(uint16_t & value)
{
    return _flash.Get(FlashIds::kVendorId, value);
}

CHIP_ERROR FlashStorage::SetVendorName(const char * value, size_t len)
{
    return _flash.Set(FlashIds::kVendorName, value, len);
}

CHIP_ERROR FlashStorage::GetVendorName(char * value, size_t max)
{
    size_t size = 0;
    return _flash.Get(FlashIds::kVendorName, value, max, size);
}

CHIP_ERROR FlashStorage::SetProductId(uint16_t value)
{
    return _flash.Set(FlashIds::kProductId, value);
}

CHIP_ERROR FlashStorage::GetProductId(uint16_t & value)
{
    return _flash.Get(FlashIds::kProductId, value);
}

CHIP_ERROR FlashStorage::SetProductName(const char * value, size_t len)
{
    return _flash.Set(FlashIds::kProductName, value, len);
}

CHIP_ERROR FlashStorage::GetProductName(char * value, size_t max)
{
    size_t size = 0;
    return _flash.Get(FlashIds::kProductName, value, max, size);
}

CHIP_ERROR FlashStorage::SetProductLabel(const char * value, size_t len)
{
    return _flash.Set(FlashIds::kProductLabel, value, len);
}

CHIP_ERROR FlashStorage::GetProductLabel(char * value, size_t max)
{
    size_t size = 0;
    return _flash.Get(FlashIds::kProductLabel, value, max, size);
}

CHIP_ERROR FlashStorage::SetProductURL(const char * value, size_t len)
{
    return _flash.Set(FlashIds::kProductURL, value, len);
}
CHIP_ERROR FlashStorage::GetProductURL(char * value, size_t max)
{
    size_t size = 0;
    return _flash.Get(FlashIds::kProductURL, value, max, size);
}

CHIP_ERROR FlashStorage::SetPartNumber(const char * value, size_t len)
{
    return _flash.Set(FlashIds::kPartNumber, value, len);
}

CHIP_ERROR FlashStorage::GetPartNumber(char * value, size_t max)
{
    size_t size = 0;
    return _flash.Get(FlashIds::kPartNumber, value, max, size);
}

CHIP_ERROR FlashStorage::SetHardwareVersion(uint16_t & value)
{
    return _flash.Set(FlashIds::kHardwareVersion, value);
}

CHIP_ERROR FlashStorage::GetHardwareVersion(uint16_t & value)
{
    return _flash.Get(FlashIds::kHardwareVersion, value);
}

CHIP_ERROR FlashStorage::SetHardwareVersionString(const char * value, size_t len)
{
    return _flash.Set(FlashIds::kHardwareVersionStr, value, len);
}

CHIP_ERROR FlashStorage::GetHardwareVersionString(char * value, size_t max)
{
    size_t size = 0;
    return _flash.Get(FlashIds::kHardwareVersionStr, value, max, size);
}


CHIP_ERROR FlashStorage::SetManufacturingDate(const char * value, size_t len)
{
    return _flash.Set(FlashIds::kManufacturingDate, value, len);
}

CHIP_ERROR FlashStorage::GetManufacturingDate(char * value, size_t max)
{
    size_t size = 0;
    return _flash.Get(FlashIds::kManufacturingDate, value, max, size);
}

CHIP_ERROR FlashStorage::SetUniqueId(const uint8_t * value, size_t size)
{
    return _flash.Set(FlashIds::kUniqueId, value, size);
}

CHIP_ERROR FlashStorage::GetRotatingDeviceIdUniqueId(MutableByteSpan & value)
{
    size_t size = 0;
    ReturnErrorOnFailure(_flash.Get(FlashIds::kUniqueId, value.data(), value.size(), size));
    value.reduce_size(size);
    return CHIP_NO_ERROR;
}


//
// CommissionableDataProvider
//

CHIP_ERROR FlashStorage::SetSetupDiscriminator(uint16_t value)
{
    return _flash.Set(FlashIds::kDiscriminator, value);
}

CHIP_ERROR FlashStorage::GetSetupDiscriminator(uint16_t & value)
{
    return _flash.Get(FlashIds::kDiscriminator, value);
}

CHIP_ERROR FlashStorage::SetSpake2pIterationCount(uint32_t value)
{
    return _flash.Set(FlashIds::kSpake2pIterations, value);
}

CHIP_ERROR FlashStorage::GetSpake2pIterationCount(uint32_t & value)
{
    return _flash.Get(FlashIds::kSpake2pIterations, value);
}

CHIP_ERROR FlashStorage::SetSetupPasscode(uint32_t value)
{
    return CHIP_ERROR_NOT_IMPLEMENTED;
}

CHIP_ERROR FlashStorage::GetSetupPasscode(uint32_t & value)
{
    return CHIP_ERROR_NOT_IMPLEMENTED;
}

CHIP_ERROR FlashStorage::SetSpake2pSalt(const char * value, size_t size)
{
    return _flash.Set(FlashIds::kSpake2pSalt, value, size);
}

CHIP_ERROR FlashStorage::GetSpake2pSalt(char * value, size_t max, size_t &size)
{
    return _flash.Get(FlashIds::kSpake2pSalt, value, max, size);
}

CHIP_ERROR FlashStorage::SetSpake2pVerifier(const char * value, size_t size)
{
    return _flash.Set(FlashIds::kSpake2pVerifier, value, size);
}

CHIP_ERROR FlashStorage::GetSpake2pVerifier(char * value, size_t max, size_t &size)
{
    return _flash.Get(FlashIds::kSpake2pVerifier, value, max, size);
}

//
// DeviceAttestationCredentialsProvider
//

CHIP_ERROR FlashStorage::SetFirmwareInformation(const ByteSpan & value)
{
    return CHIP_ERROR_NOT_IMPLEMENTED;
}

CHIP_ERROR FlashStorage::GetFirmwareInformation(MutableByteSpan & value)
{
    // TODO: We need a real example FirmwareInformation to be populated.
    value.reduce_size(0);
    return CHIP_NO_ERROR;
}

CHIP_ERROR FlashStorage::SetCertificationDeclaration(const ByteSpan & value)
{
    return _flash.Set(FlashIds::kCertificationDeclaration, value.data(), value.size());
}

CHIP_ERROR FlashStorage::GetCertificationDeclaration(MutableByteSpan & value)
{
    size_t size = 0;
    ReturnErrorOnFailure(_flash.Get(FlashIds::kCertificationDeclaration, value.data(), value.size(), size));
    value.reduce_size(size);
    return CHIP_NO_ERROR;
}

CHIP_ERROR FlashStorage::SetProductAttestationIntermediateCert(const ByteSpan & value)
{
    return _flash.Set(FlashIds::kProductAttestationIntermediateCert, value.data(), value.size());
}

CHIP_ERROR FlashStorage::GetProductAttestationIntermediateCert(MutableByteSpan & value)
{
    size_t size = 0;
    ReturnErrorOnFailure(_flash.Get(FlashIds::kProductAttestationIntermediateCert, value.data(), value.size(), size));
    value.reduce_size(size);
    return CHIP_NO_ERROR;
}

CHIP_ERROR FlashStorage::SetDeviceAttestationCert(const ByteSpan & value)
{
    return _flash.Set(FlashIds::kDeviceAttestationCert, value.data(), value.size());
}

CHIP_ERROR FlashStorage::GetDeviceAttestationCert(MutableByteSpan & value)
{
    size_t size = 0;
    ReturnErrorOnFailure(_flash.Get(FlashIds::kDeviceAttestationCert, value.data(), value.size(), size));
    value.reduce_size(size);
    return CHIP_NO_ERROR;
}

CHIP_ERROR FlashStorage::SetDeviceAttestationKey(uint32_t kid, const ByteSpan & value)
{
    return _flash.Set(FlashIds::kDeviceAttestationKey, value.data(), value.size());
}

CHIP_ERROR FlashStorage::GetDeviceAttestationCSR(uint32_t kid, uint16_t vid, uint16_t pid, const CharSpan &cn, MutableCharSpan & csr)
{
    AttestationKeyMbed key;
    uint8_t temp[128] = { 0 };
    size_t size = 0;
    ReturnErrorOnFailure(key.GenerateCSR(vid, pid, cn, csr));
    ReturnErrorOnFailure(key.Export(temp, sizeof(temp), size));
    return _flash.Set(FlashIds::kDeviceAttestationKey, temp, size);
}

CHIP_ERROR FlashStorage::SignWithDeviceAttestationKey(const ByteSpan & message, MutableByteSpan & signature)
{
    AttestationKeyMbed key;
    uint8_t temp[kDeviceAttestationKeySizeMax] = { 0 };
    size_t size = 0;
    ReturnErrorOnFailure(_flash.Get(FlashIds::kDeviceAttestationKey, temp, sizeof(temp), size));
    key.Import(temp, size);
    return key.SignMessage(message, signature);
}

//
// Other
//

CHIP_ERROR FlashStorage::SetSetupPayload(const uint8_t * value, size_t size)
{
    return _flash.Set(FlashIds::kSetupPayload, value, size);
}

CHIP_ERROR FlashStorage::GetSetupPayload(uint8_t * value, size_t max, size_t &size)
{
    return _flash.Get(FlashIds::kSetupPayload, value, max, size);
}

CHIP_ERROR FlashStorage::SetProvisionRequest(bool value)
{
    return _flash.Set(FlashIds::kProvisionRequest, value);
}

CHIP_ERROR FlashStorage::GetProvisionRequest(bool &value)
{
    return _flash.Set(FlashIds::kProvisionRequest, value);
}

} // namespace Provision

void MigrateDacProvider(void) {}

} // namespace Silabs
} // namespace DeviceLayer
} // namespace chip
