#include "ProvisionProtocol.h"
#include "ProvisionEncoder.h"
#include <lib/support/CodeUtils.h>
#include <lib/core/CHIPEncoding.h>
#include <lib/support/BytesToHex.h>
#include <lib/support/CodeUtils.h>
#include <app/data-model/Nullable.h>
#include <crypto/CHIPCryptoPAL.h>
#include <string.h>
#include <em_msc.h>

namespace chip {
namespace DeviceLayer {
namespace Silabs {
namespace Provision {

using namespace chip::app::DataModel;
using namespace Encoding::Version2;

//------------------------------------------------------------------------------
// Feedback List
//------------------------------------------------------------------------------

struct ReadEntry
{
    uint16_t id;
    uint8_t type;
    bool is_known;
};

struct ReadList
{
    static constexpr size_t kLimit = 64;

    void Clear() { this->count = 0; }
    size_t Count() { return this->count; }

    CHIP_ERROR Add(uint16_t id, uint8_t type)
    {
        VerifyOrReturnError(this->count < kLimit, CHIP_ERROR_BUFFER_TOO_SMALL);
        if(! this->Contains(id))
        {
            size_t index = (this->first + this->count) % kLimit;
            ReadEntry &e = this->entries[index];
            e.id = id;
            e.type = type;
            e.is_known = (id & Protocol2::kWellKnownMask);
            this->count++;
        }
        return CHIP_NO_ERROR;
    }

    CHIP_ERROR Remove(ReadEntry &out)
    {
        VerifyOrReturnError(this->count > 0, CHIP_ERROR_NOT_FOUND);
        out = this->entries[this->first];
        this->first = (this->first + 1) % kLimit;
        this->count--;
        return CHIP_NO_ERROR;
    }

    bool Contains(uint16_t id)
    {
        size_t count = 0;
        while(count < this->count)
        {
            size_t index = (this->first + count) % kLimit;
            if(this->entries[index].id == id)
            {
                return true;
            }
            count++;
        }
        return false;
    }
private:
    ReadEntry entries[kLimit];
    size_t first = 0;
    size_t count = 0;
};


//------------------------------------------------------------------------------
// Commands
//------------------------------------------------------------------------------

namespace {
uint8_t rx_buffer[Protocol2::kRequestHeaderSize + Storage::kArgumentSizeMax];
uint8_t tx_buffer[Protocol2::kResponseHeaderSize + Storage::kArgumentSizeMax];
Argument _incoming(rx_buffer, sizeof(rx_buffer));
Argument _outgoing(tx_buffer, sizeof(tx_buffer));
ReadList _feedback_list;
} // namespace

enum CommandIds: uint8_t
{
    kCommand_Init   = 1,
    kCommand_Finish = 2,
    kCommand_Write  = 3,
    kCommand_Read   = 4,
    kCommand_CSR    = 5,
};

struct Command
{
    Command(uint8_t id, Storage & store): mId(id), mStore(store)  {}
    virtual ~Command() = default;

    CHIP_ERROR Execute(Encoding::Buffer & in, Encoding::Buffer & out)
    {
        // Get payload size
        uint8_t payload_size = 0;
        ReturnErrorOnFailure(in.Get(payload_size));
        VerifyOrReturnError(payload_size == in.Left(), CHIP_ERROR_INVALID_MESSAGE_LENGTH);

        // Decode incoming payload
        ReturnErrorOnFailure(this->DecodePayload(in));

        // Encode outgoing payload
        ReturnErrorOnFailure(this->EncodePayload(out));

        // Success
        return CHIP_NO_ERROR;
    }

    virtual CHIP_ERROR OnPayloadDecoded()
    {
        return CHIP_NO_ERROR;
    }

    virtual CHIP_ERROR OnPayloadEncoded()
    {
        return CHIP_NO_ERROR;
    }

    virtual CHIP_ERROR DecodePayload(Encoding::Buffer & in)
    {
        while(in.Left() > 0)
        {
            CHIP_ERROR err = Decode(in, _incoming);
            VerifyOrReturnError((CHIP_ERROR_MESSAGE_INCOMPLETE == err) || (CHIP_NO_ERROR == err), err);
            if(CHIP_NO_ERROR == err)
            {
                ReturnErrorOnFailure(ProcessIncoming(_incoming));
                _incoming.Reset();
            }
        }

        return OnPayloadDecoded();
    }

    virtual CHIP_ERROR EncodePayload(Encoding::Buffer & out)
    {
        CHIP_ERROR err = CHIP_NO_ERROR;

        // Add pending fragment
        if((out.Spare() > 0) && (_outgoing.Left() > 0))
        {
            ReturnErrorOnFailure(out.Add(_outgoing, std::min(out.Spare(), _outgoing.Left())));
        }

        // Add as many outgoing arguments as possible
        while((out.Spare() > 0) && (_feedback_list.Count() > 0))
        {
            ReadEntry arg;
            _outgoing.Clear();
            err = _feedback_list.Remove(arg);
            ReturnErrorCodeIf(CHIP_ERROR_NOT_FOUND == err, CHIP_NO_ERROR);
            ReturnErrorOnFailure(this->EncodeOutgoing(arg, _outgoing));
            ReturnErrorOnFailure(out.Add(_outgoing, std::min(out.Spare(), _outgoing.Left())));
        }

        return OnPayloadEncoded();
    }

