/*
 *
 * SPDX-FileCopyrightText: 2021 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "AuthMode.h"

#include <lib/core/CASEAuthTag.h>
#include <lib/core/DataModelTypes.h>
#include <lib/core/NodeId.h>

namespace chip {
namespace Access {

struct SubjectDescriptor
{
    // Holds FabricIndex of fabric, 0 if no fabric.
    FabricIndex fabricIndex = kUndefinedFabricIndex;

    // Holds AuthMode of subject(s), kNone if no access.
    AuthMode authMode = AuthMode::kNone;

    // NOTE: due to packing there should be free bytes here

    // Holds subject according to auth mode.
    NodeId subject = kUndefinedNodeId;

    // CASE Authenticated Tags (CATs) only valid if auth mode is CASE.
    CATValues cats;
};

} // namespace Access
} // namespace chip
