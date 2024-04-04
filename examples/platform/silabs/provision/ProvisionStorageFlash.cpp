#include "ProvisionStorageFlash.h"
#include "ProvisionEncoder.h"
#include "AttestationKey.h"
#include <lib/support/CodeUtils.h>
#include <lib/core/CHIPEncoding.h>
#include <lib/support/CodeUtils.h>
#include <string.h>
#include <algorithm>

#include <psa/crypto.h>
#include <em_msc.h>
#include <nvm3.h>
#include <nvm3_default.h>
#include <nvm3_hal_flash.h>
#include "silabs_creds.h"

namespace {
extern uint8_t linker_nvm_end[];
constexpr size_t kMaxBinaryValue = 1024;
constexpr size_t kArgumentBufferSize = 2 * sizeof(uint16_t) + kMaxBinaryValue; // ID(2) + Size(2) + Value(n)
} // namespace

namespace chip {
namespace DeviceLayer {
namespace Silabs {
namespace Provision {
namespace Flash {

uint8_t *sReadOnlyPage = (uint8_t *) linker_nvm_end;
uint8_t sTemporaryPage[FLASH_PAGE_SIZE] = { 0 };
uint8_t *sActivePage = sReadOnlyPage;
uint16_t sMinOffset = FLASH_PAGE_SIZE;
uint16_t sMaxOffset = 0;


CHIP_ERROR DecodeTotal(Encoding::Buffer &reader, uint16_t &total)
{
    uint16_t sz = 0;
    ReturnErrorOnFailure(reader.Get(sz));
    total = (0xffff == sz) ? sizeof(uint16_t) : sz;
    reader.in = reader.begin + total;
    ReturnErrorCodeIf(reader.in > reader.end, CHIP_ERROR_INTERNAL);
    return CHIP_NO_ERROR;
}

CHIP_ERROR ActivateWrite(uint8_t *&active)
{
    if(sActivePage == sReadOnlyPage)
    {
        memcpy(sTemporaryPage, sReadOnlyPage, sizeof(sTemporaryPage));
    }
    active = sActivePage = sTemporaryPage;
    return CHIP_NO_ERROR;
}

CHIP_ERROR Set(uint16_t id, Encoding::Buffer &in)
{
    uint8_t *page = sActivePage;
    uint16_t total = 0;
    Encoding::Buffer reader(page, FLASH_PAGE_SIZE, true);
    Encoding::Version2::Argument found;

    // Decode total
    ReturnErrorOnFailure(DecodeTotal(reader, total));
    // Search entry
    CHIP_ERROR err = Encoding::Version2::Find(reader, id, found);
    if((CHIP_ERROR_NOT_FOUND != err) && (CHIP_NO_ERROR != err))
    {
        // Memory corruption, write at the last correct address
        return err;
        err = CHIP_ERROR_NOT_FOUND;
    }
    ReturnErrorOnFailure(ActivateWrite(page));

    Encoding::Buffer writer(page, FLASH_PAGE_SIZE);
    if(CHIP_ERROR_NOT_FOUND == err)
    {
        // New entry
        size_t temp_total = found.offset;
        ReturnErrorCodeIf(temp_total + in.Size() > FLASH_PAGE_SIZE, CHIP_ERROR_INVALID_ARGUMENT);
        // Copy entry
        ReturnErrorOnFailure(in.Get(page + temp_total, in.Size()));
        // Update total
        total = temp_total + in.Size();
        ReturnErrorOnFailure(writer.Add(total));
    }
    else
    {
        // Existing entry
        if(in.Size() == found.encoded_size)
        {
            // Same size, keep in place
            memset(page + found.offset, 0xff, found.encoded_size);
            ReturnErrorOnFailure(in.Get(page + found.offset, in.Size()));
        }
        else
        {
            // Size change, move to the end
            uint16_t temp_total = total - found.encoded_size;
            ReturnErrorCodeIf(temp_total + in.Size() > FLASH_PAGE_SIZE, CHIP_ERROR_INVALID_ARGUMENT);
            // Remove the entry
            memmove(page + found.offset, page + found.offset + found.encoded_size, temp_total);
            // Add the entry
            ReturnErrorOnFailure(in.Get(page + temp_total, in.Size()));
            // Update total
            total = temp_total + in.Size();
            ReturnErrorOnFailure(writer.Add(total));
        }
    }
    return CHIP_NO_ERROR;
}

CHIP_ERROR Get(uint16_t id, Encoding::Version2::Argument &arg)
{
    uint16_t total = 0;

    Encoding::Buffer reader(sActivePage, FLASH_PAGE_SIZE, true);
    ReturnErrorOnFailure(DecodeTotal(reader, total));
    return Encoding::Version2::Find(reader, id, arg);
}


CHIP_ERROR Set(uint16_t id, uint8_t value)
{
    Encoding::Version2::Argument arg;
    ReturnErrorOnFailure(Encoding::Version2::Encode(id, &value, arg));
    return Set(id, arg);
}

CHIP_ERROR Get(uint16_t id, uint8_t &value)
{
    Encoding::Version2::Argument arg;
    ReturnErrorOnFailure(Get(id, arg));
    VerifyOrReturnError(Encoding::Version2::Type_Int8u == arg.type, CHIP_ERROR_INVALID_ARGUMENT);
    value = arg.value.u8;
    return CHIP_NO_ERROR;
}

CHIP_ERROR Set(uint16_t id, uint16_t value)
{
    Encoding::Version2::Argument arg;
    ReturnErrorOnFailure(Encoding::Version2::Encode(id, &value, arg));
    return Set(id, arg);
}

CHIP_ERROR Get(uint16_t id, uint16_t &value)
{
    Encoding::Version2::Argument arg;
    ReturnErrorOnFailure(Get(id, arg));
    VerifyOrReturnError(Encoding::Version2::Type_Int16u == arg.type, CHIP_ERROR_INVALID_ARGUMENT);
    value = arg.value.u16;
    return CHIP_NO_ERROR;
}

CHIP_ERROR Set(uint16_t id, uint32_t value)
{
    Encoding::Version2::Argument arg;
    ReturnErrorOnFailure(Encoding::Version2::Encode(id, &value, arg));
    return Set(id, arg);
}

CHIP_ERROR Get(uint16_t id, uint32_t &value)
{
    Encoding::Version2::Argument arg;
    ReturnErrorOnFailure(Get(id, arg));
    VerifyOrReturnError(Encoding::Version2::Type_Int32u == arg.type, CHIP_ERROR_INVALID_ARGUMENT);
    value = arg.value.u32;
    return CHIP_NO_ERROR;
}

CHIP_ERROR Set(uint16_t id, const uint8_t *value, size_t size)
{
    Encoding::Version2::Argument arg;
    ReturnErrorOnFailure(Encoding::Version2::Encode(id, value, size, arg));
    return Set(id, arg);
}

CHIP_ERROR Get(uint16_t id, uint8_t *value, size_t max_size, size_t & size)
{

    Encoding::Version2::Argument arg;
    ReturnErrorOnFailure(Get(id, arg));
    VerifyOrReturnError(Encoding::Version2::Type_Binary == arg.type, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(arg.size <= max_size, CHIP_ERROR_INTERNAL);
    memcpy(value, arg.value.b, arg.size);
    size = arg.size;
    return CHIP_NO_ERROR;
}

CHIP_ERROR Set(uint16_t id, const char *value, size_t size)
{
    return Set(id, (const uint8_t*)value, size);
}

CHIP_ERROR Get(uint16_t id, char *value, size_t max_size, size_t & size)
{
    return Get(id, (uint8_t*)value, max_size - 1, size);
}

} // namespace Flash



//
// Initialization
//

CHIP_ERROR Storage::Initialize(uint32_t flash_addr, uint32_t flash_size)
{
    if(flash_size > 0)
    {
        Flash::sReadOnlyPage = (uint8_t*)(flash_addr + flash_size - FLASH_PAGE_SIZE);
    }
    Flash::sActivePage = Flash::sReadOnlyPage;
    MSC_Init();
    return CHIP_NO_ERROR;
}

CHIP_ERROR Storage::Commit()
{
    if(Flash::sActivePage == Flash::sTemporaryPage)
    {
        // Erase page
        MSC_ErasePage((uint32_t *)Flash::sReadOnlyPage);
        // Write to flash
        MSC_WriteWord((uint32_t *)Flash::sReadOnlyPage, Flash::sTemporaryPage, FLASH_PAGE_SIZE);
    }
    return CHIP_NO_ERROR;
}

CHIP_ERROR Storage::GetBaseAddress(uint32_t & value)
{
    value = (uint32_t)Flash::sReadOnlyPage;
    return CHIP_NO_ERROR;
}


//
// DeviceInstanceInfoProvider
//

CHIP_ERROR Storage::SetSerialNumber(const char * value, size_t len)
{
    return Flash::Set(Parameters::ID::kSerialNumber, value, len);
}

CHIP_ERROR Storage::GetSerialNumber(char * value, size_t max)
{
    size_t size = 0;
    return Flash::Get(Parameters::ID::kSerialNumber, value, max, size);
}

CHIP_ERROR Storage::SetVendorId(uint16_t value)
{
    return Flash::Set(Parameters::ID::kVendorId, value);
}

CHIP_ERROR Storage::GetVendorId(uint16_t & value)
{
    return Flash::Get(Parameters::ID::kVendorId, value);
}

CHIP_ERROR Storage::SetVendorName(const char * value, size_t len)
{
    return Flash::Set(Parameters::ID::kVendorName, value, len);
}

CHIP_ERROR Storage::GetVendorName(char * value, size_t max)
{
    size_t size = 0;
    return Flash::Get(Parameters::ID::kVendorName, value, max, size);
}

CHIP_ERROR Storage::SetProductId(uint16_t value)
{
    return Flash::Set(Parameters::ID::kProductId, value);
}

CHIP_ERROR Storage::GetProductId(uint16_t & value)
{
    return Flash::Get(Parameters::ID::kProductId, value);
}

CHIP_ERROR Storage::SetProductName(const char * value, size_t len)
{
    return Flash::Set(Parameters::ID::kProductName, value, len);
}

CHIP_ERROR Storage::GetProductName(char * value, size_t max)
{
    size_t size = 0;
    return Flash::Get(Parameters::ID::kProductName, value, max, size);
}

CHIP_ERROR Storage::SetProductLabel(const char * value, size_t len)
{
    return Flash::Set(Parameters::ID::kProductLabel, value, len);
}

CHIP_ERROR Storage::GetProductLabel(char * value, size_t max)
{
    size_t size = 0;
    return Flash::Get(Parameters::ID::kProductLabel, value, max, size);
}

CHIP_ERROR Storage::SetProductURL(const char * value, size_t len)
{
    return Flash::Set(Parameters::ID::kProductUrl, value, len);
}
CHIP_ERROR Storage::GetProductURL(char * value, size_t max)
{
    size_t size = 0;
    return Flash::Get(Parameters::ID::kProductUrl, value, max, size);
}

CHIP_ERROR Storage::SetPartNumber(const char * value, size_t len)
{
    return Flash::Set(Parameters::ID::kPartNumber, value, len);
}

CHIP_ERROR Storage::GetPartNumber(char * value, size_t max)
{
    size_t size = 0;
    return Flash::Get(Parameters::ID::kPartNumber, value, max, size);
}

CHIP_ERROR Storage::SetHardwareVersion(uint16_t value)
{
    return Flash::Set(Parameters::ID::kHwVersion, value);
}

CHIP_ERROR Storage::GetHardwareVersion(uint16_t & value)
{
    return Flash::Get(Parameters::ID::kHwVersion, value);
}

CHIP_ERROR Storage::SetHardwareVersionString(const char * value, size_t len)
{
    return Flash::Set(Parameters::ID::kHwVersionStr, value, len);
}

CHIP_ERROR Storage::GetHardwareVersionString(char * value, size_t max)
{
    size_t size = 0;
    return Flash::Get(Parameters::ID::kHwVersionStr, value, max, size);
}


CHIP_ERROR Storage::SetManufacturingDate(const char * value, size_t len)
{
    return Flash::Set(Parameters::ID::kManufacturingDate, value, len);
}

CHIP_ERROR Storage::GetManufacturingDate(uint8_t * value, size_t max, size_t &size)
{
    return Flash::Get(Parameters::ID::kManufacturingDate, value, max, size);
}

CHIP_ERROR Storage::SetUniqueId(const uint8_t * value, size_t size)
{
    return Flash::Set(Parameters::ID::kUniqueId, value, size);
}

CHIP_ERROR Storage::GetUniqueId(uint8_t * value, size_t max, size_t &size)
{
    return Flash::Get(Parameters::ID::kUniqueId, value, max, size);
}

//
// CommissionableDataProvider
//

CHIP_ERROR Storage::SetSetupDiscriminator(uint16_t value)
{
    return Flash::Set(Parameters::ID::kDiscriminator, value);
}

CHIP_ERROR Storage::GetSetupDiscriminator(uint16_t & value)
{
    return Flash::Get(Parameters::ID::kDiscriminator, value);
}

CHIP_ERROR Storage::SetSpake2pIterationCount(uint32_t value)
{
    return Flash::Set(Parameters::ID::kSpake2pIterations, value);
}

CHIP_ERROR Storage::GetSpake2pIterationCount(uint32_t & value)
{
    return Flash::Get(Parameters::ID::kSpake2pIterations, value);
}

CHIP_ERROR Storage::SetSetupPasscode(uint32_t value)
{
    return CHIP_ERROR_NOT_IMPLEMENTED;
}

CHIP_ERROR Storage::GetSetupPasscode(uint32_t & value)
{
    return CHIP_ERROR_NOT_IMPLEMENTED;
}

CHIP_ERROR Storage::SetSpake2pSalt(const char * value, size_t size)
{
    return Flash::Set(Parameters::ID::kSpake2pSalt, value, size);
}

CHIP_ERROR Storage::GetSpake2pSalt(char * value, size_t max, size_t &size)
{
    return Flash::Get(Parameters::ID::kSpake2pSalt, value, max, size);
}

CHIP_ERROR Storage::SetSpake2pVerifier(const char * value, size_t size)
{
    return Flash::Set(Parameters::ID::kSpake2pVerifier, value, size);
}

CHIP_ERROR Storage::GetSpake2pVerifier(char * value, size_t max, size_t &size)
{
    return Flash::Get(Parameters::ID::kSpake2pVerifier, value, max, size);
}

//
// DeviceAttestationCredentialsProvider
//

CHIP_ERROR Storage::SetFirmwareInformation(const ByteSpan & value)
{
    (void)value;
    return CHIP_NO_ERROR;
}

CHIP_ERROR Storage::GetFirmwareInformation(MutableByteSpan & value)
{
    // TODO: We need a real example FirmwareInformation to be populated.
    value.reduce_size(0);
    return CHIP_NO_ERROR;
}

CHIP_ERROR Storage::SetCertificationDeclaration(const ByteSpan & value)
{
    return Flash::Set(Parameters::ID::kCertification, value.data(), value.size());
}

CHIP_ERROR Storage::GetCertificationDeclaration(MutableByteSpan & value)
{
    size_t size = 0;
    ReturnErrorOnFailure(Flash::Get(Parameters::ID::kCertification, value.data(), value.size(), size));
    value.reduce_size(size);
    return CHIP_NO_ERROR;
}

CHIP_ERROR Storage::SetProductAttestationIntermediateCert(const ByteSpan & value)
{
    return Flash::Set(Parameters::ID::kPaiCert, value.data(), value.size());
}

CHIP_ERROR Storage::GetProductAttestationIntermediateCert(MutableByteSpan & value)
{
    size_t size = 0;
    ReturnErrorOnFailure(Flash::Get(Parameters::ID::kPaiCert, value.data(), value.size(), size));
    value.reduce_size(size);
    return CHIP_NO_ERROR;
}

CHIP_ERROR Storage::SetDeviceAttestationCert(const ByteSpan & value)
{
    return Flash::Set(Parameters::ID::kDacCert, value.data(), value.size());
}

CHIP_ERROR Storage::GetDeviceAttestationCert(MutableByteSpan & value)
{
    size_t size = 0;
    ReturnErrorOnFailure(Flash::Get(Parameters::ID::kDacCert, value.data(), value.size(), size));
    value.reduce_size(size);
    return CHIP_NO_ERROR;
}

CHIP_ERROR Storage::SetDeviceAttestationKey(const ByteSpan & value)
{
    return Flash::Set(Parameters::ID::kDacKey, value.data(), value.size());
}

CHIP_ERROR Storage::GetDeviceAttestationCSR(uint16_t vid, uint16_t pid, const CharSpan &cn, MutableCharSpan & csr)
{
    AttestationKey key;
    uint8_t temp[128] = { 0 };
    size_t size = 0;
    ReturnErrorOnFailure(key.GenerateCSR(vid, pid, cn, csr));
    ReturnErrorOnFailure(key.Export(temp, sizeof(temp), size));
    return Flash::Set(Parameters::ID::kDacKey, temp, size);
}

CHIP_ERROR Storage::SignWithDeviceAttestationKey(const ByteSpan & message, MutableByteSpan & signature)
{
    AttestationKey key;
    uint8_t temp[kDeviceAttestationKeySizeMax] = { 0 };
    size_t size = 0;
    ReturnErrorOnFailure(Flash::Get(Parameters::ID::kDacKey, temp, sizeof(temp), size));
    key.Import(temp, size);
    return key.SignMessage(message, signature);
}

//
// Other
//

CHIP_ERROR Storage::SetProvisionVersion(const char * value, size_t size)
{
    return Flash::Set(Parameters::ID::kVersion, value, size);
}

CHIP_ERROR Storage::GetProvisionVersion(char * value, size_t max, size_t &size)
{
    return Flash::Get(Parameters::ID::kVersion, value, max, size);
}

CHIP_ERROR Storage::SetSetupPayload(const uint8_t * value, size_t size)
{
    return Flash::Set(Parameters::ID::kSetupPayload, value, size);
}

CHIP_ERROR Storage::GetSetupPayload(uint8_t * value, size_t max, size_t &size)
{
    return Flash::Get(Parameters::ID::kSetupPayload, value, max, size);
}

CHIP_ERROR Storage::SetProvisionRequest(bool value)
{
    // return Flash::Set(Parameters::ID::kProvisionRequest, value);
    return CHIP_ERROR_NOT_IMPLEMENTED;
}

CHIP_ERROR Storage::GetProvisionRequest(bool &value)
{
    // return Flash::Set(Parameters::ID::kProvisionRequest, value);
    return CHIP_ERROR_NOT_IMPLEMENTED;

}
#if OTA_ENCRYPTION_ENABLE
CHIP_ERROR Storage::SetOtaTlvEncryptionKey(const ByteSpan & value)
{
    return CHIP_ERROR_NOT_IMPLEMENTED;

}
#endif

} // namespace Provision

void MigrateDacProvider(void) {}

} // namespace Silabs
} // namespace DeviceLayer
} // namespace chip