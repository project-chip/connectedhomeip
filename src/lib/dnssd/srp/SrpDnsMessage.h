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

/**
 * @file
 *   Self-contained DNS wire-format codec for the Service Registration Protocol
 *   (RFC 9665 / RFC 2136 DNS UPDATE) and unicast DNS queries/responses.
 *
 *   For now this is self-contained and does not depend on the existing minimal_mdns
 *   record model, so it can be built on any platform that enables a SRP
 *   client or server, independent of the selected mDNS backend. Generic DNS
 *   primitives (name compression, header, integer I/O) could be factored into
 *   a shared, backend-agnostic core and reused here in the future.
 */

#pragma once

#include <cstddef>
#include <cstdint>

#include <inet/IPAddress.h>
#include <lib/core/CHIPError.h>
#include <lib/dnssd/platform/Dnssd.h>
#include <lib/support/Span.h>

namespace chip {
namespace Dnssd {
namespace Srp {

// Maximum size, in bytes, of a fully-decoded dotted DNS name (including the
// trailing terminator). DNS names are at most 255 octets on the wire.
inline constexpr size_t kMaxDottedNameSize = 256;

// DNS message header size, in bytes.
inline constexpr size_t kDnsHeaderSize = 12;

// DNS record TYPE values used by SRP and Matter discovery.
enum class RecordType : uint16_t
{
    kA     = 1,
    kNs    = 2,
    kCname = 5,
    kSoa   = 6,
    kPtr   = 12,
    kTxt   = 16,
    kSig   = 24,
    kKey   = 25,
    kAaaa  = 28,
    kSrv   = 33,
    kOpt   = 41,
    kAny   = 255,
};

// DNS record CLASS values. SRP uses IN for additions and NONE/ANY for the
// RFC 2136 delete forms.
enum class RecordClass : uint16_t
{
    kIn   = 1,
    kNone = 254,
    kAny  = 255,
};

// DNS OPCODE values.
inline constexpr uint8_t kOpcodeQuery  = 0;
inline constexpr uint8_t kOpcodeUpdate = 5;

// DNS RCODE values (subset relevant to SRP).
enum class ResponseCode : uint8_t
{
    kNoError  = 0,
    kFormErr  = 1,
    kServFail = 2,
    kNxDomain = 3,
    kNotImp   = 4,
    kRefused  = 5,
    kYxDomain = 6,
    kYxRrset  = 7,
    kNxRrset  = 8,
    kNotAuth  = 9,
    kNotZone  = 10,
};

// QR bit (response) in the header flags word.
inline constexpr uint16_t kFlagQr = 0x8000;

// KEY / SIG record RDATA constants (RFC 4034 / RFC 6605).
// Algorithm 13 is ECDSA P-256 with SHA-256.
inline constexpr uint16_t kKeyRdataFlags              = 0x0200;
inline constexpr uint8_t kKeyRdataProtocol            = 3;
inline constexpr uint8_t kKeyAlgorithmEcdsaP256Sha256 = 13;

// Raw (X||Y) P-256 public key length carried in the KEY record for algorithm 13.
inline constexpr size_t kSrpPublicKeyRawSize = 64;

// Raw (r||s) P-256 signature length carried in the SIG(0) record.
inline constexpr size_t kSrpSignatureRawSize = 64;

// Build the header flags word from an opcode (and optional QR/response code).
constexpr uint16_t MakeFlags(uint8_t opcode, bool response = false, ResponseCode rcode = ResponseCode::kNoError)
{
    return static_cast<uint16_t>((response ? kFlagQr : 0) | (static_cast<uint16_t>(opcode & 0x0F) << 11) |
                                 (static_cast<uint16_t>(rcode) & 0x0F));
}

// Parsed DNS message header.
struct Header
{
    uint16_t id;
    uint16_t flags;
    uint16_t qdcount; // Zone count for UPDATE
    uint16_t ancount; // Prerequisite count for UPDATE
    uint16_t nscount; // Update count for UPDATE
    uint16_t arcount; // Additional count

