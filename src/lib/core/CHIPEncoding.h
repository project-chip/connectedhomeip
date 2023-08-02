/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
 *    Copyright (c) 2013-2017 Nest Labs, Inc.
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
 *    @file
 *      This file provides functions for:
 *
 *      <ul>
 *         <li>Performing byte reordering by value for 16-, 32-, and 64-bit
 *         types.</li>
 *         <li>Safely performing simple, efficient memory-mapped
 *         accesses, potentially to unaligned memory locations, with or
 *         without byte reordering, to 8-, 16-, 32-, and 64-bit
 *         quantities, both with and without pointer management.</li>
 *      </ul>
 *
 */

#pragma once

// The nlio headers use [inout] instead of [in,out], which makes the clang
// documentation warning unhappy.  Suppress it for those headers.
#pragma GCC diagnostic push
#ifdef __clang__
#pragma GCC diagnostic ignored "-Wdocumentation"
#endif // __clang__

#include <nlbyteorder.hpp>
#include <nlio-byteorder.hpp>
#include <nlio.hpp>

#pragma GCC diagnostic pop

#include <stdint.h>

namespace chip {

/**
 *  @namespace chip::Encoding
 *
 *  This namespace provides functions for:
 *
 *  <ul>
 *     <li>Performing byte reordering by value for 16-, 32-, and 64-bit
 *     types.</li>
 *     <li>Safely performing simple, efficient memory-mapped
 *     accesses, potentially to unaligned memory locations, with or
 *     without byte reordering, to 8-, 16-, 32-, and 64-bit
 *     quantities, both with and without pointer management.</li>
 *  </ul>
 *
 */
namespace Encoding {

/**
 * This unconditionally performs a byte order swap by value of the
 * specified 16-bit value.
 *
 * @param[in]  v  The 16-bit value to be byte order swapped.
 *
 * @return The input value, byte order swapped.
 */
inline uint16_t Swap16(uint16_t v)
{
    return nl::ByteOrder::Swap16(v);
}

/**
 * This unconditionally performs a byte order swap by value of the
 * specified 32-bit value.
 *
 * @param[in]  v  The 32-bit value to be byte order swapped.
 *
 * @return The input value, byte order swapped.
 */
inline uint32_t Swap32(uint32_t v)
{
    return nl::ByteOrder::Swap32(v);
}

/**
 * This unconditionally performs a byte order swap by value of the
 * specified 64-bit value.
 *
 * @param[in]  v  The 64-bit value to be byte order swapped.
 *
 * @return The input value, byte order swapped.
 */
inline uint64_t Swap64(uint64_t v)
{
    return nl::ByteOrder::Swap64(v);
}

/**
 * Perform a, potentially unaligned, memory read of the 8-bit value
 * from the specified pointer address.
 *
 * @param[in]  p      A pointer address, potentially unaligned, to read
 *                    the 8-bit value from.
 *
 * @return The 8-bit value at the specified pointer address.
 */
inline uint8_t Get8(const uint8_t * p)
{
    return nl::IO::Get8(p);
}

/**
 * Perform a, potentially unaligned, memory write of the target system
 * byte ordered 8-bit value to the specified pointer address.
 *
 * @param[in]  p      A pointer address, potentially unaligned, to write
 *                    the 8-bit value to.
 *
 * @param[in]  v      The 8-bit value to write.
 *
 */
inline void Put8(uint8_t * p, uint8_t v)
{
    nl::IO::Put8(p, v);
}

/**
 * Perform a, potentially unaligned, memory read of the 8-bit value
 * from the specified pointer address and increment the pointer by
 * 8-bits (1 byte).
 *
 * @param[in,out] p   A reference to a constant pointer address,
 *                    potentially unaligned, to read the 8-bit value
 *                    from and to then increment by 8-bits (1 byte).
 *
 * @return The 8-bit value at the specified pointer address.
 */
inline uint8_t Read8(const uint8_t *& p)
{
    return nl::IO::Read8(reinterpret_cast<const void *&>(p));
}

/**
 * Perform a, potentially unaligned, memory read of the 8-bit value
 * from the specified pointer address and increment the pointer by
 * 8-bits (1 byte).
 *
 * @param[in,out] p   A reference to a pointer address, potentially
 *                    unaligned, to read the 8-bit value from and to
 *                    then increment by 8-bits (1 byte).
 *
 * @return The 8-bit value at the specified pointer address.
 */
inline uint8_t Read8(uint8_t *& p)
{
    return Read8(const_cast<const uint8_t *&>(p));
}

/**
 * Perform a, potentially unaligned, memory write of the 8-bit value
 * to the specified pointer address and increment the pointer by
 * 8-bits (1 byte).
 *
 * @param[in,out] p   A reference to a pointer address, potentially
 *                    unaligned, to read the 8-bit value from and to
 *                    then increment by 8-bits (1 byte).
 *
 * @param[in]     v   The 8-bit value to write.
 *
 */
inline void Write8(uint8_t *& p, uint8_t v)
{
    *p++ = v;
}

/**
 *  @namespace chip::Encoding::LittleEndian
 *
 *  This namespace provides functions for:
 *
 *  <ul>
 *     <li>Performing byte reordering, relative to the host system, by
 *     value of little endian byte ordered values for 16-, 32-, and
 *     64-bit types.</li>
 *     <li>Safely performing simple, efficient memory-mapped accesses,
 *     potentially to unaligned memory locations, with or without byte
 *     reordering, to 8-, 16-, 32-, and 64-bit little endian byte
 *     ordered quantities, both with and without pointer
 *     management.</li>
 *  </ul>
 *
 *  On little endian host systems no actual byte reordering will
 *  occur. On other systems, byte reordering is performed as
 *  appropriate.
 *
 */
namespace LittleEndian {

/**
 * This conditionally performs, as necessary for the target system, a
 * byte order swap by value of the specified value, presumed to be in
 * little endian byte ordering to the target system (i.e. host)
 * byte ordering.
 */
template <typename T>
inline T HostSwap(T v);

// For completeness of the set, we have identity.
template <>
inline uint8_t HostSwap<uint8_t>(uint8_t v)
{
    return v;
}

/**
 * This conditionally performs, as necessary for the target system, a
 * byte order swap by value of the specified 16-bit value, presumed to
 * be in little endian byte ordering to the target system (i.e. host)
 * byte ordering.
 *
 * Consequently, on little endian target systems, this is a no-op and
 * on big endian target systems, this performs a reordering.
 *
 * @param[in]  v  The 16-bit value to be byte order swapped.
 *
 * @return The input value, if necessary, byte order swapped.
 */
inline uint16_t HostSwap16(uint16_t v)
{
    return nl::ByteOrder::Swap16LittleToHost(v);
}

template <>
inline uint16_t HostSwap<uint16_t>(uint16_t v)
{
    return HostSwap16(v);
}

/**
 * This conditionally performs, as necessary for the target system, a
 * byte order swap by value of the specified 32-bit value, presumed to
 * be in little endian byte ordering to the target system (i.e. host)
 * byte ordering.
 *
 * Consequently, on little endian target systems, this is a no-op and
 * on big endian target systems, this performs a reordering.
 *
 * @param[in]  v  The 32-bit value to be byte order swapped.
 *
 * @return The input value, if necessary, byte order swapped.
 */
inline uint32_t HostSwap32(uint32_t v)
{
    return nl::ByteOrder::Swap32LittleToHost(v);
}

template <>
inline uint32_t HostSwap<uint32_t>(uint32_t v)
{
    return HostSwap32(v);
}

/**
 * This conditionally performs, as necessary for the target system, a
 * byte order swap by value of the specified 64-bit value, presumed to
 * be in little endian byte ordering to the target system (i.e. host)
 * byte ordering.
 *
 * Consequently, on little endian target systems, this is a no-op and
 * on big endian target systems, this performs a reordering.
 *
 * @param[in]  v  The 64-bit value to be byte order swapped.
 *
 * @return The input value, if necessary, byte order swapped.
 */
inline uint64_t HostSwap64(uint64_t v)
{
    return nl::ByteOrder::Swap64LittleToHost(v);
}

template <>
inline uint64_t HostSwap<uint64_t>(uint64_t v)
{
    return HostSwap64(v);
}

/**
 * Perform a, potentially unaligned, memory read of the little endian
 * byte ordered 16-bit value from the specified pointer address,
 * perform byte reordering, as necessary, for the target system to
 * put the value in target system byte ordering.
 *
 * @param[in]  p      A pointer address, potentially unaligned, to read
 *                    the 16-bit little endian byte ordered value from.
 *
 * @return The 16-bit value at the specified pointer address, if
 *         necessary, byte reordered.
 */
inline uint16_t Get16(const uint8_t * p)
{
    return nl::IO::LittleEndian::GetUnaligned16(p);
}

/**
 * Perform a, potentially unaligned, memory read of the little endian
 * byte ordered 32-bit value from the specified pointer address,
 * perform byte reordering, as necessary, for the target system to
 * put the value in target system byte ordering.
 *
 * @param[in]  p      A pointer address, potentially unaligned, to read
 *                    the 32-bit little endian byte ordered value from.
 *
 * @return The 32-bit value at the specified pointer address, if
 *         necessary, byte reordered.
 */
inline uint32_t Get32(const uint8_t * p)
{
    return nl::IO::LittleEndian::GetUnaligned32(p);
}

/**
 * Perform a, potentially unaligned, memory read of the little endian
 * byte ordered 64-bit value from the specified pointer address,
 * perform byte reordering, as necessary, for the target system to
 * put the value in target system byte ordering.
 *
 * @param[in]  p      A pointer address, potentially unaligned, to read
 *                    the 64-bit little endian byte ordered value from.
 *
 * @return The 64-bit value at the specified pointer address, if
 *         necessary, byte reordered.
 */
inline uint64_t Get64(const uint8_t * p)
{
    return nl::IO::LittleEndian::GetUnaligned64(p);
}

/**
 * Perform a, potentially unaligned, memory write of the target system
 * byte ordered 16-bit value to the specified pointer address,
 * perform byte reordering, as necessary, for the target system to
 * put the value in little endian byte ordering.
 *
 * @param[in]  p      A pointer address, potentially unaligned, to write
 *                    the target system byte ordered 16-bit value to in little
 *                    endian byte ordering.
 *
 * @param[in]  v      The 16-bit value to write, if necessary, byte reordered.
 *
 */
inline void Put16(uint8_t * p, uint16_t v)
{
    nl::IO::LittleEndian::PutUnaligned16(p, v);
}

/**
 * Perform a, potentially unaligned, memory write of the target system
 * byte ordered 32-bit value to the specified pointer address,
 * perform byte reordering, as necessary, for the target system to
 * put the value in little endian byte ordering.
 *
 * @param[in]  p      A pointer address, potentially unaligned, to write
 *                    the target system byte ordered 32-bit value to in little
 *                    endian byte ordering.
 *
 * @param[in]  v      The 32-bit value to write, if necessary, byte reordered.
 *
 */
inline void Put32(uint8_t * p, uint32_t v)
{
    nl::IO::LittleEndian::PutUnaligned32(p, v);
}

/**
 * Perform a, potentially unaligned, memory write of the target system
 * byte ordered 64-bit value to the specified pointer address,
 * perform byte reordering, as necessary, for the target system to
 * put the value in little endian byte ordering.
 *
 * @param[in]  p      A pointer address, potentially unaligned, to write
 *                    the target system byte ordered 64-bit value to in little
 *                    endian byte ordering.
 *
 * @param[in]  v      The 64-bit value to write, if necessary, byte reordered.
 *
 */
inline void Put64(uint8_t * p, uint64_t v)
{
    nl::IO::LittleEndian::PutUnaligned64(p, v);
}

/**
 * Perform a, potentially unaligned, memory read of the little endian
 * byte ordered 16-bit value from the specified pointer address,
 * perform byte reordering, as necessary, for the target system to put
 * the value in target system byte ordering, and increment the pointer
 * by 16-bits (2 bytes).
 *
 * @param[in,out] p   A reference to a constant pointer address, potentially
 *                    unaligned, to read the 16-bit little endian byte
 *                    ordered value from and to then increment by 16-
 *                    bits (2 bytes).
 *
 * @return The 16-bit value at the specified pointer address, if necessary,
 *         byte order swapped.
 */
inline uint16_t Read16(const uint8_t *& p)
{
    return nl::IO::LittleEndian::ReadUnaligned16(reinterpret_cast<const void *&>(p));
}

/**
 * Perform a, potentially unaligned, memory read of the little endian
 * byte ordered 16-bit value from the specified pointer address,
 * perform byte reordering, as necessary, for the target system to put
 * the value in target system byte ordering, and increment the pointer
 * by 16-bits (2 bytes).
 *
 * @param[in,out] p   A reference to a pointer address, potentially
 *                    unaligned, to read the 16-bit little endian byte
 *                    ordered value from and to then increment by 16-
 *                    bits (2 bytes).
 *
 * @return The 16-bit value at the specified pointer address, if necessary,
 *         byte order swapped.
 */
inline uint16_t Read16(uint8_t *& p)
{
    return Read16(const_cast<const uint8_t *&>(p));
}

/**
 * Perform a, potentially unaligned, memory read of the little endian
 * byte ordered 32-bit value from the specified pointer address,
 * perform byte reordering, as necessary, for the target system to put
 * the value in target system byte ordering, and increment the pointer
 * by 32-bits (4 bytes).
 *
 * @param[in,out] p   A reference to a constant pointer address, potentially
 *                    unaligned, to read the 32-bit little endian byte
 *                    ordered value from and to then increment by 32-
 *                    bits (4 bytes).
 *
 * @return The 32-bit value at the specified pointer address, if necessary,
 *         byte order swapped.
 */
inline uint32_t Read32(const uint8_t *& p)
{
    return nl::IO::LittleEndian::ReadUnaligned32(reinterpret_cast<const void *&>(p));
}

/**
 * Perform a, potentially unaligned, memory read of the little endian
 * byte ordered 32-bit value from the specified pointer address,
 * perform byte reordering, as necessary, for the target system to put
 * the value in target system byte ordering, and increment the pointer
 * by 32-bits (4 bytes).
 *
 * @param[in,out] p   A reference to a pointer address, potentially
 *                    unaligned, to read the 32-bit little endian byte
 *                    ordered value from and to then increment by 32-
 *                    bits (4 bytes).
 *
 * @return The 32-bit value at the specified pointer address, if necessary,
 *         byte order swapped.
 */
inline uint32_t Read32(uint8_t *& p)
{
    return Read32(const_cast<const uint8_t *&>(p));
}

/**
 * Perform a, potentially unaligned, memory read of the little endian
 * byte ordered 64-bit value from the specified pointer address,
 * perform byte reordering, as necessary, for the target system to put
 * the value in target system byte ordering, and increment the pointer
 * by 64-bits (8 bytes).
 *
 * @param[in,out] p   A reference to a constant pointer address, potentially
 *                    unaligned, to read the 64-bit little endian byte
 *                    ordered value from and to then increment by 64-
 *                    bits (8 bytes).
 *
 * @return The 64-bit value at the specified pointer address, if necessary,
 *         byte order swapped.
 */
inline uint64_t Read64(const uint8_t *& p)
{
    return nl::IO::LittleEndian::ReadUnaligned64(reinterpret_cast<const void *&>(p));
}

/**
 * Perform a, potentially unaligned, memory read of the little endian
 * byte ordered 64-bit value from the specified pointer address,
 * perform byte reordering, as necessary, for the target system to put
 * the value in target system byte ordering, and increment the pointer
 * by 64-bits (8 bytes).
 *
 * @param[in,out] p   A reference to a pointer address, potentially
 *                    unaligned, to read the 64-bit little endian byte
 *                    ordered value from and to then increment by 64-
 *                    bits (8 bytes).
 *
 * @return The 64-bit value at the specified pointer address, if necessary,
 *         byte order swapped.
 */
inline uint64_t Read64(uint8_t *& p)
{
    return Read64(const_cast<const uint8_t *&>(p));
}

/**
 * Perform a, potentially unaligned, memory write of the target system
 * byte ordered 16-bit value to the specified pointer address,
 * perform byte reordering, as necessary, for the target system to
 * put the value in little endian byte ordering.
 *
 * @param[in]  p      A reference to a pointer address, potentially
 *                    unaligned, to write the target system byte
 *                    ordered 16-bit value to in little endian byte
 *                    ordering and to then increment by 16-bits (2
 *                    bytes).
 *
 * @param[in]  v      The 16-bit value to write, if necessary, byte order
 *                    swapped.
 *
 */
inline void Write16(uint8_t *& p, uint16_t v)
{
    Put16(p, v);
    p += sizeof(uint16_t);
}

/**
 * Perform a, potentially unaligned, memory write of the target system
 * byte ordered 32-bit value to the specified pointer address,
 * perform byte reordering, as necessary, for the target system to
 * put the value in little endian byte ordering.
 *
 * @param[in]  p      A reference to a pointer address, potentially
 *                    unaligned, to write the target system byte
 *                    ordered 32-bit value to in little endian byte
 *                    ordering and to then increment by 32-bits (4
 *                    bytes).
 *
 * @param[in]  v      The 32-bit value to write, if necessary, byte order
 *                    swapped.
 *
 */
inline void Write32(uint8_t *& p, uint32_t v)
{
    Put32(p, v);
    p += sizeof(uint32_t);
}

/**
 * Perform a, potentially unaligned, memory write of the target system
 * byte ordered 64-bit value to the specified pointer address,
 * perform byte reordering, as necessary, for the target system to
 * put the value in little endian byte ordering.
 *
 * @param[in]  p      A reference to a pointer address, potentially
 *                    unaligned, to write the target system byte
 *                    ordered 64-bit value to in little endian byte
 *                    ordering and to then increment by 64-bits (8
 *                    bytes).
 *
 * @param[in]  v      The 64-bit value to write, if necessary, byte order
 *                    swapped.
 *
 */
inline void Write64(uint8_t *& p, uint64_t v)
{
    Put64(p, v);
    p += sizeof(uint64_t);
}

} // namespace LittleEndian

/**
 *  @namespace chip::Encoding::BigEndian
 *
 *  This namespace provides functions for:
 *
 *  <ul>
 *     <li>Performing byte reordering, relative to the host system, by
 *     value of big endian byte ordered values for 16-, 32-, and
 *     64-bit types.</li>
 *     <li>Safely performing simple, efficient memory-mapped accesses,
 *     potentially to unaligned memory locations, with or without byte
 *     reordering, to 8-, 16-, 32-, and 64-bit big endian byte
 *     ordered quantities, both with and without pointer
 *     management.</li>
 *  </ul>
 *
 *  On big endian host systems no actual byte reordering will
 *  occur. On other systems, byte reordering is performed as
 *  appropriate.
 *
 */
namespace BigEndian {

/**
 * This conditionally performs, as necessary for the target system, a
 * byte order swap by value of the specified 16-bit value, presumed to
 * be in big endian byte ordering to the target system (i.e. host)
 * byte ordering.
 *
 * Consequently, on bit endian target systems, this is a no-op and
 * on big endian target systems, this performs a reordering.
 *
 * @param[in]  v  The 16-bit value to be byte order swapped.
 *
 * @return The input value, if necessary, byte order swapped.
 */
inline uint16_t HostSwap16(uint16_t v)
{
    return nl::ByteOrder::Swap16BigToHost(v);
}

/**
 * This conditionally performs, as necessary for the target system, a
 * byte order swap by value of the specified 32-bit value, presumed to
 * be in big endian byte ordering to the target system (i.e. host)
 * byte ordering.
 *
 * Consequently, on bit endian target systems, this is a no-op and
 * on big endian target systems, this performs a reordering.
 *
 * @param[in]  v  The 32-bit value to be byte order swapped.
 *
 * @return The input value, if necessary, byte order swapped.
 */
inline uint32_t HostSwap32(uint32_t v)
{
    return nl::ByteOrder::Swap32BigToHost(v);
}

/**
 * This conditionally performs, as necessary for the target system, a
 * byte order swap by value of the specified 64-bit value, presumed to
 * be in big endian byte ordering to the target system (i.e. host)
 * byte ordering.
 *
 * Consequently, on bit endian target systems, this is a no-op and
 * on big endian target systems, this performs a reordering.
 *
 * @param[in]  v  The 64-bit value to be byte order swapped.
 *
 * @return The input value, if necessary, byte order swapped.
 */
inline uint64_t HostSwap64(uint64_t v)
{
    return nl::ByteOrder::Swap64BigToHost(v);
}

/**
 * Perform a, potentially unaligned, memory read of the big endian
 * byte ordered 16-bit value from the specified pointer address,
 * perform byte reordering, as necessary, for the target system to
 * put the value in target system byte ordering.
 *
 * @param[in]  p      A pointer address, potentially unaligned, to read
 *                    the 16-bit big endian byte ordered value from.
 *
 * @return The 16-bit value at the specified pointer address, if
 *         necessary, byte reordered.
 */
inline uint16_t Get16(const uint8_t * p)
{
    return nl::IO::BigEndian::GetUnaligned16(p);
}

/**
 * Perform a, potentially unaligned, memory read of the big endian
 * byte ordered 32-bit value from the specified pointer address,
 * perform byte reordering, as necessary, for the target system to
 * put the value in target system byte ordering.
 *
 * @param[in]  p      A pointer address, potentially unaligned, to read
 *                    the 32-bit big endian byte ordered value from.
 *
 * @return The 32-bit value at the specified pointer address, if
 *         necessary, byte reordered.
 */
inline uint32_t Get32(const uint8_t * p)
{
    return nl::IO::BigEndian::GetUnaligned32(p);
}

/**
 * Perform a, potentially unaligned, memory read of the big endian
 * byte ordered 64-bit value from the specified pointer address,
 * perform byte reordering, as necessary, for the target system to
 * put the value in target system byte ordering.
 *
 * @param[in]  p      A pointer address, potentially unaligned, to read
 *                    the 64-bit big endian byte ordered value from.
 *
 * @return The 64-bit value at the specified pointer address, if
 *         necessary, byte reordered.
 */
inline uint64_t Get64(const uint8_t * p)
{
    return nl::IO::BigEndian::GetUnaligned64(p);
}

/**
 * Perform a, potentially unaligned, memory write of the target system
 * byte ordered 16-bit value to the specified pointer address,
 * perform byte reordering, as necessary, for the target system to
 * put the value in big endian byte ordering.
 *
 * @param[in]  p      A pointer address, potentially unaligned, to write
 *                    the target system byte ordered 16-bit value to in big
 *                    endian byte ordering.
 *
 * @param[in]  v      The 16-bit value to write, if necessary, byte order
 *                    swapped.
 *
 */
inline void Put16(uint8_t * p, uint16_t v)
{
    nl::IO::BigEndian::PutUnaligned16(p, v);
}

/**
 * Perform a, potentially unaligned, memory write of the target system
 * byte ordered 32-bit value to the specified pointer address,
 * perform byte reordering, as necessary, for the target system to
 * put the value in big endian byte ordering.
 *
 * @param[in]  p      A pointer address, potentially unaligned, to write
 *                    the target system byte ordered 32-bit value to in big
 *                    endian byte ordering.
 *
 * @param[in]  v      The 32-bit value to write, if necessary, byte order
 *                    swapped.
 *
 */
inline void Put32(uint8_t * p, uint32_t v)
{
    nl::IO::BigEndian::PutUnaligned32(p, v);
}

/**
 * Perform a, potentially unaligned, memory write of the target system
 * byte ordered 64-bit value to the specified pointer address,
 * perform byte reordering, as necessary, for the target system to
 * put the value in big endian byte ordering.
 *
 * @param[in]  p      A pointer address, potentially unaligned, to write
 *                    the target system byte ordered 64-bit value to in big
 *                    endian byte ordering.
 *
 * @param[in]  v      The 64-bit value to write, if necessary, byte order
 *                    swapped.
 *
 */
inline void Put64(uint8_t * p, uint64_t v)
{
    nl::IO::BigEndian::PutUnaligned64(p, v);
}

/**
 * Perform a, potentially unaligned, memory read of the big endian
 * byte ordered 16-bit value from the specified pointer address,
 * perform byte reordering, as necessary, for the target system to put
 * the value in target system byte ordering, and increment the pointer
 * by 16-bits (2 bytes).
 *
 * @param[in,out] p   A reference to a constant pointer address, potentially
 *                    unaligned, to read the 16-bit big endian byte
 *                    ordered value from and to then increment by 16-
 *                    bits (2 bytes).
 *
 * @return The 16-bit value at the specified pointer address, if necessary,
 *         byte order swapped.
 */
inline uint16_t Read16(const uint8_t *& p)
{
    return nl::IO::BigEndian::ReadUnaligned16(reinterpret_cast<const void *&>(p));
}

/**
 * Perform a, potentially unaligned, memory read of the big endian
 * byte ordered 16-bit value from the specified pointer address,
 * perform byte reordering, as necessary, for the target system to put
 * the value in target system byte ordering, and increment the pointer
 * by 16-bits (2 bytes).
 *
 * @param[in,out] p   A reference to a pointer address, potentially
 *                    unaligned, to read the 16-bit big endian byte
 *                    ordered value from and to then increment by 16-
 *                    bits (2 bytes).
 *
 * @return The 16-bit value at the specified pointer address, if necessary,
 *         byte order swapped.
 */
inline uint16_t Read16(uint8_t *& p)
{
    return Read16(const_cast<const uint8_t *&>(p));
}

/**
 * Perform a, potentially unaligned, memory read of the big endian
 * byte ordered 32-bit value from the specified pointer address,
 * perform byte reordering, as necessary, for the target system to put
 * the value in target system byte ordering, and increment the pointer
 * by 32-bits (4 bytes).
 *
 * @param[in,out] p   A reference to a constant pointer address, potentially
 *                    unaligned, to read the 32-bit big endian byte
 *                    ordered value from and to then increment by 32-
 *                    bits (4 bytes).
 *
 * @return The 32-bit value at the specified pointer address, if necessary,
 *         byte order swapped.
 */
inline uint32_t Read32(const uint8_t *& p)
{
    return nl::IO::BigEndian::ReadUnaligned32(reinterpret_cast<const void *&>(p));
}

/**
 * Perform a, potentially unaligned, memory read of the big endian
 * byte ordered 32-bit value from the specified pointer address,
 * perform byte reordering, as necessary, for the target system to put
 * the value in target system byte ordering, and increment the pointer
 * by 32-bits (4 bytes).
 *
 * @param[in,out] p   A reference to a pointer address, potentially
 *                    unaligned, to read the 32-bit big endian byte
 *                    ordered value from and to then increment by 32-
 *                    bits (4 bytes).
 *
 * @return The 32-bit value at the specified pointer address, if necessary,
 *         byte order swapped.
 */
inline uint32_t Read32(uint8_t *& p)
{
    return Read32(const_cast<const uint8_t *&>(p));
}

/**
 * Perform a, potentially unaligned, memory read of the big endian
 * byte ordered 64-bit value from the specified pointer address,
 * perform byte reordering, as necessary, for the target system to put
 * the value in target system byte ordering, and increment the pointer
 * by 64-bits (8 bytes).
 *
 * @param[in,out] p   A reference to a constant pointer address, potentially
 *                    unaligned, to read the 64-bit big endian byte
 *                    ordered value from and to then increment by 64-
 *                    bits (8 bytes).
 *
 * @return The 64-bit value at the specified pointer address, if necessary,
 *         byte order swapped.
 */
inline uint64_t Read64(const uint8_t *& p)
{
    return nl::IO::BigEndian::ReadUnaligned64(reinterpret_cast<const void *&>(p));
}

/**
 * Perform a, potentially unaligned, memory read of the big endian
 * byte ordered 64-bit value from the specified pointer address,
 * perform byte reordering, as necessary, for the target system to put
 * the value in target system byte ordering, and increment the pointer
 * by 64-bits (8 bytes).
 *
 * @param[in,out] p   A reference to a pointer address, potentially
 *                    unaligned, to read the 64-bit big endian byte
 *                    ordered value from and to then increment by 64-
 *                    bits (8 bytes).
 *
 * @return The 64-bit value at the specified pointer address, if necessary,
 *         byte order swapped.
 */
inline uint64_t Read64(uint8_t *& p)
{
    return Read64(const_cast<const uint8_t *&>(p));
}

/**
 * Perform a, potentially unaligned, memory write of the target system
 * byte ordered 16-bit value to the specified pointer address,
 * perform byte reordering, as necessary, for the target system to
 * put the value in big endian byte ordering.
 *
 * @param[in]  p      A reference to a pointer address, potentially
 *                    unaligned, to write the target system byte
 *                    ordered 16-bit value to in big endian byte
 *                    ordering and to then increment by 16-bits (2
 *                    bytes).
 *
 * @param[in]  v      The 16-bit value to write, if necessary, byte order
 *                    swapped.
 *
 */
inline void Write16(uint8_t *& p, uint16_t v)
{
    Put16(p, v);
    p += sizeof(uint16_t);
}

/**
 * Perform a, potentially unaligned, memory write of the target system
 * byte ordered 32-bit value to the specified pointer address,
 * perform byte reordering, as necessary, for the target system to
 * put the value in big endian byte ordering.
 *
 * @param[in]  p      A reference to a pointer address, potentially
 *                    unaligned, to write the target system byte
 *                    ordered 32-bit value to in big endian byte
 *                    ordering and to then increment by 32-bits (4
 *                    bytes).
 *
 * @param[in]  v      The 32-bit value to write, if necessary, byte order
 *                    swapped.
 *
 */
inline void Write32(uint8_t *& p, uint32_t v)
{
    Put32(p, v);
    p += sizeof(uint32_t);
}

/**
 * Perform a, potentially unaligned, memory write of the target system
 * byte ordered 64-bit value to the specified pointer address,
 * perform byte reordering, as necessary, for the target system to
 * put the value in big endian byte ordering.
 *
 * @param[in]  p      A reference to a pointer address, potentially
 *                    unaligned, to write the target system byte
 *                    ordered 64-bit value to in big endian byte
 *                    ordering and to then increment by 64-bits (8
 *                    bytes).
 *
 * @param[in]  v      The 64-bit value to write, if necessary, byte order
 *                    swapped.
 *
 */
inline void Write64(uint8_t *& p, uint64_t v)
{
    Put64(p, v);
    p += sizeof(uint64_t);
}

} // namespace BigEndian

} // namespace Encoding
} // namespace chip
