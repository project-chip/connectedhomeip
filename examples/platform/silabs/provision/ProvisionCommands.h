#pragma once

#include "ProvisionEncoder.h"
#include "ProvisionStorage.h"
#include <lib/core/CHIPError.h>
#include <stddef.h>
#include <stdint.h>

namespace chip {
namespace DeviceLayer {
namespace Silabs {
namespace Provision {

enum CommandIds: uint32_t
{
    kCommand_None = 0,
    kCommand_Init = 1,
    kCommand_CSR = 2,
    kCommand_Import = 3,
    kCommand_Setup = 4,
    kCommand_SPAKE2p = 11,
    kCommand_NVM3 = 12,
};

enum FileIds
{
    kFile_None = 0,
    kFile_Key = 1,
    kFile_DAC = 2,
    kFile_PAI = 3,
    kFile_CD = 4,
};

struct Command
{
    Command(uint8_t id): _id(id) {}
    virtual ~Command() {}
    virtual CHIP_ERROR DecodeRequest(Encoder & in) = 0;
    virtual CHIP_ERROR Execute(Storage & store) = 0;
    virtual void EncodeResponse(Encoder & out) const = 0;

    uint8_t id() { return _id; }
private:
    uint8_t _id = 0;
};

class InitCommand : public Command
{
public:
    InitCommand(): Command(kCommand_Init) {}
    CHIP_ERROR DecodeRequest(Encoder & in) override;
    CHIP_ERROR Execute(Storage & store) override;
    void EncodeResponse(Encoder & out) const override;

private:
    uint32_t _flash_addr = 0;
    uint32_t _flash_size = 0;
    uint32_t _creds_base_addr = 0;
};


class CsrCommand : public Command
{
public:
    static constexpr size_t kCsrLengthMax = 1024;
    static constexpr size_t kCommonNameMax = 128;

    CsrCommand(): Command(kCommand_CSR) {}
    CHIP_ERROR DecodeRequest(Encoder & in) override;
    CHIP_ERROR Execute(Storage & store) override;
    void EncodeResponse(Encoder & out) const override;

private:
    char _csr[kCsrLengthMax] = { 0 };
    size_t _csr_len = 0;
    char _common_name[kCommonNameMax] = { 0 };
    size_t _common_name_len = 0;
    uint16_t _vendor_id = 0;
    uint16_t _product_id = 0;
    uint32_t _key_id = 0;
};


class ImportCommand : public Command
{
public:
    static constexpr size_t kFileSizeMax = 1024;

    ImportCommand(): Command(kCommand_Import) {}
    CHIP_ERROR DecodeRequest(Encoder & in) override;
    CHIP_ERROR Execute(Storage & store) override;
    void EncodeResponse(Encoder & out) const override;

private:
    // CHIP_ERROR writeKey(const uint8_t *data, size_t size);
    // CHIP_ERROR writeDAC(const uint8_t *data, size_t size);
    // CHIP_ERROR writePAI(const uint8_t *data, size_t size);
    // CHIP_ERROR writeCD(const uint8_t *data, size_t size);

    uint8_t _file_id = 0;
    uint32_t _key_id = 0;
    uint8_t _flash = 0;
    uint8_t _data[kFileSizeMax] = { 0 };
    uint32_t _offset = 0;
    size_t _size = 0;
};


class SetupCommand : public Command
{
public:
    SetupCommand(): Command(kCommand_Setup) {}
    CHIP_ERROR DecodeRequest(Encoder & in) override;
    CHIP_ERROR Execute(Storage & store) override;
    void EncodeResponse(Encoder & out) const override;

private:
    uint32_t _vendor_id = 0;
    char _vendor_name[64] = { 0 };
    size_t _vendor_name_size = 0;
    uint32_t _product_id = 0;
    char _product_name[kProductNameLengthMax] = { 0 };
    size_t _product_name_size = 0;
    char _product_label[kProductLabelLengthMax] = { 0 };
    size_t _product_label_size = 0;
    char _product_url[kProductLabelLengthMax] = { 0 };
    size_t _product_url_size = 0;
    char _part_number[kPartNumberLengthMax] = { 0 };
    size_t _part_number_size = 0;
    uint16_t _hw_version = 0;
    char _hw_version_string[kHardwareVersionStrLengthMax] = { 0 };
    size_t _hw_version_string_size = 0;
    char _manufacturing_date[kManufacturingDateLengthMax] = { 0 };
    size_t _manufacturing_date_size = 0;
    uint8_t _unique_id[kUniqueIdLengthMax] = { 0 };
    size_t _unique_size = 0;
    uint16_t _discriminator = 0; // 12-bit
    uint32_t _spake2p_iterations = 0;
    char _spake2p_salt[kSpake2pSaltB64LengthMax] = { 0 };
    size_t _spake2p_salt_size = 0;
    char _spake2p_verifier[kSpake2pVerifierB64LengthMax] = { 0 };
    size_t _spake2p_verifier_size = 0;
    uint32_t _passcode = 0;
    uint32_t _commissioning_flow = 0;
    uint32_t _rendezvous_flags = 0;
    uint8_t _setup_payload[kTotalPayloadDataSize] = { 0 };
};


} // namespace Provision
} // namespace Silabs
} // namespace DeviceLayer
} // namespace chip
