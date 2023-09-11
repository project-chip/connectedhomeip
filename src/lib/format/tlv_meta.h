/*
 *
 * SPDX-FileCopyrightText: 2023 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#pragma once

#include <lib/core/TLVTags.h>

namespace chip {
namespace TLVMeta {

static constexpr uint32_t kAttributeProfile = 1;
static constexpr uint32_t kCommandProfile   = 2;
static constexpr uint32_t kEventProfile     = 3;

constexpr TLV::Tag ClusterTag(uint32_t cluster_id)
{
    return TLV::CommonTag(cluster_id);
}

constexpr TLV::Tag AttributeTag(uint32_t attribute_id)
{
    return TLV::ProfileTag(kAttributeProfile, attribute_id);
}

constexpr TLV::Tag CommandTag(uint32_t command_id)
{
    return TLV::ProfileTag(kCommandProfile, command_id);
}

constexpr TLV::Tag EventTag(uint32_t event_id)
{
    return TLV::ProfileTag(kEventProfile, event_id);
}

constexpr TLV::Tag ConstantValueTag(uint64_t value)
{
    // Re-use common tag for a constant value
    // Will make "RawValue be equal to value"
    return TLV::ProfileTag(static_cast<uint32_t>(value >> 32), static_cast<uint32_t>(value & 0xFFFFFFFF));
}

enum class ItemType : uint8_t
{
    kDefault,
    kList,
    kEnum,
    kBitmap,

    // Special protocol types
    kProtocolClusterId,
    kProtocolAttributeId,
    kProtocolCommandId,
    kProtocolEventId,

    kProtocolPayloadAttribute,
    kProtocolPayloadCommand,
    kProtocolPayloadEvent,

    kProtocolBinaryData,
};

struct ItemInfo
{
    TLV::Tag tag;
    const char * name;
    ItemType type;
};

} // namespace TLVMeta
} // namespace chip