    uint8_t Opcode() const { return static_cast<uint8_t>((flags >> 11) & 0x0F); }
    ResponseCode Rcode() const { return static_cast<ResponseCode>(flags & 0x0F); }
    bool IsResponse() const { return (flags & kFlagQr) != 0; }
};

/**
 * Incremental DNS message writer with RDLENGTH backpatching.
 *
 * The writer operates on a caller-provided buffer and tracks whether all writes
 * fit. Records are written with StartRecord()/FinishRecord() or via the typed
 * helpers. Header section counts are typically patched at the end via
 * PatchHeaderCounts().
 */
class DnsWriter
{
public:
    DnsWriter(uint8_t * buffer, size_t size) : mBuffer(buffer), mSize(size) {}

    /**
     * @brief Write the fixed 12-byte DNS header.
     * @param id       Transaction id.
     * @param flags    Flags word (see MakeFlags()).
     * @param qdcount  Question / zone entry count.
     * @param ancount  Answer / prerequisite count.
     * @param nscount  Authority / update count.
     * @param arcount  Additional record count.
     * @return CHIP_NO_ERROR, or CHIP_ERROR_BUFFER_TOO_SMALL if it does not fit.
     */
    CHIP_ERROR PutHeader(uint16_t id, uint16_t flags, uint16_t qdcount, uint16_t ancount, uint16_t nscount, uint16_t arcount);

    /**
     * @brief Overwrite the section counts in an already-written header.
     *
     * Useful when the final counts are only known after all records are
     * emitted. On failure the writer is marked not-Ok.
     * @return CHIP_NO_ERROR, or CHIP_ERROR_BUFFER_TOO_SMALL
     */
    CHIP_ERROR PatchHeaderCounts(uint16_t qdcount, uint16_t ancount, uint16_t nscount, uint16_t arcount);

    /**
     * @brief Encode a dotted DNS name (no compression).
     * @param name  e.g. "foo._matter._tcp.default.service.arpa". An empty
     *              string or "." encodes the DNS root (a single zero octet).
     * @return CHIP_ERROR_INVALID_ARGUMENT for a null name, a label longer than
     *         63 octets, or a name longer than 255 octets.
     */
    CHIP_ERROR PutName(const char * name);

    CHIP_ERROR PutU16(uint16_t value);
    CHIP_ERROR PutU32(uint32_t value);
    CHIP_ERROR PutBytes(const uint8_t * data, size_t len);

    // Write a question / zone entry: NAME, TYPE, CLASS.
    CHIP_ERROR PutQuestion(const char * name, RecordType type, RecordClass recordClass);

    /**
     * @brief Begin a resource record.
     *
     * Writes NAME, TYPE, CLASS, TTL and reserves the RDLENGTH field. Follow with
     * RDATA writes and then FinishRecord(). Records may not be nested.
     * @return CHIP_ERROR_INCORRECT_STATE if a record is already open.
     */
    CHIP_ERROR StartRecord(const char * name, RecordType type, RecordClass recordClass, uint32_t ttl);

    /**
     * @brief Backpatch the RDLENGTH of the record opened by StartRecord().
     * @return CHIP_ERROR_INCORRECT_STATE if no record is open.
     */
    CHIP_ERROR FinishRecord();

    // Typed record helpers (each is a full StartRecord()/rdata/FinishRecord()).
    CHIP_ERROR PutSrv(const char * name, RecordClass recordClass, uint32_t ttl, uint16_t priority, uint16_t weight, uint16_t port,
                      const char * target);
    CHIP_ERROR PutTxt(const char * name, RecordClass recordClass, uint32_t ttl, const TextEntry * entries, size_t count);
    CHIP_ERROR PutAaaa(const char * name, RecordClass recordClass, uint32_t ttl, const Inet::IPAddress & address);
    CHIP_ERROR PutPtr(const char * name, RecordClass recordClass, uint32_t ttl, const char * target);
    CHIP_ERROR PutKey(const char * name, RecordClass recordClass, uint32_t ttl, ByteSpan publicKey);

    // Write the "delete an RRset" form: CLASS ANY, TTL 0, empty RDATA. (RFC 2136)
    CHIP_ERROR PutDeleteRRset(const char * name, RecordType type);

    // True while every write so far has fit in the buffer.
    bool Ok() const { return mOk; }
    size_t Length() const { return mOffset; }
    const uint8_t * Data() const { return mBuffer; }

private:
    CHIP_ERROR Require(size_t additional);
    CHIP_ERROR OverwriteU16(size_t offset, uint16_t value);

