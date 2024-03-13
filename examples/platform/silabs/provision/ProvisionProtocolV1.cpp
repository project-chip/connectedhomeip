#include "ProvisionProtocol.h"
#include "ProvisionEncoder.h"
#include "ProvisionStorage.h"
#include <lib/support/CodeUtils.h>
#include <lib/core/CHIPEncoding.h>
#include <crypto/CHIPCryptoPAL.h>
#include <mbedtls/x509_crt.h>

namespace chip {
namespace DeviceLayer {
namespace Silabs {
namespace Provision {

#ifdef SILABS_PROVISION_PROTOCOL_V1

enum CommandIds: uint32_t
{
    kCommand_None   = 0,
    kCommand_Init   = 1,
    kCommand_CSR    = 2,
    kCommand_Import = 3,
    kCommand_Setup  = 4,
    kCommand_Read   = 5,
};

enum FileIds
{
    kFile_None = 0,
    kFile_Key = 1,
    kFile_DAC = 2,
    kFile_PAI = 3,
    kFile_CD = 4,
};

constexpr size_t kResponseHeaderSize = 7; // type(1) + command(1) + type(1) +  error(4)

//------------------------------------------------------------------------------
// Protocol
//------------------------------------------------------------------------------

bool Protocol1::Execute(ByteSpan & req, MutableByteSpan & res)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    uint8_t command_id = 0;
    size_t payload_size = 0;

