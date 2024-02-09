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
#pragma once

#include <lib/support/StringBuilder.h>
#include <lib/support/TypeTraits.h>

#include <cstdint>
#include <type_traits>

namespace chip {
namespace TLV {

class Tag
{
public:
    enum SpecialTagNumber : uint32_t
    {
        kContextTagMaxNum = UINT8_MAX,
        kAnonymousTagNum,
        kUnknownImplicitTagNum
    };

    Tag() = default;

    constexpr bool operator==(const Tag & other) const { return mVal == other.mVal; }
    constexpr bool operator!=(const Tag & other) const { return mVal != other.mVal; }

    /// Appends the text representation of the tag to the given string builder base.
    StringBuilderBase & AppendTo(StringBuilderBase & out);

private:
    explicit constexpr Tag(uint64_t val) : mVal(val) {}

    friend constexpr Tag ProfileTag(uint32_t profileId, uint32_t tagNum);
    friend constexpr Tag ProfileTag(uint16_t vendorId, uint16_t profileNum, uint32_t tagNum);
    friend constexpr Tag ContextTag(uint8_t tagNum);
    friend constexpr Tag CommonTag(uint32_t tagNum);
    friend constexpr Tag AnonymousTag();
    friend constexpr Tag UnknownImplicitTag();

    // The following friend functions could be Tag class methods, but it turns out in some cases
    // they may not be inlined and then passing the tag by argument/value results in smaller code
    // than passing it by 'this' pointer. This can be worked around by applying 'always_inline'
    // function attribute, but friend functions are likely a more portable solution.

    friend constexpr uint32_t ProfileIdFromTag(Tag tag);
    friend constexpr uint16_t VendorIdFromTag(Tag tag);
    friend constexpr uint16_t ProfileNumFromTag(Tag tag);
    friend constexpr uint32_t TagNumFromTag(Tag tag);

    friend constexpr bool IsProfileTag(Tag tag);
    friend constexpr bool IsContextTag(Tag tag);
    friend constexpr bool IsSpecialTag(Tag tag);

    static constexpr uint32_t kProfileIdShift      = 32;
    static constexpr uint32_t kVendorIdShift       = 48;
    static constexpr uint32_t kProfileNumShift     = 32;
    static constexpr uint32_t kSpecialTagProfileId = 0xFFFFFFFF;

