#pragma once

#include "encoding.h"
#include "config.h"

enum CommandIds
{
    kCommand_None = 0,
    kCommand_Debug = 1,
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


class Command
{
public:
    virtual ~Command() {}
    virtual int decode(Encoder & in) = 0;
    virtual int execute() = 0;
    virtual void encode(Encoder & out) const = 0;
};

class VoidCommand : public Command
{
public:
    int decode(Encoder & in) override;
    int execute() override;
    void encode(Encoder & out) const override;
};

class DebugCommand : public Command
{
public:
    int decode(Encoder & in) override;
    int execute() override;
    void encode(Encoder & out) const override;

private:
    uint32_t _key_id = 0;
};


class CsrCommand : public Command
{
public:
    static constexpr size_t kCsrLengthMax = 1024;
    static constexpr size_t kCommonNameMax = 128;

    int decode(Encoder & in) override;
    int execute() override;
    void encode(Encoder & out) const override;

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

    int decode(Encoder & in) override;
    int execute() override;
    void encode(Encoder & out) const override;

private:
    int writeKey(const uint8_t *data, size_t size);
    int writeDAC(const uint8_t *data, size_t size);
    int writePAI(const uint8_t *data, size_t size);
    int writeCD(const uint8_t *data, size_t size);
    int writeFile(Key offset_key, Key size_key, uint32_t offset, const uint8_t *data, size_t size);

    uint8_t _file_id = 0;
    uint32_t _key_id = 0;
    uint8_t _flash = 0;
    uint8_t _data[kFileSizeMax] = { 0 };
    uint32_t _address = 0;
    uint32_t _offset = 0;
    size_t _size = 0;
};


class SetupCommand : public Command
{
public:
    static constexpr size_t kVersionFieldLengthInBits = 3;
    static constexpr size_t kVendorIDFieldLengthInBits = 16;
    static constexpr size_t kProductIDFieldLengthInBits = 16;
    static constexpr size_t kCommissioningFlowFieldLengthInBits = 2;
    static constexpr size_t kRendezvousInfoFieldLengthInBits = 8;
    static constexpr size_t kPayloadDiscriminatorFieldLengthInBits = 12;
    static constexpr size_t kSetupPINCodeFieldLengthInBits = 27;
    static constexpr size_t kPaddingFieldLengthInBits = 4;
    static constexpr size_t kTotalPayloadDataSizeInBits = (
                                kVersionFieldLengthInBits + kVendorIDFieldLengthInBits +
                                kProductIDFieldLengthInBits + kCommissioningFlowFieldLengthInBits +
                                kRendezvousInfoFieldLengthInBits + kPayloadDiscriminatorFieldLengthInBits +
                                kSetupPINCodeFieldLengthInBits + kPaddingFieldLengthInBits);
    static constexpr size_t kTotalPayloadDataSize = kTotalPayloadDataSizeInBits / 8;


    int decode(Encoder & in) override;
    int execute() override;
    void encode(Encoder & out) const override;

    void generateQR(uint8_t bits[], uint32_t vendor_id, uint32_t product_id,
                uint32_t commissioning_flow, uint32_t rendezvous_flag,
                uint32_t discriminator, uint32_t passcode);
private:
    size_t writeQR(uint8_t bits[], size_t offset,
                uint32_t input, size_t numberOfBits, size_t totalPayloadSizeInBits);
    int computeHash(uint8_t hash[32], size_t & hash_size);

    uint32_t _vendor_id = 0;
    uint32_t _product_id = 0;
    uint16_t _discriminator = 0; // 12-bit
    uint32_t _passcode = 0; // 27-bit
    uint32_t _commissioning_flow = 0;
    uint32_t _rendezvous_flag = 0;
    uint8_t _unique_id[16] = { 0 };
    size_t _unique_size = 0;
    uint8_t _hash[32] = { 0 };
    size_t _hash_size = 0;
    uint8_t _setup_payload[kTotalPayloadDataSize] = { 0 };
};
