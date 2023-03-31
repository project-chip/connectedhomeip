#include "commands.h"
#include "credentials.h"
#include "assert.h"
#include <em_msc.h>
#include <nvm3.h>
#include <nvm3_default.h>
#include <nvm3_hal_flash.h>
#include <string.h>

using namespace chip::DeviceLayer::Internal;

//------------------------------------------------------------------------------
// Utils
//------------------------------------------------------------------------------

int round_up(int value, int multiple)
{
    if (multiple == 0)
    {
        return value;
    }
    int remainder = value % multiple;
    if (remainder == 0)
    {
        return value;
    }
    return value + multiple - remainder;
}

//------------------------------------------------------------------------------
// Void
//------------------------------------------------------------------------------

int VoidCommand::decode(Encoder & in)
{
    return 0;
}

int VoidCommand::execute()
{
    return 0;
}

void VoidCommand::encode(Encoder & out) const
{
}

//------------------------------------------------------------------------------
// Debug
//------------------------------------------------------------------------------

int DebugCommand::decode(Encoder & in)
{
    int err = 0;

    err = in.getUint32(_key_id);
    ASSERT(!err, return err, "Decode error");

    return 0;
}

int DebugCommand::execute()
{
    return 0;
}

void DebugCommand::encode(Encoder & out) const
{
    out.addUint32(_key_id);
}

//------------------------------------------------------------------------------
// CSR
//------------------------------------------------------------------------------

int CsrCommand::decode(Encoder & in)
{
    int err = 0;

    err = in.getString(_common_name, sizeof(_common_name), _common_name_len);
    ASSERT(!err, return err, "Decode error");

    err = in.getUint16(_vendor_id);
    ASSERT(!err, return err, "Decode error");

    err = in.getUint16(_product_id);
    ASSERT(!err, return err, "Decode error");

    err = in.getUint32(_key_id);
    ASSERT(!err, return err, "Decode error");

    return 0;
}

int CsrCommand::execute()
{
    Credentials creds = Credentials(_key_id);

    int err = creds.generateKey();
    ASSERT(!err, return err, "Key generation error %d", err);

    _key_id = creds.getKeyId();
    err = creds.generateCSR(_common_name, _vendor_id, _product_id, _csr, sizeof(_csr), &_csr_len);
    ASSERT(!err, return err, "CSR generation error %d", err);

    return 0;
}

void CsrCommand::encode(Encoder & out) const
{
    out.addUint32(_key_id);
    out.addArray((uint8_t*)_csr, _csr_len);
}

//------------------------------------------------------------------------------
// Import
//------------------------------------------------------------------------------

extern uint8_t linker_nvm_end[];
uint32_t * _credentials_page_address = (uint32_t *)linker_nvm_end;

static uint8_t _credentials_page[FLASH_PAGE_SIZE] = { 0 };
size_t _credentials_page_size = FLASH_PAGE_SIZE;

static constexpr size_t kCreds_DAC_Offset_Default        = 0x000;
static constexpr size_t kCreds_PAI_Offset_Default        = 0x200;
static constexpr size_t kCreds_CD_Offset_Default         = 0x400;
static constexpr size_t kMaxSerialNumberLength           = 32;

int ImportCommand::decode(Encoder & in)
{
    int err = 0;

    err = in.getUint8(_file_id);
    ASSERT(!err, return err, "File ID decode error");
    ASSERT((_file_id > kFile_None) && (_file_id <= kFile_CD), err = 1; return err, "Invalid file ID");

    err = in.getUint32(_key_id);
    ASSERT(!err, return err, "Decode error");

    err = in.getUint8(_flash);
    ASSERT(!err, return err, "Decode error");

    err = in.getArray(_data, sizeof(_data), _size);
    ASSERT(!err, return err, "Decode error");

    return err;
}

int ImportCommand::execute()
{
    switch(_file_id)
    {
        case kFile_Key:
            return writeKey(_data, _size);
        case kFile_DAC:
            return writeDAC(_data, _size);
        case kFile_PAI:
            return writePAI(_data, _size);
        case kFile_CD:
            return writeCD(_data, _size);
        default:
            // Unknown file
            return -1;
    }
}

