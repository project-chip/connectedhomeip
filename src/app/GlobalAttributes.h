/*
 * SPDX-FileCopyrightText: 2022-2023 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <app-common/zap-generated/ids/Attributes.h>
#include <lib/support/CodeUtils.h>

namespace chip {
namespace app {

/**
 * List of attribute ids of attributes that appear on every cluster and have
 * values that are always produced via code, hence do not appear in attribute
 * metadata to save space.  These _must_ appear in order.
 */
constexpr AttributeId GlobalAttributesNotInMetadata[] = {
    Clusters::Globals::Attributes::GeneratedCommandList::Id,
    Clusters::Globals::Attributes::AcceptedCommandList::Id,
#if CHIP_CONFIG_ENABLE_EVENTLIST_ATTRIBUTE
    Clusters::Globals::Attributes::EventList::Id,
#endif // CHIP_CONFIG_ENABLE_EVENTLIST_ATTRIBUTE
    Clusters::Globals::Attributes::AttributeList::Id,
};

static_assert(ArrayIsSorted(GlobalAttributesNotInMetadata), "Array of global attribute ids must be sorted");

} // namespace app
} // namespace chip