    // The storage of the tag value uses the following encoding:
    //
    //  63                              47                              31
    // +-------------------------------+-------------------------------+----------------------------------------------+
    // | Vendor id (bitwise-negated)   | Profile num (bitwise-negated) | Tag number                                   |
    // +-------------------------------+-------------------------------+----------------------------------------------+
    //
    // Vendor id and profile number are bitwise-negated in order to optimize the code size when
    // using context tags, the most commonly used tags in the SDK.
    uint64_t mVal;
};

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
constexpr Tag ProfileTag(uint32_t profileId, uint32_t tagNum)
{
    return Tag((static_cast<uint64_t>(~profileId) << Tag::kProfileIdShift) | tagNum);
}

/**
 * Generates the API representation of a profile-specific TLV tag from a vendor id, profile number and tag number
 *
 * @param[in]   vendorId        The id of the vendor that defined the tag.
 * @param[in]   profileNum      The vendor assigned number for the profile within which the tag is defined.
 * @param[in]   tagNum          The profile-specific tag number assigned to the tag.
 * @return                      A 64-bit integer representing the tag.
 */
constexpr Tag ProfileTag(uint16_t vendorId, uint16_t profileNum, uint32_t tagNum)
{
    constexpr uint32_t kVendorIdShift = Tag::kVendorIdShift - Tag::kProfileIdShift;

    return ProfileTag((static_cast<uint32_t>(vendorId) << kVendorIdShift) | profileNum, tagNum);
}

/**
 * Generates the API representation of a context-specific TLV tag
 *
 * @param[in]   tagNum          The context-specific tag number assigned to the tag.
 * @return                      A 64-bit integer representing the tag.
 */
constexpr Tag ContextTag(uint8_t tagNum)
{
    return ProfileTag(Tag::kSpecialTagProfileId, tagNum);
}

/**
 * Also allow using enum class values as context tags.  There are four possible
 * cases when is_enum is true:
 *
 * 1) The enum is a scoped enum (enum class) and the underlying type is
 *    uint8_t.  Then the is_convertible test will test false, this overload will be
 *    used, to_underlying will return a uint8_t, and everything will work.
 * 2) The enum is a scoped enum (enum class) and the underlying type is
 *    not uint8_t.  Then the is_convertible test will test false, this overload will be
 *    used, to_underlying will return the other type, and -Wconversion will
 *    catch the type mismatch, if it's enabled.
 * 3) The enum is an old-style enum.  Then the is_convertible test will test
 *    true, this overload will be not be used, and the uint8_t overload will be
 *    used instead.  The compiler should then catch (at least with sufficient
 *    warnings turned on) if the constant being passed in is too big to fit into
 *    uint8_t.
 *
 * Leaving out the is_convertible test, so this overload gets used for old-style
 * enums, would cause old-style enums where the underlying type is wider than
 * uint8_t to fail compilation with -Wconversion even if the values are all
 * inside the uint8_t range, since to_underlying would explicitly return a type
 * that is wider than uint8_t.
 */
template <typename T, std::enable_if_t<std::is_enum<T>::value && !std::is_convertible<T, uint8_t>::value, int> = 0>
constexpr Tag ContextTag(T tagNum)
{
    return ContextTag(to_underlying(tagNum));
}

/**
 * Generates the API representation of a common profile TLV tag
 *
 * @param[in]   tagNum          The common profile tag number assigned to the tag.
 * @return                      A 64-bit integer representing the tag.
 */
constexpr Tag CommonTag(uint32_t tagNum)
{
    return ProfileTag(kCommonProfileId, tagNum);
}

/**
 * A value signifying a TLV element that has no tag (i.e. an anonymous element).
 */
constexpr Tag AnonymousTag()
{
    return ProfileTag(Tag::kSpecialTagProfileId, Tag::kAnonymousTagNum);
}

/**
 * An invalid tag that represents a TLV element decoding error due to unknown implicit profile id.
 */
constexpr Tag UnknownImplicitTag()
{
    return ProfileTag(Tag::kSpecialTagProfileId, Tag::kUnknownImplicitTagNum);
}

/**
 * Returns the profile id from a TLV tag
 *
 * @note The behavior of this function is undefined if the supplied tag is not a profile-specific tag.
 *
 * @param[in]   tag             The API representation of a profile-specific TLV tag.
 * @return                      The profile id.
 */
constexpr uint32_t ProfileIdFromTag(Tag tag)
{
    return ~static_cast<uint32_t>(tag.mVal >> Tag::kProfileIdShift);
}

/**
 * Returns the vendor id from a TLV tag
 *
 * @note The behavior of this function is undefined if the supplied tag is not a profile-specific tag.
 *
 * @param[in]   tag             The API representation of a profile-specific TLV tag.
 * @return                      The associated vendor id.
 */
constexpr uint16_t VendorIdFromTag(Tag tag)
{
    constexpr uint32_t kVendorIdShift = Tag::kVendorIdShift - Tag::kProfileIdShift;

    return static_cast<uint16_t>(ProfileIdFromTag(tag) >> kVendorIdShift);
}

/**
 * Returns the profile number from a TLV tag
 *
 * @note The behavior of this function is undefined if the supplied tag is not a profile-specific tag.
 *
 * @param[in]   tag             The API representation of a profile-specific TLV tag.
 * @return                      The associated profile number.
 */
constexpr uint16_t ProfileNumFromTag(Tag tag)
{
    return static_cast<uint16_t>(ProfileIdFromTag(tag));
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
constexpr uint32_t TagNumFromTag(Tag tag)
{
    return static_cast<uint32_t>(tag.mVal);
}

/**
 * Returns true of the supplied tag is a profile-specific tag.
 */
constexpr bool IsProfileTag(Tag tag)
{
    return ProfileIdFromTag(tag) != Tag::kSpecialTagProfileId;
}

/**
 * Returns true if the supplied tag is a context-specific tag.
 */
constexpr bool IsContextTag(Tag tag)
{
    return ProfileIdFromTag(tag) == Tag::kSpecialTagProfileId && TagNumFromTag(tag) <= Tag::kContextTagMaxNum;
}

// TODO: move to private namespace
constexpr bool IsSpecialTag(Tag tag)
{
    return ProfileIdFromTag(tag) == Tag::kSpecialTagProfileId;
}

} // namespace TLV
} // namespace chip