void ImportCommand::encode(Encoder & out) const
{
    out.addUint32(_key_id);
    out.addUint32(_address);
    out.addUint32(_offset);
    out.addUint32((uint32_t)_size);
    // out.addArray(_data, _size);
}

int ImportCommand::writeKey(const uint8_t *data, size_t size)
{
    Credentials creds = Credentials(_key_id);
    int err = creds.importKey(data, size);
    _key_id = creds.getKeyId();
    return err;
}

int ImportCommand::writeDAC(const uint8_t *data, size_t size)
{
    uint8_t serial_num[kMaxSerialNumberLength];
    size_t serial_size = 0;

    // serial_number
    int err = Credentials::parseCert(data, size, serial_num, sizeof(serial_num), serial_size);
    ASSERT(!err, return err, "Decode error");

    err = Config::WriteBin(SilabsConfig::kConfigKey_SerialNum, serial_num, serial_size);
    ASSERT(!err, return err, "Write error");

    size_t offset = kCreds_DAC_Offset_Default;
    return writeFile(SilabsConfig::kConfigKey_Creds_DAC_Offset, SilabsConfig::kConfigKey_Creds_DAC_Size, offset, data, size);
}

int ImportCommand::writePAI(const uint8_t *data, size_t size)
{
    size_t offset = kCreds_PAI_Offset_Default;
    return writeFile(SilabsConfig::kConfigKey_Creds_PAI_Offset, SilabsConfig::kConfigKey_Creds_PAI_Size, offset, data, size);
}

int ImportCommand::writeCD(const uint8_t *data, size_t size)
{
    size_t offset = kCreds_CD_Offset_Default;
    return writeFile(SilabsConfig::kConfigKey_Creds_CD_Offset, SilabsConfig::kConfigKey_Creds_CD_Size, offset, data, size);
}

int ImportCommand::writeFile(Key offset_key, Key size_key, uint32_t offset, const uint8_t *data, size_t size)
{
    uint32_t *base_addr = _credentials_page_address;

    // Write file 
    memcpy(&_credentials_page[offset], data, size);

    // Store file offset
    int err = Config::Write(offset_key, offset);
    ASSERT(!err, return err, "Config write failed");

    // Store file size
    err = Config::Write(size_key, (uint32_t)size);
    ASSERT(!err, return err, "NVM3 read error");

    if(_flash)
    {
        MSC_ErasePage(_credentials_page_address);

        // Read page address
        err = Config::Write(SilabsConfig::kConfigKey_Creds_Base_Addr, (uint32_t)base_addr);
        ASSERT(!err, return err, "write read error");

        // Write to flash
        MSC_WriteWord(base_addr, _credentials_page, _credentials_page_size);
    }

    _address = (uint32_t)base_addr;
    _offset = offset;
    _size = size;
    return err;
}


//------------------------------------------------------------------------------
// Setup Command
//------------------------------------------------------------------------------