    uint8_t * mBuffer;
    size_t mSize;
    size_t mOffset      = 0;
    size_t mRdlenOffset = 0; // offset of the RDLENGTH placeholder on the open record
    bool mInRecord      = false;
    bool mOk            = true;
};

/**
 * @brief Sequential DNS message reader supporting name decompression.
 *
 * The reader maintains a cursor (Offset()) into a caller-owned buffer and
 * advances it as fields are consumed. Every accessor is bounds-checked against
 * the buffer size and returns CHIP_ERROR_BUFFER_TOO_SMALL on truncation, so a
 * malformed or partial message can never read out of bounds.
 *
 * Typical usage: read the header, then walk each section by reading a record
 * header and either parsing its RDATA with a typed helper or skipping it.
 *
 * A typed RDATA parser must only be called immediately after
 * ReadRecordHeader() for a record of the matching type; it consumes RDATA from
 * the current cursor. Use SkipRecordData() for record types you do not handle.
 */
class DnsReader
{
public:
    // A parsed resource-record header. `rdataOffset` points just past RDLENGTH.
    struct RecordHeader
    {
        char name[kMaxDottedNameSize];
        RecordType type;
        RecordClass recordClass;
        uint32_t ttl;
        uint16_t rdlength;
        size_t rdataOffset;
    };

    DnsReader(const uint8_t * buffer, size_t size) : mBuffer(buffer), mSize(size) {}

    /**
     * @brief Read the fixed 12-byte header and advance the cursor past it.
     */
    CHIP_ERROR ReadHeader(Header & out);

    /**
     * @brief Decode a (possibly compressed) name at the current cursor.
     * @param out      Destination for the dotted, null-terminated name.
     * @param outSize  Size of @p out; should be at least kMaxDottedNameSize.
     *
     * The cursor advances only past the bytes physically present at the current
     * offset (a compression pointer counts as its 2 bytes), matching how names
     * are embedded in records. Forward or self-referential pointers are
     * rejected with CHIP_ERROR_INVALID_ARGUMENT.
     */
    CHIP_ERROR ReadName(char * out, size_t outSize);

    // Read helper functions. Advance the cursor past the field.
    CHIP_ERROR ReadU16(uint16_t & value);
    CHIP_ERROR ReadU32(uint32_t & value);
    CHIP_ERROR ReadBytes(uint8_t * out, size_t len);

    CHIP_ERROR SkipQuestion();

    CHIP_ERROR ReadRecordHeader(RecordHeader & out);

    // Advance past the RDATA of a record just read with ReadRecordHeader().
    CHIP_ERROR SkipRecordData(const RecordHeader & header);

    // Typed RDATA parsers. The current offset must be at the record's RDATA.
    CHIP_ERROR ReadSrv(uint16_t & priority, uint16_t & weight, uint16_t & port, char * target, size_t targetSize);
    CHIP_ERROR ReadAaaa(Inet::IPAddress & address);
    CHIP_ERROR ReadPtr(char * target, size_t targetSize);

    /**
     * @brief Parse a KEY record's RDATA (flags/protocol/algorithm + public key).
     * @param rdlength  The record's RDLENGTH, so the key material is bounded.
     * @param out       Destination for the raw public key.
     * @param outSize   Capacity of @p out.
     * @param outLen    Set to the number of key bytes written.
     * @return CHIP_ERROR_INVALID_ARGUMENT if @p rdlength is smaller than the
     *         fixed 4-byte prefix, or CHIP_ERROR_BUFFER_TOO_SMALL if the key
     *         does not fit in @p out.
     */
    CHIP_ERROR ReadKey(uint16_t rdlength, uint8_t * out, size_t outSize, size_t & outLen);

    size_t Offset() const { return mOffset; }
    void Seek(size_t offset) { mOffset = offset; }
    const uint8_t * Data() const { return mBuffer; }
    size_t Size() const { return mSize; }

private:
    CHIP_ERROR ReadNameAt(size_t offset, char * out, size_t outSize, size_t & bytesConsumedAtStart);

    const uint8_t * mBuffer;
    size_t mSize;
    size_t mOffset = 0;
};

} // namespace Srp
} // namespace Dnssd
} // namespace chip