    virtual CHIP_ERROR ProcessIncoming(Argument &arg)
    {
        return CHIP_NO_ERROR;
    }

    virtual CHIP_ERROR EncodeOutgoing(ReadEntry &arg, Argument &out)
    {
        GenericStorage &store = (arg.is_known ? (GenericStorage &)mStore : (GenericStorage &)mStore.mCustom);
        CHIP_ERROR err = CHIP_NO_ERROR;

        switch(arg.type)
        {
        case Type_Int8u:
        {
            uint8_t value = 0;
            err = store.Get(arg.id, value);
            VerifyOrReturnError((CHIP_ERROR_NOT_FOUND == err) || (CHIP_NO_ERROR == err), err);
            uint8_t *x = (CHIP_ERROR_NOT_FOUND == err) ? nullptr : &value;
            ReturnErrorOnFailure(Encode(arg.id, x, out));
            break;
        }
        case Type_Int16u:
        {
            uint16_t value = 0;
            err = store.Get(arg.id, value);
            VerifyOrReturnError((CHIP_ERROR_NOT_FOUND == err) || (CHIP_NO_ERROR == err), err);
            uint16_t *x = (CHIP_ERROR_NOT_FOUND == err) ? nullptr : &value;
            ReturnErrorOnFailure(Encode(arg.id, x, out));
            break;
        }
        case Type_Int32u:
        {
            uint32_t value = 0;
            err = store.Get(arg.id, value);
            VerifyOrReturnError((CHIP_ERROR_NOT_FOUND == err) || (CHIP_NO_ERROR == err), err);
            uint32_t *x = (CHIP_ERROR_NOT_FOUND == err) ? nullptr : &value;
            ReturnErrorOnFailure(Encode(arg.id, x, out));
            break;
        }
        case Type_Binary:
        {
            uint8_t *value = Storage::aux_buffer;
            size_t size = 0;
            err = store.Get(arg.id, value, Storage::kArgumentSizeMax, size);
            VerifyOrReturnError((CHIP_ERROR_NOT_FOUND == err) || (CHIP_NO_ERROR == err), err);
            uint8_t *x = (CHIP_ERROR_NOT_FOUND == err) ? nullptr : value;
            ReturnErrorOnFailure(Encode(arg.id, x, size, out));
            break;
        }
        default:
            return CHIP_ERROR_WRONG_TLV_TYPE;
        }

        return CHIP_NO_ERROR;
    }

    uint8_t mId;
    Storage & mStore;
};

struct InitCommand: public Command
{
    InitCommand(Storage & store): Command(kCommand_Init, store)
    {
        _incoming.Reset();
        _outgoing.Clear();
        _feedback_list.Clear();
    }

    CHIP_ERROR ProcessIncoming(Argument &arg) override
    {
        switch(arg.id)
        {
        case Parameters::kFlashAddress:
            mFlashAddress = arg.value.u32;
            break;
        case Parameters::kFlashSize:
            mFlashSize = arg.value.u32;
            break;
        case Parameters::kFlashPageSize:
        case Parameters::kBaseAddress:
            ReturnErrorOnFailure(_feedback_list.Add(arg.id, arg.type));
            break;
        default:
            return CHIP_ERROR_UNKNOWN_RESOURCE_ID;
        }
        return CHIP_NO_ERROR;
    }

    CHIP_ERROR EncodeOutgoing(ReadEntry &arg, Argument &out) override
    {
        switch(arg.id)
        {
        case Parameters::kFlashPageSize:
        {
            uint32_t page_size = FLASH_PAGE_SIZE;
            ReturnErrorOnFailure(Encode(arg.id, &page_size, out));
            break;
        }
        case Parameters::kBaseAddress:
        {
            uint32_t creds_base_addr = 0;
            ReturnErrorOnFailure(mStore.GetBaseAddress(creds_base_addr));
            ReturnErrorOnFailure(Encode(arg.id, &creds_base_addr, out));
            break;
        }
        default:
            return CHIP_ERROR_UNKNOWN_RESOURCE_ID;
        }
        return CHIP_NO_ERROR;
    }

    CHIP_ERROR OnPayloadDecoded() override
    {
        return mStore.Initialize(mFlashAddress, mFlashSize);
    }

private:
    uint32_t mFlashAddress = 0;
    uint32_t mFlashSize = 0;
};

struct CsrCommand: public Command
{
    CsrCommand(Storage & store): Command(kCommand_Init, store)
    {
        // Always return CSR file
        _feedback_list.Add(Parameters::kCsrFile, Type_Binary);
    }

