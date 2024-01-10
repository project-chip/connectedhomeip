#include "ProvisionCommands.h"
#include "ProvisionEncoder.h"
#include <lib/support/CodeUtils.h>
#include <crypto/CHIPCryptoPAL.h>
#include <mbedtls/x509_crt.h>
#include <string.h>
#include <stdio.h>


namespace chip {
namespace DeviceLayer {
namespace Silabs {
namespace Provision {

//------------------------------------------------------------------------------
// Utils
//------------------------------------------------------------------------------

size_t writeQR(uint8_t bits[], size_t offset, uint32_t input, size_t numberOfBits, size_t totalPayloadSizeInBits)
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

void generateQR(uint8_t bits[], uint32_t vendor_id, uint32_t product_id,
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

CHIP_ERROR parseCert(const uint8_t * cert_data, size_t cert_size, uint8_t *serial_num, size_t serial_max, size_t &serial_size)
{
    mbedtls_x509_crt cert;

    mbedtls_x509_crt_init(&cert);

    serial_size = 0;
    int err = mbedtls_x509_crt_parse_der(&cert, cert_data, cert_size);
    VerifyOrReturnError(0 == err, CHIP_ERROR_INTERNAL);

    serial_size = cert.serial.len > serial_max ? serial_max : cert.serial.len;
    memcpy(serial_num, cert.serial.p, serial_size);

    mbedtls_x509_crt_free(&cert);
    return CHIP_NO_ERROR;
}

//------------------------------------------------------------------------------
// Init
//------------------------------------------------------------------------------

CHIP_ERROR InitCommand::DecodeRequest(Encoder & in)
{
    ReturnErrorOnFailure(in.getUint32(_flash_addr));
    ReturnErrorOnFailure(in.getUint32(_flash_size));
    return CHIP_NO_ERROR;
}

CHIP_ERROR InitCommand::Execute(Storage & store)
{
    ReturnErrorOnFailure(store.Initialize(_flash_addr, _flash_size));
    ReturnErrorOnFailure(store.GetBaseAddress(_creds_base_addr));
    return CHIP_NO_ERROR;
}

void InitCommand::EncodeResponse(Encoder & out) const
{
    out.addUint32((uint32_t)_creds_base_addr);
    out.addUint32((uint32_t)FLASH_PAGE_SIZE);
}

//------------------------------------------------------------------------------
// CSR
//------------------------------------------------------------------------------

CHIP_ERROR CsrCommand::DecodeRequest(Encoder & in)
{
    ReturnErrorOnFailure(in.getString(_common_name, sizeof(_common_name), _common_name_len));
    ReturnErrorOnFailure(in.getUint16(_vendor_id));
    ReturnErrorOnFailure(in.getUint16(_product_id));
    ReturnErrorOnFailure(in.getUint32(_key_id));
    return CHIP_NO_ERROR;
}

CHIP_ERROR CsrCommand::Execute(Storage & store)
{
    MutableCharSpan csr(_csr, sizeof(_csr));
    CharSpan cn(_common_name, _common_name_len);
    return store.GetDeviceAttestationCSR(_key_id, _vendor_id, _product_id, cn, csr);
}

void CsrCommand::EncodeResponse(Encoder & out) const
{
    out.addUint32(_key_id);
    out.addArray((uint8_t*)_csr, _csr_len);
}

//------------------------------------------------------------------------------
// Import
//------------------------------------------------------------------------------

CHIP_ERROR ImportCommand::DecodeRequest(Encoder & in)
{
    ReturnErrorOnFailure(in.getUint8(_file_id));
    VerifyOrReturnError((_file_id > kFile_None) && (_file_id <= kFile_CD), CHIP_ERROR_INVALID_ARGUMENT);

    ReturnErrorOnFailure(in.getUint32(_key_id));
    ReturnErrorOnFailure(in.getUint8(_flash));
    ReturnErrorOnFailure(in.getArray(_data, sizeof(_data), _size));

    return CHIP_NO_ERROR;
}

// FIXME: DAC Serial Number is used as Device SerialNumber. This number
// should be provided as an input of the provisioning script instead.
// https://jira.silabs.com/browse/MATTER-2513
uint8_t serial_num[kSerialNumberLengthMax];
size_t serial_size = 0;
void parseSerialNum(const uint8_t *sn_raw, size_t size, char *sn_str, size_t &sn_len)
{
    sn_len = 0;
    for(size_t i = 0; i < size; ++i)
    {
        sprintf(&sn_str[sn_len], "%02x", sn_raw[i]);
        sn_len += 2;
    }
    sn_str[sn_len] = 0;
}

CHIP_ERROR ImportCommand::Execute(Storage & store)
{
    switch(_file_id)
    {
    case kFile_Key:
        return store.SetDeviceAttestationKey(_key_id, ByteSpan(_data, _size));

    case kFile_DAC:
    {
        // serial_number
        ReturnErrorOnFailure(parseCert(_data, _size, serial_num, sizeof(serial_num), serial_size));

        char serial_str[kSerialNumberLengthMax];
        size_t serial_len = 0;
        parseSerialNum(serial_num, serial_size, serial_str, serial_len);
        ReturnErrorOnFailure(store.SetSerialNumber(serial_str, serial_len));
        return store.SetDeviceAttestationCert(ByteSpan(_data, _size));
    }
    case kFile_PAI:
        return store.SetProductAttestationIntermediateCert(ByteSpan(_data, _size));
    case kFile_CD:
        return store.SetCertificationDeclaration(ByteSpan(_data, _size));
    default:
        // Unknown file
        return CHIP_ERROR_INVALID_ARGUMENT;
    }
}

void ImportCommand::EncodeResponse(Encoder & out) const
{
    out.addUint32(_key_id);
    out.addUint32(_offset);
    out.addUint32((uint32_t)_size);
}

//------------------------------------------------------------------------------
// Setup Command
//------------------------------------------------------------------------------

CHIP_ERROR SetupCommand::DecodeRequest(Encoder & in)
{
    // vendor_id
    ReturnErrorOnFailure(in.getUint32(_vendor_id));
    // vendor_name
    ReturnErrorOnFailure(in.getString(_vendor_name, sizeof(_vendor_name), _vendor_name_size));
    // product_id
    ReturnErrorOnFailure(in.getUint32(_product_id));
    // product_name
    ReturnErrorOnFailure(in.getString(_product_name, sizeof(_product_name), _product_name_size));
    // product_label
    ReturnErrorOnFailure(in.getString(_product_label, sizeof(_product_label), _product_label_size));
    // product_url
    ReturnErrorOnFailure(in.getString(_product_url, sizeof(_product_url), _product_url_size));
    // part_number
    ReturnErrorOnFailure(in.getString(_part_number, sizeof(_part_number), _part_number_size));
    // hw_version
    ReturnErrorOnFailure(in.getUint16(_hw_version));
    // hw_version_str
    ReturnErrorOnFailure(in.getString(_hw_version_string, sizeof(_hw_version_string), _hw_version_string_size));
    // manufacturing_date
    ReturnErrorOnFailure(in.getString(_manufacturing_date, sizeof(_manufacturing_date), _manufacturing_date_size));
    // unique_id
    ReturnErrorOnFailure(in.getArray(_unique_id, sizeof(_unique_id), _unique_size));
    // commissioning_flow
    ReturnErrorOnFailure(in.getUint32(_commissioning_flow));
    // rendezvous_flags
    ReturnErrorOnFailure(in.getUint32(_rendezvous_flags));
    // discriminator
    ReturnErrorOnFailure(in.getUint16(_discriminator));
    // spake2p_verifier
    ReturnErrorOnFailure(in.getString(_spake2p_verifier, sizeof(_spake2p_verifier), _spake2p_verifier_size));
    // spake2p_iterations
    ReturnErrorOnFailure(in.getUint32(_spake2p_iterations));
    // spake2p_salt
    ReturnErrorOnFailure(in.getString(_spake2p_salt, sizeof(_spake2p_salt), _spake2p_salt_size));
    // passcode
    ReturnErrorOnFailure(in.getUint32(_passcode));

    return CHIP_NO_ERROR;
}

CHIP_ERROR SetupCommand::Execute(Storage & store)
{
    // vendor_id
    ReturnErrorOnFailure(store.SetVendorId(_vendor_id));
    // vendor_name
    if(_vendor_name_size > 0)
    {
        ReturnErrorOnFailure(store.SetVendorName(_vendor_name, _vendor_name_size));
    }
    // product_id
    ReturnErrorOnFailure(store.SetProductId(_product_id));
    // product_name
    if(_product_name_size > 0)
    {
        ReturnErrorOnFailure(store.SetProductName(_product_name, _product_name_size));
    }
    // product_label
    if(_product_label_size > 0)
    {
        ReturnErrorOnFailure(store.SetProductLabel(_product_label, _product_label_size));
    }
    // product_url
    if(_product_url_size > 0)
    {
        ReturnErrorOnFailure(store.SetProductURL(_product_url, _product_url_size));
    }
    // part_number
    if(_part_number_size > 0)
    {
        ReturnErrorOnFailure(store.SetPartNumber(_part_number, _part_number_size));
    }
    // hw_version
    ReturnErrorOnFailure(store.SetHardwareVersion(_hw_version));
    // hw_version_str
    if(0 == _hw_version_string_size)
    {
        // string(hw_version)
        _hw_version_string_size = snprintf((char*)_hw_version_string, sizeof(_hw_version_string), "%d", _hw_version);
    }
    ReturnErrorOnFailure(store.SetHardwareVersionString(_hw_version_string, _hw_version_string_size));
    // manufacturing_date
    if(_manufacturing_date_size > 0)
    {
        ReturnErrorOnFailure(store.SetManufacturingDate(_manufacturing_date, _manufacturing_date_size));
    }
    // unique_id, discriminator
    if((0 == _unique_size) || (0 == _discriminator))
    {
        // Calculate hash
        // char serial_num[kSerialNumberLengthMax];
        // ReturnErrorOnFailure(store.GetSerialNumber(serial_num, kSerialNumberLengthMax));
        ReturnErrorOnFailure(Crypto::Hash_SHA256((const uint8_t *)serial_num, serial_size, _hash));
        _hash_size = chip::Crypto::kSHA256_Hash_Length;
        if(0 == _unique_size)
        {
            // Default unique_id: First 128-bit of the hash
            _unique_size = kUniqueIdLengthMax;
            memcpy(_unique_id, _hash, _unique_size);
        }
        if(0 == _discriminator)
        {
            // // Default discriminator: Last 12-bit of the hash
            _discriminator = ((_hash[_hash_size - 2] << 8) | _hash[_hash_size - 1]) & 0xfff;
        }
    }
    ReturnErrorOnFailure(store.SetUniqueId(_unique_id, _unique_size));
    ReturnErrorOnFailure(store.SetSetupDiscriminator(_discriminator));

    // spake2p_verifier
    ReturnErrorOnFailure(store.SetSpake2pVerifier(_spake2p_verifier, _spake2p_verifier_size));
    // spake2p_iterations
    ReturnErrorOnFailure(store.SetSpake2pIterationCount(_spake2p_iterations));
    // spake2p_salt
    ReturnErrorOnFailure(store.SetSpake2pSalt(_spake2p_salt, _spake2p_salt_size));

    // setup_payload: commissioning_flow, rendezvous_flags, passcode
    generateQR(_setup_payload, _vendor_id, _product_id, _commissioning_flow, _rendezvous_flags, _discriminator, _passcode);
    ReturnErrorOnFailure(store.SetSetupPayload(_setup_payload, kTotalPayloadDataSize));

    return CHIP_NO_ERROR;
}

void SetupCommand::EncodeResponse(Encoder & out) const
{
    out.addUint32(_passcode);
    out.addUint16(_discriminator);
    out.addArray(_unique_id, _unique_size);
    out.addArray(_setup_payload, sizeof(_setup_payload));
}


} // namespace Provision
} // namespace Silabs
} // namespace DeviceLayer
} // namespace chip