    VerifyOrExit(res.size() > kResponseHeaderSize, err = CHIP_ERROR_BUFFER_TOO_SMALL);
    {
        Encoding::Buffer in((uint8_t*)req.data(), req.size(), true);
        Encoding::Buffer payload(res.data() + kResponseHeaderSize, res.size() - kResponseHeaderSize);

        SuccessOrExit(err = Encoding::Version1::Decode(in, command_id));
        switch(command_id)
        {
        case kCommand_Init:
            err = Init(in, payload);
            break;
        case kCommand_CSR:
            err = GenerateCSR(in, payload);
            break;
        case kCommand_Import:
            err = Import(in, payload);
            break;
        case kCommand_Setup:
            err =  Setup(in, payload);
            break;
        case kCommand_Read:
            err =  Read(in, payload);
            break;
        default:
            err = CHIP_ERROR_INVALID_MESSAGE_TYPE;
        }
        payload_size = payload.Size();
    }
exit:
    Encoding::Buffer out(res.data(), kResponseHeaderSize);
    Encoding::Version1::Encode(out, command_id);
    Encoding::Version1::Encode(out, (uint32_t)err.AsInteger());
    res.reduce_size(kResponseHeaderSize + payload_size);
    // Continue until Setup command is executed
    return (kCommand_Setup != command_id);
}

//------------------------------------------------------------------------------
// Commands
//------------------------------------------------------------------------------


CHIP_ERROR Protocol1::Init(Encoding::Buffer &in, Encoding::Buffer &out)
{
    uint32_t flash_addr = 0;
    uint32_t flash_size = 0;
    uint32_t creds_base_addr = 0;

    // Decode request
    ReturnErrorOnFailure(Encoding::Version1::Decode(in, flash_addr));
    ReturnErrorOnFailure(Encoding::Version1::Decode(in, flash_size));

    // Execute
    ReturnErrorOnFailure(mStore.Initialize(flash_addr, flash_size));
    ReturnErrorOnFailure(mStore.Commit());
    ReturnErrorOnFailure(mStore.GetBaseAddress(creds_base_addr));

    // Encode response
    Encoding::Version1::Encode(out, (uint32_t)creds_base_addr);
    Encoding::Version1::Encode(out, (uint32_t)FLASH_PAGE_SIZE);
    return CHIP_NO_ERROR;
}


CHIP_ERROR Protocol1::GenerateCSR(Encoding::Buffer &in, Encoding::Buffer &out)
{
    char csr[Storage::kCsrLengthMax] = { 0 };
    char common_name[Storage::kCommonNameMax] = { 0 };
    size_t common_name_len = 0;
    uint16_t vendor_id = 0;
    uint16_t product_id = 0;
    uint32_t key_id = 0;

    // Decode request
    ReturnErrorOnFailure(Encoding::Version1::Decode(in, common_name, sizeof(common_name), common_name_len));
    ReturnErrorOnFailure(Encoding::Version1::Decode(in, vendor_id));
    ReturnErrorOnFailure(Encoding::Version1::Decode(in, product_id));
    ReturnErrorOnFailure(Encoding::Version1::Decode(in, key_id));

    // Execute
    MutableCharSpan span(csr, sizeof(csr));
    CharSpan cn(common_name, common_name_len);
    ReturnErrorOnFailure(mStore.GetDeviceAttestationCSR(vendor_id, product_id, cn, span));

    // Encode response
    Encoding::Version1::Encode(out, key_id);
    Encoding::Version1::Encode(out, (uint8_t*)span.data(), span.size());
    return CHIP_NO_ERROR;
}


CHIP_ERROR Protocol1::Import(Encoding::Buffer &in, Encoding::Buffer &out)
{
    uint8_t file_id = 0;
    uint32_t key_id = 0;
    uint8_t do_flash = 0;
    uint8_t *data = Storage::aux_buffer;
    uint32_t offset = 0;
    size_t size = 0;

    //
    // Decode request
    //
    ReturnErrorOnFailure(Encoding::Version1::Decode(in, file_id));
    VerifyOrReturnError((file_id > kFile_None) && (file_id <= kFile_CD), CHIP_ERROR_INVALID_ARGUMENT);
    ReturnErrorOnFailure(Encoding::Version1::Decode(in, key_id));
    ReturnErrorOnFailure(Encoding::Version1::Decode(in, do_flash));
    ReturnErrorOnFailure(Encoding::Version1::Decode(in, data, Storage::kArgumentSizeMax, size));

    //
    // Execute
    //
    switch(file_id)
    {
    case kFile_Key:
        ReturnErrorOnFailure(mStore.Set(Parameters::ID::kDacKey, data, size));
        break;

    case kFile_DAC:
    {
        ReturnErrorOnFailure(mStore.Set(Parameters::ID::kDacCert, data, size));
        break;
    }
    case kFile_PAI:
        ReturnErrorOnFailure(mStore.Set(Parameters::ID::kPaiCert, data, size));
        break;

    case kFile_CD:
        ReturnErrorOnFailure(mStore.Set(Parameters::ID::kCertification, data, size));
        break;;

    default:
        // Unknown file
        return CHIP_ERROR_INVALID_ARGUMENT;
    }
    ReturnErrorOnFailure(mStore.Commit());

    //
    // Encode response
    //
    Encoding::Version1::Encode(out, key_id);
    Encoding::Version1::Encode(out, offset);
    Encoding::Version1::Encode(out, (uint32_t)size);
    return CHIP_NO_ERROR;
}


CHIP_ERROR Protocol1::Setup(Encoding::Buffer &in, Encoding::Buffer &out)
{
    uint16_t vendor_id = 0;
    char vendor_name[Storage::kVendorNameLengthMax] = { 0 };
    size_t vendor_name_size = 0;
    uint16_t product_id = 0;
    char product_name[Storage::kProductNameLengthMax] = { 0 };
    size_t product_name_size = 0;
    char product_label[Storage::kProductLabelLengthMax] = { 0 };
    size_t product_label_size = 0;
    char product_url[Storage::kProductLabelLengthMax] = { 0 };
    size_t product_url_size = 0;
    char part_number[Storage::kPartNumberLengthMax] = { 0 };
    size_t part_number_size = 0;
    uint16_t hw_version = 0;
    char hw_version_string[Storage::kHardwareVersionStrLengthMax] = { 0 };
    size_t hw_version_string_size = 0;
    char manufacturing_date[Storage::kManufacturingDateLengthMax] = { 0 };
    size_t manufacturing_date_size = 0;
    uint8_t unique_id[Storage::kUniqueIdLengthMax] = { 0 };
    size_t unique_size = 0;
    uint16_t discriminator = 0; // 12-bit
    uint32_t spake2p_iterations = 0;
    char spake2p_salt[Storage::kSpake2pSaltB64LengthMax] = { 0 };
    size_t spake2p_salt_size = 0;
    char spake2p_verifier[Storage::kSpake2pVerifierB64LengthMax] = { 0 };
    size_t spake2p_verifier_size = 0;
    uint32_t passcode = 0;
    uint8_t commissioning_flow = 0;
    uint8_t rendezvous_flags = 0;
    uint8_t setup_payload[Storage::kTotalPayloadDataSize] = { 0 };
    size_t setup_payload_size = 0;

    //
    // Decode request
    //

    ReturnErrorOnFailure(Encoding::Version1::Legacy::DecodeInt16u(in, vendor_id));
    ReturnErrorOnFailure(Encoding::Version1::Decode(in, vendor_name, sizeof(vendor_name), vendor_name_size));
    ReturnErrorOnFailure(Encoding::Version1::Legacy::DecodeInt16u(in, product_id));
    ReturnErrorOnFailure(Encoding::Version1::Decode(in, product_name, sizeof(product_name), product_name_size));
    ReturnErrorOnFailure(Encoding::Version1::Decode(in, product_label, sizeof(product_label), product_label_size));
    ReturnErrorOnFailure(Encoding::Version1::Decode(in, product_url, sizeof(product_url), product_url_size));
    ReturnErrorOnFailure(Encoding::Version1::Decode(in, part_number, sizeof(part_number), part_number_size));
    ReturnErrorOnFailure(Encoding::Version1::Decode(in, hw_version));
    ReturnErrorOnFailure(Encoding::Version1::Decode(in, hw_version_string, sizeof(hw_version_string), hw_version_string_size));
    ReturnErrorOnFailure(Encoding::Version1::Decode(in, manufacturing_date, sizeof(manufacturing_date), manufacturing_date_size));
    ReturnErrorOnFailure(Encoding::Version1::Decode(in, unique_id, sizeof(unique_id), unique_size));
    ReturnErrorOnFailure(Encoding::Version1::Legacy::DecodeInt8u(in, commissioning_flow));
    ReturnErrorOnFailure(Encoding::Version1::Legacy::DecodeInt8u(in, rendezvous_flags));
    ReturnErrorOnFailure(Encoding::Version1::Decode(in, discriminator));
    ReturnErrorOnFailure(Encoding::Version1::Decode(in, spake2p_verifier, sizeof(spake2p_verifier), spake2p_verifier_size));
    ReturnErrorOnFailure(Encoding::Version1::Legacy::DecodeInt32u(in, spake2p_iterations));
    ReturnErrorOnFailure(Encoding::Version1::Decode(in, spake2p_salt, sizeof(spake2p_salt), spake2p_salt_size));
    ReturnErrorOnFailure(Encoding::Version1::Decode(in, passcode));

    //
    // Execute
    //

    ReturnErrorOnFailure(mStore.Set(Parameters::ID::kVendorId, (uint16_t*)&vendor_id));
    ReturnErrorOnFailure(mStore.Set(Parameters::ID::kVendorName, (uint8_t*)vendor_name, vendor_name_size));
    ReturnErrorOnFailure(mStore.Set(Parameters::ID::kProductId, (uint16_t*)&product_id));
    ReturnErrorOnFailure(mStore.Set(Parameters::ID::kProductName, (uint8_t*)product_name, product_name_size));
    ReturnErrorOnFailure(mStore.Set(Parameters::ID::kProductLabel, (uint8_t*)product_label, product_label_size));
    ReturnErrorOnFailure(mStore.Set(Parameters::ID::kProductUrl, (uint8_t*)product_url, product_url_size));
    ReturnErrorOnFailure(mStore.Set(Parameters::ID::kPartNumber, (uint8_t*)part_number, part_number_size));
    ReturnErrorOnFailure(mStore.Set(Parameters::ID::kHwVersion, &hw_version));
    ReturnErrorOnFailure(mStore.Set(Parameters::ID::kHwVersionStr, (uint8_t*)hw_version_string, hw_version_string_size));
    ReturnErrorOnFailure(mStore.Set(Parameters::ID::kManufacturingDate, (uint8_t*)manufacturing_date, manufacturing_date_size));
    ReturnErrorOnFailure(mStore.Set(Parameters::ID::kUniqueId, unique_id, unique_size));
    ReturnErrorOnFailure(mStore.Set(Parameters::ID::kDiscriminator, discriminator > 0 ? &discriminator : nullptr));
    ReturnErrorOnFailure(mStore.Set(Parameters::ID::kSpake2pVerifier, (uint8_t*)spake2p_verifier, spake2p_verifier_size));
    ReturnErrorOnFailure(mStore.Set(Parameters::ID::kSpake2pPasscode, passcode > 0 ? &passcode : nullptr));
    ReturnErrorOnFailure(mStore.Set(Parameters::ID::kCommissioningFlow, (uint8_t*)&commissioning_flow));
    ReturnErrorOnFailure(mStore.Set(Parameters::ID::kRendezvousFlags, (uint8_t*)&rendezvous_flags));
    ReturnErrorOnFailure(mStore.Set(Parameters::ID::kSpake2pIterations, &spake2p_iterations));
    ReturnErrorOnFailure(mStore.Set(Parameters::ID::kSpake2pSalt, (uint8_t*)spake2p_salt, spake2p_salt_size));
    ReturnErrorOnFailure(mStore.Set(Parameters::ID::kSetupPayload, nullptr, 0));
    ReturnErrorOnFailure(mStore.Commit());

    //
    // Encode response
    //
    ReturnErrorOnFailure(mStore.Get(Parameters::ID::kSpake2pPasscode, passcode));
    ReturnErrorOnFailure(mStore.Get(Parameters::ID::kDiscriminator, discriminator));
    ReturnErrorOnFailure(mStore.Get(Parameters::ID::kUniqueId, unique_id, sizeof(unique_id), unique_size));
    ReturnErrorOnFailure(mStore.Get(Parameters::ID::kSetupPayload, setup_payload, sizeof(setup_payload), setup_payload_size));
    Encoding::Version1::Encode(out, passcode);
    Encoding::Version1::Encode(out, discriminator);
    Encoding::Version1::Encode(out, unique_id, unique_size);
    Encoding::Version1::Encode(out, setup_payload, setup_payload_size);
    return CHIP_NO_ERROR; // spake2p_passcode, discriminator, unique_id, setup_payload
}


CHIP_ERROR Protocol1::Read(Encoding::Buffer &in, Encoding::Buffer &out)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    //
    // Decode / Encode
    //
    uint16_t id = 0;
    uint8_t type = 0;

