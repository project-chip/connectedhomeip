/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
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
#pragma once

#include <inet/IPAddress.h>
#include <lib/core/CHIPEncoding.h>
#include <lib/dnssd/wire/QName.h>
#include <lib/support/BufferWriter.h>
#include <lib/support/Span.h>

#include <optional>

namespace mdns {
namespace Minimal {

/**
 * Handles writing into mdns packets.
 *
 * Generally the same as a binary data writer, but can handle qname writing with
 * compression.
 */
class RecordWriter
{
public:
    RecordWriter(chip::Encoding::BigEndian::BufferWriter * output) : mOutput(output) { Reset(); }

    void Reset()
    {
        for (size_t i = 0; i < kMaxCachedReferences; i++)
        {
            mPreviousQNames[i] = kInvalidOffset;
        }
    }

    chip::Encoding::BigEndian::BufferWriter & Writer() { return *mOutput; }

    /// Writes  the given qname into the underlying buffer, applying
    /// compression if possible
    RecordWriter & WriteQName(const FullQName & qname);

    /// Writes  the given qname into the underlying buffer, applying
    /// compression if possible
    RecordWriter & WriteQName(const SerializedQNameIterator & qname);

    inline RecordWriter & Put8(uint8_t value)
    {
        mOutput->Put8(value);
        return *this;
    }

    inline RecordWriter & Put16(uint16_t value)
    {
        mOutput->Put16(value);
        return *this;
    }

    inline RecordWriter & Put32(uint32_t value)
    {
        mOutput->Put32(value);
        return *this;
    }

    inline RecordWriter & PutString(const char * value)
    {
        mOutput->Put(value);
        return *this;
    }

    inline RecordWriter & Put(const BytesRange & range)
    {
        mOutput->Put(range.Start(), range.Size());
        return *this;
    }

    // Maximum length of a single TXT character-string entry.
    // Per DNS-SD TXT Record Size (RFC 6763), it is suggested to keep the length of a single TXT entry small.
    // While it can be larger than 63, this is the current value used in lib/dnssd/minimal_mdns/records/Txt.h.
    // TODO : I could not find any reference to why 63 is used. This shall be re-visited during ULD implementation.
    static constexpr size_t kMaxTxtRecordLength = 63;

    /**
     * @brief Writes a placeholder 16-bit RDLENGTH field.
     *
     * @return A BufferWriter positioned at the reserved RDLENGTH. After the record
     *         data has been written, pass this value to FinishRdlength() to
     *         backpatch the actual data length.
     */
    chip::Encoding::BigEndian::BufferWriter ReserveRdlength();

    /**
     * @brief Backpatches the RDLENGTH reserved by ReserveRdlength().
     *
     * Writes the number of bytes written to the underlying output since the
     * reservation into the previously reserved 16-bit length field.
     *
     * @param[in,out] rdlengthPosition BufferWriter returned by ReserveRdlength().
     */
    void FinishRdlength(chip::Encoding::BigEndian::BufferWriter & rdlengthPosition);

    /**
     * @brief Writes SRV record data (priority, weight, port, target name).
     *
     * The target name is subject to qname compression.
     *
     * @param[in] priority SRV priority field.
     * @param[in] weight   SRV weight field.
     * @param[in] port     SRV port field.
     * @param[in] server   Target host name.
     *
     * @return Reference to this writer for chaining.
     */
    RecordWriter & PutSrv(uint16_t priority, uint16_t weight, uint16_t port, const FullQName & server);

    /**
     * @brief Writes PTR record data (a single target name).
     *
     * The target name may be qname-compressed.
     *
     * @param[in] name Target name.
     *
     * @return Reference to this writer for chaining.
     */
    RecordWriter & PutPtr(const FullQName & name);

    /**
     * @brief Writes A or AAAA record data for the given address.
     *
     * Emits 4 (IPv4) or 16 (IPv6) address bytes in network byte order, selected
     * by the address family.
     *
     * @param[in] address IP address to encode.
     *
     * @return Reference to this writer for chaining.
     */
    RecordWriter & PutIpAddress(const chip::Inet::IPAddress & address);

    /// Writes TXT record data as a sequence of length-prefixed character-strings.
    /// Returns false (writing nothing further) if any entry exceeds
    /// kMaxTxtRecordLength.
    bool PutTxt(chip::Span<const char * const> entries);

    inline bool Fit() const { return mOutput->Fit(); }

private:
    // How  many paths to remember as 'previously written'
    // and make use of them
    static constexpr size_t kMaxCachedReferences = 8;
    static constexpr uint16_t kInvalidOffset     = 0xFFFF;
    static constexpr uint16_t kMaxReuseOffset    = 0x3FFF;

    // Where the data is being outputted
    chip::Encoding::BigEndian::BufferWriter * mOutput;
    uint16_t mPreviousQNames[kMaxCachedReferences];

    /// Find the offset at which this qname was previously seen (if any)
    /// works with QName and SerializedQNameIterator
    template <class T>
    std::optional<uint16_t> FindPreviousName(const T & name) const
    {
        for (size_t i = 0; i < kMaxCachedReferences; i++)
        {
            SerializedQNameIterator previous = PreviousName(i);

            // Any of the sub-segments may match
            while (previous.IsValid())
            {
                if (previous == name)
                {
                    return std::make_optional(static_cast<uint16_t>(previous.OffsetInCurrentValidData()));
                }

                if (!previous.Next())
                {
                    break;
                }
            }
        }

        return std::nullopt;
    }

    /// Gets the iterator corresponding to the previous name
    /// with the given index.
    ///
    /// Will return an iterator  that is not valid if
    /// lookbehind index is not valid
    SerializedQNameIterator PreviousName(size_t index) const;

    /// Keep track that a qname was written at the given offset
    void RememberWrittenQnameOffset(size_t offset);
};

} // namespace Minimal
} // namespace mdns