    CHIP_ERROR ProcessIncoming(Argument &arg) override
    {
        switch(arg.id)
        {
        case Parameters::kVendorId:
        case Parameters::kProductId:
            return mStore.Set(arg.id, &arg.value.u16);

        case Parameters::kCommonName:
            return mStore.Set(arg.id, arg.value.b, arg.size);

        default:
            return CHIP_ERROR_UNKNOWN_RESOURCE_ID;
        }
    }
};

struct WriteCommand: public Command
{
    WriteCommand(Storage & store): Command(kCommand_Write, store) {}

    CHIP_ERROR ProcessIncoming(Argument &arg) override
    {
        GenericStorage &store = (arg.is_known ? (GenericStorage &)mStore : (GenericStorage &)mStore.mCustom);

        if(arg.feedback)
        {
            // Return only flagged arguments
            ReturnErrorOnFailure(_feedback_list.Add(arg.id, arg.type));
        }

        switch(arg.type)
        {
        case Type_Int8u:
            return store.Set(arg.id, arg.is_null ? nullptr : &arg.value.u8);

        case Type_Int16u:
            return store.Set(arg.id, arg.is_null ? nullptr : &arg.value.u16);

        case Type_Int32u:
            return store.Set(arg.id, arg.is_null ? nullptr : &arg.value.u32);

        case Type_Binary:
        {
            return store.Set(arg.id, arg.is_null ? nullptr : arg.value.b, arg.size);
        }

        default:
            return CHIP_ERROR_WRONG_TLV_TYPE;
        }

    }

    CHIP_ERROR OnPayloadDecoded() override
    {
        return this->mStore.Commit();
    }

};


struct ReadCommand: public Command
{
    ReadCommand(Storage & store): Command(kCommand_Read, store) {}

    CHIP_ERROR ProcessIncoming(Argument &arg) override
    {
        // Return all arguments
        return _feedback_list.Add(arg.id, arg.type);
    }

};

//------------------------------------------------------------------------------
// Protocol
//------------------------------------------------------------------------------

bool Protocol2::Execute(ByteSpan & req, MutableByteSpan & res)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    uint8_t version = 0;
    uint8_t command_id = 0;
    uint8_t counter = 0;
    uint8_t payload_size = 0;

    VerifyOrExit(req.size() >= (kRequestHeaderSize + kChecksumSize), err = CHIP_ERROR_BUFFER_TOO_SMALL);
    {
        Encoding::Buffer in((uint8_t*)req.data(), req.size() - kChecksumSize, true);
        SuccessOrExit(err = in.Get(version));
        SuccessOrExit(err = in.Get(command_id));
        SuccessOrExit(err = in.Get(counter));

        VerifyOrExit(req.size() <= kPackageSizeMax, err = CHIP_ERROR_INBOUND_MESSAGE_TOO_BIG);
        VerifyOrExit(res.size() >= kPackageSizeMax, err = CHIP_ERROR_BUFFER_TOO_SMALL);
        {
            Encoding::Buffer payload(res.data() + kResponseHeaderSize, kPackageSizeMax - kResponseHeaderSize - kChecksumSize);

            switch(command_id)
            {
            case kCommand_Init:
            {
                InitCommand cmd(mStore);
                err = cmd.Execute(in, payload);
                break;
            }
            case kCommand_Finish:
                break;

            case kCommand_CSR:
            {
                CsrCommand cmd(mStore);
                err = cmd.Execute(in, payload);
                break;
            }
            case kCommand_Write:
            {
                WriteCommand cmd(mStore);
                err = cmd.Execute(in, payload);
                break;
            }
            case kCommand_Read:
            {
                ReadCommand cmd(mStore);
                err = cmd.Execute(in, payload);
                break;
            }
            default:
                err = CHIP_ERROR_INVALID_MESSAGE_TYPE;
            }
            payload_size = payload.Size();
        }
    }

exit:
    Encoding::Buffer header(res.data(), kResponseHeaderSize);
    uint32_t err_code = static_cast<uint32_t>(err.AsInteger());
    uint8_t checksum[32] = { 0 };

    header.Add(Protocol2::kVersion);
    header.Add(static_cast<uint8_t>(command_id | Protocol2::kResponseFlag));
    header.Add(counter);
    header.Add(err_code);
    header.Add(payload_size);
    res.reduce_size(kResponseHeaderSize + payload_size + kChecksumSize);
    // Checksum
    chip::Crypto::Hash_SHA256(res.data(), kResponseHeaderSize + payload_size, checksum);
    memcpy(res.data() + kResponseHeaderSize + payload_size, checksum, kChecksumSize);
    return kCommand_Finish != command_id;
}


} // namespace Provision
} // namespace Silabs
} // namespace DeviceLayer
} // namespace chip
