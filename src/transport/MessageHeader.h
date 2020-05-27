#ifndef MESSAGEHEADER_H_
#define MESSAGEHEADER_H_

#include <cstddef>
#include <cstdint>

#include <core/CHIPError.h>
#include <core/Optional.h>

namespace chip {

/** Handles encoding/decoding of CHIP message headers */
class MessageHeader
{
public:
    const Optional<uint64_t> & GetSourceNodeId() const { return mSourceNodeId; }
    const Optional<uint64_t> & GetDestinationNodeId() const { return mDestinationNodeId; }
    uint32_t GetMessageId() const { return mMessageId; }

    MessageHeader & SetMessageId(uint32_t id)
    {
        mMessageId = id;
        return *this;
    }

    MessageHeader & SetSourceNodeId(uint64_t id)
    {
        mSourceNodeId.SetValue(id);
        return *this;
    }

    MessageHeader & ClearSourceNodeId()
    {
        mSourceNodeId.ClearValue();
        return *this;
    }

    MessageHeader & SetDestinationNodeId(uint64_t id)
    {
        mDestinationNodeId.SetValue(id);
        return *this;
    }

    MessageHeader & ClearDestinationNodeId()
    {
        mDestinationNodeId.ClearValue();
        return *this;
    }

    size_t EncodeSizeBytes() const;
    CHIP_ERROR Decode(const uint8_t * data, size_t size);
    CHIP_ERROR Encode(uint8_t * data, size_t size, size_t * encode_size);

private:
    static constexpr int kHeaderVersion = 2;

    uint32_t mMessageId = 0;

    Optional<uint64_t> mSourceNodeId;
    Optional<uint64_t> mDestinationNodeId;
};

} // namespace chip

#endif // MESSAGEHEADER_H_