int SetupCommand::decode(Encoder & in)
{
    uint8_t temp[512] = { 0 };
    size_t size = 0;
    int err = 0;

    // vendor_id
    err = in.getUint32(_vendor_id);
    ASSERT(!err, goto exit, "Decode error");

    // vendor_name
    err = in.getArray(temp, sizeof(temp), size);
    ASSERT(!err, goto exit, "Decode error");
    if(size > 0)
    {
        err = Config::WriteBin(SilabsConfig::kConfigKey_VendorName, temp, size);
        ASSERT(!err, goto exit, "Write error");
    }

    // product_id
    err = in.getUint32(_product_id);
    ASSERT(!err, goto exit, "Decode error");

    // product_name
    err = in.getArray(temp, sizeof(temp), size);
    ASSERT(!err, goto exit, "Decode error");
    if(size > 0)
    {
        err = Config::WriteBin(SilabsConfig::kConfigKey_ProductName, temp, size);
        ASSERT(!err, goto exit, "Write error");
    }

    // product_label
    err = in.getArray(temp, sizeof(temp), size);
    ASSERT(!err, goto exit, "Decode error");
    if(size > 0)
    {
        err = Config::WriteBin(SilabsConfig::KConfigKey_ProductLabel, temp, size);
        ASSERT(!err, goto exit, "Write error");
    }

    // product_url
    err = in.getArray(temp, sizeof(temp), size);
    ASSERT(!err, goto exit, "Decode error");
    if(size > 0)
    {
        err = Config::WriteBin(SilabsConfig::kConfigKey_ProductURL, temp, size);
        ASSERT(!err, goto exit, "Write error");
    }

    // part_number
    err = in.getArray(temp, sizeof(temp), size);
    ASSERT(!err, goto exit, "Decode error");
    if(size > 0)
    {
        err = Config::WriteBin(SilabsConfig::kConfigKey_PartNumber, temp, size);
        ASSERT(!err, goto exit, "Write error");
    }

    // hw_version
    err = in.getArray(temp, sizeof(temp), size);
    ASSERT(!err, goto exit, "Decode error");
    if(size > 0)
    {
        err = Config::WriteBin(SilabsConfig::kConfigKey_HardwareVersion, temp, size);
        ASSERT(!err, goto exit, "Write error");
    }

    // hw_version_str
    err = in.getArray(temp, sizeof(temp), size);
    ASSERT(!err, goto exit, "Decode error");
    if(size > 0)
    {
        err = Config::WriteBin(SilabsConfig::kConfigKey_HardwareVersionString, temp, size);
        ASSERT(!err, goto exit, "Write error");
    }

    // manufacturing_date
    err = in.getArray(temp, sizeof(temp), size);
    ASSERT(!err, goto exit, "Decode error");
    if(size > 0)
    {
        err = Config::WriteBin(SilabsConfig::kConfigKey_ManufacturingDate, temp, size);
        ASSERT(!err, goto exit, "Write error");
    }

    // unique_id
    err = in.getArray(_unique_id, sizeof(_unique_id), _unique_size);
    ASSERT(!err, goto exit, "Decode error");

    // commissioning_flow
    err = in.getUint32(_commissioning_flow);
    ASSERT(!err, goto exit, "Decode error");

    // rendezvous_flag
    err = in.getUint32(_rendezvous_flag);
    ASSERT(!err, goto exit, "Decode error");

    // discriminator
    err = in.getUint16(_discriminator);
    ASSERT(!err, goto exit, "Decode error");

    // spake2p_verifier
    err = in.getArray(temp, sizeof(temp), size);
    ASSERT(!err, goto exit, "Decode error");
    err = Config::WriteBin(SilabsConfig::kConfigKey_Spake2pVerifier, temp, size);
    ASSERT(!err, goto exit, "Write error");

    // spake2p_iterations
    err = in.getArray(temp, sizeof(temp), size);
    ASSERT(!err, goto exit, "Decode error");
    err = Config::WriteBin(SilabsConfig::kConfigKey_Spake2pIterationCount, temp, size);
    ASSERT(!err, goto exit, "Write error");

    // spake2p_salt
    err = in.getArray(temp, sizeof(temp), size);
    ASSERT(!err, goto exit, "Decode error");
    err = Config::WriteBin(SilabsConfig::kConfigKey_Spake2pSalt, temp, size);
    ASSERT(!err, goto exit, "Write error");

    // passcode
    err = in.getUint32(_passcode);
    ASSERT(!err, goto exit, "Decode error");

exit:
    return err;
}

