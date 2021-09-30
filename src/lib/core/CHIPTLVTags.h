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
 *      This file contains definitions for working with CHIP TLV tags.
 *
 */

#pragma once

namespace chip {
namespace TLV {

enum TLVCommonProfiles
{
    /**
     * Used to indicate the absence of a profile id in a variable or member.
     * This is essentially the same as kCHIPProfile_NotSpecified defined in CHIPProfiles.h
     */
    kProfileIdNotSpecified = 0xFFFFFFFF,

    // TODO: Replace with chip::Profiles::kCHIPProfile_Common
    kCommonProfileId = 0
};

// TODO: Move to private namespace
enum TLVTagFields
{
    kProfileIdMask    = 0xFFFFFFFF00000000ULL,
    kProfileIdShift   = 32,
    kVendorIdShift    = 48,
    kProfileNumShift  = 32,
    kTagNumMask       = 0x00000000FFFFFFFFULL,
    kSpecialTagMarker = 0xFFFFFFFF00000000ULL,
    kContextTagMaxNum = 256
};

// TODO: Move to private namespace
enum class TLVTagControl : uint8_t
{
    // IMPORTANT: All values here must have no bits in common with specified
    // values of TLVElementType.
    Anonymous              = 0x00,
    ContextSpecific        = 0x20,
    CommonProfile_2Bytes   = 0x40,
    CommonProfile_4Bytes   = 0x60,
    ImplicitProfile_2Bytes = 0x80,
    ImplicitProfile_4Bytes = 0xA0,
    FullyQualified_6Bytes  = 0xC0,
    FullyQualified_8Bytes  = 0xE0
};

template <typename T>
inline uint8_t operator>>(TLVTagControl lhs, const T & rhs)
{
    return static_cast<uint8_t>(static_cast<uint8_t>(lhs) >> rhs);
}

// TODO: Move to private namespace
enum
{
    kTLVTagControlMask  = 0xE0,
    kTLVTagControlShift = 5
};

/**
 * Generates the API representation of a profile-specific TLV tag from a profile id and tag number
 *
 * @param[in]   profileId       The id of the profile within which the tag is defined.
 * @param[in]   tagNum          The profile-specific tag number assigned to the tag.
 * @return                      A 64-bit integer representing the tag.
 */
inline constexpr uint64_t ProfileTag(uint32_t profileId, uint32_t tagNum)
{
    return ((static_cast<uint64_t>(profileId)) << kProfileIdShift) | tagNum;
}

/**
 * Generates the API representation of a profile-specific TLV tag from a vendor id, profile number and tag number
 *
 * @param[in]   vendorId        The id of the vendor that defined the tag.
 * @param[in]   profileNum      The vendor assigned number for the profile within which the tag is defined.
 * @param[in]   tagNum          The profile-specific tag number assigned to the tag.
 * @return                      A 64-bit integer representing the tag.
 */
inline uint64_t ProfileTag(uint16_t vendorId, uint16_t profileNum, uint32_t tagNum)
{
    return ((static_cast<uint64_t>(vendorId)) << kVendorIdShift) | ((static_cast<uint64_t>(profileNum)) << kProfileNumShift) |
        tagNum;
}

/**
 * Generates the API representation for of context-specific TLV tag
 *
 * @param[in]   tagNum          The context-specific tag number assigned to the tag.
 * @return                      A 64-bit integer representing the tag.
 */
inline constexpr uint64_t ContextTag(uint8_t tagNum)
{
    return kSpecialTagMarker | tagNum;
}

/**
 * Generates the API representation of a common profile TLV tag
 *
 * @param[in]   tagNum          The common profile tag number assigned to the tag.
 * @return                      A 64-bit integer representing the tag.
 */
inline uint64_t CommonTag(uint32_t tagNum)
{
    return ProfileTag(kCommonProfileId, tagNum);
}

enum
{
    /**
     * A value signifying a TLV element that has no tag (i.e. an anonymous element).
     */
    AnonymousTag = kSpecialTagMarker | 0x00000000FFFFFFFFULL,

    // TODO: Move to private namespace
    UnknownImplicitTag = kSpecialTagMarker | 0x00000000FFFFFFFEULL
};

/**
 * Returns the profile id from a TLV tag
 *
 * @note The behavior of this function is undefined if the supplied tag is not a profile-specific tag.
 *
 * @param[in]   tag             The API representation of a profile-specific TLV tag.
 * @return                      The profile id.
 */
inline uint32_t ProfileIdFromTag(uint64_t tag)
{
    return static_cast<uint32_t>((tag & kProfileIdMask) >> kProfileIdShift);
}

/**
 * Returns the profile number from a TLV tag
 *
 * @note The behavior of this function is undefined if the supplied tag is not a profile-specific tag.
 *
 * @param[in]   tag             The API representation of a profile-specific TLV tag.
 * @return                      The associated profile number.
 */
inline uint16_t ProfileNumFromTag(uint64_t tag)
{
    return static_cast<uint16_t>((tag & kProfileIdMask) >> kProfileIdShift);
}

/**
 * Returns the tag number from a TLV tag
 *
 * @note The behavior of this function is undefined if the supplied tag is not a profile-specific
 * or context-specific tag.
 *
 * @sa IsProfileTag() and IsContextTag()
 *
 * @param[in]   tag             The API representation of a profile-specific or context-specific TLV tag.
 * @return                      The associated tag number.
 */
inline uint32_t TagNumFromTag(uint64_t tag)
{
    return static_cast<uint32_t>(tag & kTagNumMask);
}

/**
 * Returns the vendor id from a TLV tag
 *
 * @note The behavior of this function is undefined if the supplied tag is not a profile-specific tag.
 *
 * @param[in]   tag             The API representation of a profile-specific TLV tag.
 * @return                      The associated vendor id.
 */
inline uint16_t VendorIdFromTag(uint64_t tag)
{
    return static_cast<uint16_t>((tag & kProfileIdMask) >> kVendorIdShift);
}

/**
 * Returns true of the supplied tag is a profile-specific tag.
 */
inline bool IsProfileTag(uint64_t tag)
{
    return (tag & kProfileIdMask) != kSpecialTagMarker;
}

/**
 * Returns true if the supplied tag is a context-specific tag.
 */
inline bool IsContextTag(uint64_t tag)
{
    return (tag & kProfileIdMask) == kSpecialTagMarker && TagNumFromTag(tag) < kContextTagMaxNum;
}

// TODO: move to private namespace
inline bool IsSpecialTag(uint64_t tag)
{
    return (tag & kProfileIdMask) == kSpecialTagMarker;
}

} // namespace TLV
} // namespace chip