    while(in.Left() > 0)
    {
        ReturnErrorOnFailure(Encoding::Version1::Decode(in, id));
        ReturnErrorOnFailure(Encoding::Version1::Decode(in, type));
        ReturnErrorOnFailure(Encoding::Version1::Encode(out, id));
        ReturnErrorOnFailure(Encoding::Version1::Encode(out, type));
        switch(type)
        {
        case Encoding::Version1::Type_Int8u:
        {
            uint8_t value = 0;
            err = mStore.Get(id, value);
            VerifyOrReturnError((CHIP_ERROR_NOT_FOUND == err) || (CHIP_NO_ERROR == err), err);
            ReturnErrorOnFailure(Encoding::Version1::Encode(out, value));
            break;
        }
        case Encoding::Version1::Type_Int16u:
        {
            uint16_t value = 0;
            err = mStore.Get(id, value);
            VerifyOrReturnError((CHIP_ERROR_NOT_FOUND == err) || (CHIP_NO_ERROR == err), err);
            ReturnErrorOnFailure(Encoding::Version1::Encode(out, value));
            break;
        }
        case Encoding::Version1::Type_Int32u:
        {
            uint32_t value = 0;
            err = mStore.Get(id, value);
            VerifyOrReturnError((CHIP_ERROR_NOT_FOUND == err) || (CHIP_NO_ERROR == err), err);
            ReturnErrorOnFailure(Encoding::Version1::Encode(out, value));
            break;
        }
        case Encoding::Version1::Type_Array:
        {
            uint8_t *value = Storage::aux_buffer;
            size_t size = 0;
            err = mStore.Get(id, value, Storage::kArgumentSizeMax, size);
            VerifyOrReturnError((CHIP_ERROR_NOT_FOUND == err) || (CHIP_NO_ERROR == err), err);
            ReturnErrorOnFailure(Encoding::Version1::Encode(out, value, size));
            break;
        }
        default:
            return CHIP_ERROR_WRONG_TLV_TYPE;
        }
    }

    return CHIP_NO_ERROR;
}

#endif // SILABS_PROVISION_PROTOCOL_V1

} // namespace Provision
} // namespace Silabs
} // namespace DeviceLayer
} // namespace chip