int SetupCommand::execute()
{
    int err = 0;

    // vendor_id
    err = Config::Write(SilabsConfig::kConfigKey_VendorId, _vendor_id);
    ASSERT(!err, goto exit, "Write error");

    // product_id
    err = Config::Write(SilabsConfig::kConfigKey_ProductId, _product_id);
    ASSERT(!err, goto exit, "Write error");

    // unique_id
    if(0 == _unique_size)
    {
        err = computeHash(_hash, _hash_size);
        ASSERT(!err, goto exit, "Hash error");
        _unique_size = sizeof(_unique_id);
        // First 128-bit of the hash
        memcpy(_unique_id, _hash, _unique_size);
    }
    err = Config::WriteBin(SilabsConfig::kConfigKey_UniqueId, _unique_id, _unique_size);
    ASSERT(!err, goto exit, "Write error");

    // discriminator
    if(0 == _discriminator)
    {
        err = computeHash(_hash, _hash_size);
        ASSERT(!err, goto exit, "Hash error");
        // Last 12-bit of the hash
        _discriminator = ((_hash[_hash_size - 2] << 8) | _hash[_hash_size - 1]) & 0xfff;
    }
    err = Config::Write(SilabsConfig::kConfigKey_SetupDiscriminator, _discriminator);
    ASSERT(!err, goto exit, "Write error");

    // setup_payload
    generateQR(_setup_payload, _vendor_id, _product_id, _commissioning_flow, _rendezvous_flag, _discriminator, _passcode);
    err = Config::WriteBin(SilabsConfig::kConfigKey_SetupPayloadBitSet, _setup_payload, sizeof(_setup_payload));
    ASSERT(!err, goto exit, "Write error");


exit:
    return err;
}

void SetupCommand::encode(Encoder & out) const
{
    out.addUint32(_passcode);
    out.addUint16(_discriminator);
    out.addArray(_unique_id, _unique_size);
    out.addArray(_setup_payload, sizeof(_setup_payload));
    // out.addArray(_hash, _hash_size);
}

void SetupCommand::generateQR(uint8_t bits[], uint32_t vendor_id, uint32_t product_id,
        uint32_t commissioning_flow, uint32_t rendezvous_flag, uint32_t discriminator, uint32_t passcode)
{
    size_t offset = 0;

    offset = writeQR(bits, offset, 0, kVersionFieldLengthInBits, kTotalPayloadDataSizeInBits);
    offset = writeQR(bits, offset, vendor_id, kVendorIDFieldLengthInBits, kTotalPayloadDataSizeInBits);
    offset = writeQR(bits, offset, product_id, kProductIDFieldLengthInBits, kTotalPayloadDataSizeInBits);
    offset = writeQR(bits, offset, commissioning_flow, kCommissioningFlowFieldLengthInBits, kTotalPayloadDataSizeInBits);
    offset = writeQR(bits, offset, rendezvous_flag, kRendezvousInfoFieldLengthInBits, kTotalPayloadDataSizeInBits);
    offset = writeQR(bits, offset, discriminator, kPayloadDiscriminatorFieldLengthInBits, kTotalPayloadDataSizeInBits);
    offset = writeQR(bits, offset, passcode, kSetupPINCodeFieldLengthInBits, kTotalPayloadDataSizeInBits);
    offset = writeQR(bits, offset, 0, kPaddingFieldLengthInBits, kTotalPayloadDataSizeInBits);
}

size_t SetupCommand::writeQR(uint8_t bits[], size_t offset, uint32_t input, size_t numberOfBits, size_t totalPayloadSizeInBits)
{
    ASSERT((offset + numberOfBits) <= totalPayloadSizeInBits, return 0, "Invalid input.");
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

int SetupCommand::computeHash(uint8_t *hash, size_t & hash_size)
{
    // The hash (256-bit) is used to calculate default values for the
    // the unique_id (first 128-bit) and discriminator (last 16-bit)
    if(0 == hash_size)
    {
        uint8_t serial_num[kMaxSerialNumberLength];
        size_t serial_size = 0;

        int err = Config::ReadBin(SilabsConfig::kConfigKey_SerialNum, serial_num, sizeof(serial_num), serial_size);
        ASSERT(!err, return err, "Read error");

        err = psa_hash_compute(PSA_ALG_SHA_256, serial_num, serial_size, hash, PSA_HASH_LENGTH(PSA_ALG_SHA_256), &hash_size);
        ASSERT(!err, return err, "Read error");
    }
    return 0;
}
