/*
 *
 *    Copyright (c) 2026 Project CHIP Authors
 *
 *    Licensed under the Apache License, Version 2.0 (the "License");
 *    you may not use this file except in compliance with the License.
 *    You may obtain a copy of the License at
 *
 *        http://www.apache.org/licenses/LICENSE-2.0
 *
 *    Unless required by applicable law or agreed to in writing, software
 *    distributed under the License is distributed on an "AS IS" BASIS,
 *    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *    See the License for the specific language governing permissions and
 *    limitations under the License.
 */

#include "SrpDnsMessage.h"

#include <cstring>

#include <lib/support/CodeUtils.h>

namespace chip {
namespace Dnssd {
namespace Srp {

namespace {

constexpr uint8_t kLabelMaxLength   = 63;
constexpr size_t kNameMaxWireLength = 255;
constexpr uint8_t kPointerTag       = 0xC0;

} // namespace

// ---------------------------------------------------------------------------
// DnsWriter
// ---------------------------------------------------------------------------

CHIP_ERROR DnsWriter::Require(size_t additional)
{
    VerifyOrReturnError(mOk, CHIP_ERROR_BUFFER_TOO_SMALL);
    if (mOffset + additional > mSize)
    {
        mOk = false;
        return CHIP_ERROR_BUFFER_TOO_SMALL;
    }
    return CHIP_NO_ERROR;
}

CHIP_ERROR DnsWriter::PutU16(uint16_t value)
{
    ReturnErrorOnFailure(Require(2));
    mBuffer[mOffset++] = static_cast<uint8_t>(value >> 8);
    mBuffer[mOffset++] = static_cast<uint8_t>(value & 0xFF);
    return CHIP_NO_ERROR;
}

CHIP_ERROR DnsWriter::PutU32(uint32_t value)
{
    ReturnErrorOnFailure(Require(4));
    mBuffer[mOffset++] = static_cast<uint8_t>((value >> 24) & 0xFF);
    mBuffer[mOffset++] = static_cast<uint8_t>((value >> 16) & 0xFF);
    mBuffer[mOffset++] = static_cast<uint8_t>((value >> 8) & 0xFF);
    mBuffer[mOffset++] = static_cast<uint8_t>(value & 0xFF);
    return CHIP_NO_ERROR;
}

CHIP_ERROR DnsWriter::PutBytes(const uint8_t * data, size_t len)
{
    if (len == 0)
    {
        return CHIP_NO_ERROR;
    }
    ReturnErrorOnFailure(Require(len));
    memcpy(mBuffer + mOffset, data, len);
    mOffset += len;
    return CHIP_NO_ERROR;
}

CHIP_ERROR DnsWriter::OverwriteU16(size_t offset, uint16_t value)
{
    VerifyOrReturnError(offset + 2 <= mSize, CHIP_ERROR_BUFFER_TOO_SMALL);
    mBuffer[offset]     = static_cast<uint8_t>(value >> 8);
    mBuffer[offset + 1] = static_cast<uint8_t>(value & 0xFF);
    return CHIP_NO_ERROR;
}

CHIP_ERROR DnsWriter::PutHeader(uint16_t id, uint16_t flags, uint16_t qdcount, uint16_t ancount, uint16_t nscount, uint16_t arcount)
{
    ReturnErrorOnFailure(PutU16(id));
    ReturnErrorOnFailure(PutU16(flags));
    ReturnErrorOnFailure(PutU16(qdcount));
    ReturnErrorOnFailure(PutU16(ancount));
    ReturnErrorOnFailure(PutU16(nscount));
    ReturnErrorOnFailure(PutU16(arcount));
    return CHIP_NO_ERROR;
}

void DnsWriter::PatchHeaderCounts(uint16_t qdcount, uint16_t ancount, uint16_t nscount, uint16_t arcount)
{
    constexpr size_t kDnsHeaderQdcountOffset = 4;
    constexpr size_t kDnsHeaderAncountOffset = 6;
    constexpr size_t kDnsHeaderNscountOffset = 8;
    constexpr size_t kDnsHeaderArcountOffset = 10;

    mOk = false; // In case the overwrite fails, we prehenteve set the Writer state to not ok.
    ReturnErrorOnFailure(OverwriteU16(kDnsHeaderQdcountOffset, qdcount));
    ReturnErrorOnFailure(OverwriteU16(kDnsHeaderAncountOffset, ancount));
    ReturnErrorOnFailure(OverwriteU16(kDnsHeaderNscountOffset, nscount));
    ReturnErrorOnFailure(OverwriteU16(kDnsHeaderArcountOffset, arcount));
    mOk = true;
}

CHIP_ERROR DnsWriter::PutName(const char * name)
{
    VerifyOrReturnError(name != nullptr, CHIP_ERROR_INVALID_ARGUMENT);

    size_t startOffset = mOffset;

    // Empty name or "." is the DNS root: a single zero octet.
    const char * cursor = name;
    while (*cursor != '\0')
    {
        const char * dot = strchr(cursor, '.');
        size_t labelLen  = (dot != nullptr) ? static_cast<size_t>(dot - cursor) : strlen(cursor);

        // A trailing dot terminates the name; skip a zero-length final label.
        if (labelLen == 0)
        {
            VerifyOrReturnError(dot != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
            cursor = dot + 1;
            continue;
        }

        VerifyOrReturnError(labelLen <= kLabelMaxLength, CHIP_ERROR_INVALID_ARGUMENT);
        ReturnErrorOnFailure(Require(1 + labelLen));
        mBuffer[mOffset++] = static_cast<uint8_t>(labelLen);
        memcpy(mBuffer + mOffset, cursor, labelLen);
        mOffset += labelLen;

        if (dot == nullptr)
        {
            break;
        }
        cursor = dot + 1;
    }

    // Root terminator.
    ReturnErrorOnFailure(Require(1));
    mBuffer[mOffset++] = 0;

    VerifyOrReturnError(mOffset - startOffset <= kNameMaxWireLength, CHIP_ERROR_INVALID_ARGUMENT);
    return CHIP_NO_ERROR;
}

CHIP_ERROR DnsWriter::PutQuestion(const char * name, RecordType type, RecordClass recordClass)
{
    ReturnErrorOnFailure(PutName(name));
    ReturnErrorOnFailure(PutU16(static_cast<uint16_t>(type)));
    ReturnErrorOnFailure(PutU16(static_cast<uint16_t>(recordClass)));
    return CHIP_NO_ERROR;
}

CHIP_ERROR DnsWriter::StartRecord(const char * name, RecordType type, RecordClass recordClass, uint32_t ttl)
{
    VerifyOrReturnError(!mInRecord, CHIP_ERROR_INCORRECT_STATE);
    ReturnErrorOnFailure(PutName(name));
    ReturnErrorOnFailure(PutU16(static_cast<uint16_t>(type)));
    ReturnErrorOnFailure(PutU16(static_cast<uint16_t>(recordClass)));
    ReturnErrorOnFailure(PutU32(ttl));
    mRdlenOffset = mOffset;
    ReturnErrorOnFailure(PutU16(0)); // RDLENGTH placeholder
    mInRecord = true;
    return CHIP_NO_ERROR;
}

CHIP_ERROR DnsWriter::FinishRecord()
{
    VerifyOrReturnError(mInRecord, CHIP_ERROR_INCORRECT_STATE);
    VerifyOrReturnError(mOk, CHIP_ERROR_BUFFER_TOO_SMALL);
    size_t rdlen = mOffset - (mRdlenOffset + 2);
    VerifyOrReturnError(rdlen <= UINT16_MAX, CHIP_ERROR_INVALID_ARGUMENT);
    ReturnErrorOnFailure(OverwriteU16(mRdlenOffset, static_cast<uint16_t>(rdlen)));
    mInRecord = false;
    return CHIP_NO_ERROR;
}

CHIP_ERROR DnsWriter::PutSrv(const char * name, RecordClass recordClass, uint32_t ttl, uint16_t priority, uint16_t weight,
                             uint16_t port, const char * target)
{
    ReturnErrorOnFailure(StartRecord(name, RecordType::kSrv, recordClass, ttl));
    ReturnErrorOnFailure(PutU16(priority));
    ReturnErrorOnFailure(PutU16(weight));
    ReturnErrorOnFailure(PutU16(port));
    ReturnErrorOnFailure(PutName(target));
    return FinishRecord();
}

CHIP_ERROR DnsWriter::PutTxt(const char * name, RecordClass recordClass, uint32_t ttl, const TextEntry * entries, size_t count)
{
    ReturnErrorOnFailure(StartRecord(name, RecordType::kTxt, recordClass, ttl));

    if (count == 0)
    {
        // A TXT record must carry at least one (empty) character-string.
        ReturnErrorOnFailure(Require(1));
        mBuffer[mOffset++] = 0;
        return FinishRecord();
    }

    for (size_t i = 0; i < count; i++)
    {
        const TextEntry & entry = entries[i];
        size_t keyLen           = (entry.mKey != nullptr) ? strlen(entry.mKey) : 0;
        size_t valueLen         = entry.mData != nullptr ? entry.mDataSize : 0;
        // "key" or "key=value"
        size_t stringLen = keyLen + (entry.mData != nullptr ? (1 + valueLen) : 0);
        VerifyOrReturnError(stringLen <= UINT8_MAX, CHIP_ERROR_INVALID_ARGUMENT);

        ReturnErrorOnFailure(Require(1 + stringLen));
        mBuffer[mOffset++] = static_cast<uint8_t>(stringLen);
        memcpy(mBuffer + mOffset, entry.mKey, keyLen);
        mOffset += keyLen;
        if (entry.mData != nullptr)
        {
            mBuffer[mOffset++] = '=';
            memcpy(mBuffer + mOffset, entry.mData, valueLen);
            mOffset += valueLen;
        }
    }
    return FinishRecord();
}

CHIP_ERROR DnsWriter::PutAaaa(const char * name, RecordClass recordClass, uint32_t ttl, const Inet::IPAddress & address)
{
    ReturnErrorOnFailure(StartRecord(name, RecordType::kAaaa, recordClass, ttl));
    // Addr is stored in network byte order; write the 16 bytes raw.
    ReturnErrorOnFailure(PutBytes(reinterpret_cast<const uint8_t *>(address.Addr), 16));
    return FinishRecord();
}

CHIP_ERROR DnsWriter::PutPtr(const char * name, RecordClass recordClass, uint32_t ttl, const char * target)
{
    ReturnErrorOnFailure(StartRecord(name, RecordType::kPtr, recordClass, ttl));
    ReturnErrorOnFailure(PutName(target));
    return FinishRecord();
}

CHIP_ERROR DnsWriter::PutKey(const char * name, RecordClass recordClass, uint32_t ttl, ByteSpan publicKey)
{
    // RFC 9665 uses a KEY record whose RDATA is: flags(2) protocol(1) algorithm(1)
    // then the public key. algorithm 13 == ECDSA P-256 / SHA-256.
    ReturnErrorOnFailure(StartRecord(name, RecordType::kKey, recordClass, ttl));
    ReturnErrorOnFailure(PutU16(kKeyRdataFlags));
    ReturnErrorOnFailure(Require(2));
    mBuffer[mOffset++] = kKeyRdataProtocol;
    mBuffer[mOffset++] = kKeyAlgorithmEcdsaP256Sha256;
    ReturnErrorOnFailure(PutBytes(publicKey.data(), publicKey.size()));
    return FinishRecord();
}

CHIP_ERROR DnsWriter::PutDeleteRRset(const char * name, RecordType type)
{
    ReturnErrorOnFailure(StartRecord(name, type, RecordClass::kAny, 0));
    return FinishRecord(); // empty RDATA
}

// ---------------------------------------------------------------------------
// DnsReader
// ---------------------------------------------------------------------------

CHIP_ERROR DnsReader::ReadU16(uint16_t & value)
{
    VerifyOrReturnError(mOffset + 2 <= mSize, CHIP_ERROR_BUFFER_TOO_SMALL);
    value = static_cast<uint16_t>((static_cast<uint16_t>(mBuffer[mOffset]) << 8) | mBuffer[mOffset + 1]);
    mOffset += 2;
    return CHIP_NO_ERROR;
}

CHIP_ERROR DnsReader::ReadU32(uint32_t & value)
{
    VerifyOrReturnError(mOffset + 4 <= mSize, CHIP_ERROR_BUFFER_TOO_SMALL);
    value = (static_cast<uint32_t>(mBuffer[mOffset]) << 24) | (static_cast<uint32_t>(mBuffer[mOffset + 1]) << 16) |
        (static_cast<uint32_t>(mBuffer[mOffset + 2]) << 8) | static_cast<uint32_t>(mBuffer[mOffset + 3]);
    mOffset += 4;
    return CHIP_NO_ERROR;
}

CHIP_ERROR DnsReader::ReadBytes(uint8_t * out, size_t len)
{
    VerifyOrReturnError(mOffset + len <= mSize, CHIP_ERROR_BUFFER_TOO_SMALL);
    memcpy(out, mBuffer + mOffset, len);
    mOffset += len;
    return CHIP_NO_ERROR;
}

CHIP_ERROR DnsReader::ReadHeader(Header & out)
{
    ReturnErrorOnFailure(ReadU16(out.id));
    ReturnErrorOnFailure(ReadU16(out.flags));
    ReturnErrorOnFailure(ReadU16(out.qdcount));
    ReturnErrorOnFailure(ReadU16(out.ancount));
    ReturnErrorOnFailure(ReadU16(out.nscount));
    ReturnErrorOnFailure(ReadU16(out.arcount));
    return CHIP_NO_ERROR;
}

CHIP_ERROR DnsReader::ReadNameAt(size_t offset, char * out, size_t outSize, size_t & bytesConsumedAtStart)
{
    VerifyOrReturnError(out != nullptr && outSize > 0, CHIP_ERROR_INVALID_ARGUMENT);

    size_t pos             = offset;
    size_t outPos          = 0;
    bool jumped            = false;
    size_t consumedAtStart = 0;
    int safety             = 0;

    while (true)
    {
        VerifyOrReturnError(++safety < 256, CHIP_ERROR_INVALID_ARGUMENT);
        VerifyOrReturnError(pos < mSize, CHIP_ERROR_BUFFER_TOO_SMALL);

        uint8_t len = mBuffer[pos];
        if ((len & kPointerTag) == kPointerTag)
        {
            VerifyOrReturnError(pos + 2 <= mSize, CHIP_ERROR_BUFFER_TOO_SMALL);
            uint16_t pointer = static_cast<uint16_t>(((len & 0x3F) << 8) | mBuffer[pos + 1]);
            if (!jumped)
            {
                consumedAtStart = (pos + 2) - offset;
                jumped          = true;
            }
            VerifyOrReturnError(pointer < mSize, CHIP_ERROR_BUFFER_TOO_SMALL);
            VerifyOrReturnError(pointer < pos, CHIP_ERROR_INVALID_ARGUMENT); // pointers must go backwards
            pos = pointer;
            continue;
        }

        VerifyOrReturnError((len & kPointerTag) == 0, CHIP_ERROR_INVALID_ARGUMENT);

        if (len == 0)
        {
            if (!jumped)
            {
                consumedAtStart = (pos + 1) - offset;
            }
            break;
        }

        VerifyOrReturnError(pos + 1 + len <= mSize, CHIP_ERROR_BUFFER_TOO_SMALL);
        if (outPos != 0)
        {
            VerifyOrReturnError(outPos + 1 < outSize, CHIP_ERROR_BUFFER_TOO_SMALL);
            out[outPos++] = '.';
        }
        VerifyOrReturnError(outPos + len < outSize, CHIP_ERROR_BUFFER_TOO_SMALL);
        memcpy(out + outPos, mBuffer + pos + 1, len);
        outPos += len;
        pos += 1 + len;
    }

    out[outPos]          = '\0';
    bytesConsumedAtStart = consumedAtStart;
    return CHIP_NO_ERROR;
}

CHIP_ERROR DnsReader::ReadName(char * out, size_t outSize)
{
    size_t consumed = 0;
    ReturnErrorOnFailure(ReadNameAt(mOffset, out, outSize, consumed));
    mOffset += consumed;
    return CHIP_NO_ERROR;
}

CHIP_ERROR DnsReader::SkipQuestion()
{
    char name[kMaxDottedNameSize];
    ReturnErrorOnFailure(ReadName(name, sizeof(name)));
    uint16_t type, recordClass;
    ReturnErrorOnFailure(ReadU16(type));
    ReturnErrorOnFailure(ReadU16(recordClass));
    return CHIP_NO_ERROR;
}

CHIP_ERROR DnsReader::ReadRecordHeader(RecordHeader & out)
{
    ReturnErrorOnFailure(ReadName(out.name, sizeof(out.name)));
    uint16_t type, recordClass;
    ReturnErrorOnFailure(ReadU16(type));
    ReturnErrorOnFailure(ReadU16(recordClass));
    ReturnErrorOnFailure(ReadU32(out.ttl));
    ReturnErrorOnFailure(ReadU16(out.rdlength));
    out.type        = static_cast<RecordType>(type);
    out.recordClass = static_cast<RecordClass>(recordClass);
    out.rdataOffset = mOffset;
    VerifyOrReturnError(mOffset + out.rdlength <= mSize, CHIP_ERROR_BUFFER_TOO_SMALL);
    return CHIP_NO_ERROR;
}

CHIP_ERROR DnsReader::SkipRecordData(const RecordHeader & header)
{
    VerifyOrReturnError(header.rdataOffset + header.rdlength <= mSize, CHIP_ERROR_BUFFER_TOO_SMALL);
    mOffset = header.rdataOffset + header.rdlength;
    return CHIP_NO_ERROR;
}

CHIP_ERROR DnsReader::ReadSrv(uint16_t & priority, uint16_t & weight, uint16_t & port, char * target, size_t targetSize)
{
    ReturnErrorOnFailure(ReadU16(priority));
    ReturnErrorOnFailure(ReadU16(weight));
    ReturnErrorOnFailure(ReadU16(port));
    return ReadName(target, targetSize);
}

CHIP_ERROR DnsReader::ReadAaaa(Inet::IPAddress & address)
{
    uint8_t bytes[16];
    ReturnErrorOnFailure(ReadBytes(bytes, sizeof(bytes)));
    memcpy(address.Addr, bytes, sizeof(bytes));
    return CHIP_NO_ERROR;
}

CHIP_ERROR DnsReader::ReadPtr(char * target, size_t targetSize)
{
    return ReadName(target, targetSize);
}

CHIP_ERROR DnsReader::ReadKey(uint16_t rdlength, uint8_t * out, size_t outSize, size_t & outLen)
{
    // RDATA layout: flags(2) protocol(1) algorithm(1) followed by the public key.
    constexpr uint16_t kFixedPrefix = 4;
    VerifyOrReturnError(rdlength >= kFixedPrefix, CHIP_ERROR_INVALID_ARGUMENT);
    size_t keyLen = static_cast<size_t>(rdlength - kFixedPrefix);
    VerifyOrReturnError(keyLen <= outSize, CHIP_ERROR_BUFFER_TOO_SMALL);

    uint16_t flags;
    ReturnErrorOnFailure(ReadU16(flags));
    VerifyOrReturnError(mOffset + 2 <= mSize, CHIP_ERROR_BUFFER_TOO_SMALL);
    mOffset += 2; // protocol + algorithm
    ReturnErrorOnFailure(ReadBytes(out, keyLen));
    outLen = keyLen;
    return CHIP_NO_ERROR;
}

} // namespace Srp
} // namespace Dnssd
} // namespace chip
