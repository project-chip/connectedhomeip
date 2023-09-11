/*
 *
 * SPDX-FileCopyrightText: 2021 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <lib/core/NodeId.h>
#include <lib/support/Span.h>

namespace chip {
namespace bdx {

constexpr const char kScheme[] = "bdx://";

/**
 * Parses the URI into NodeId and File Designator
 */
CHIP_ERROR ParseURI(CharSpan uri, NodeId & nodeId, CharSpan & fileDesignator);

/**
 * Creates a null-terminated URI in the form of bdx://<16-char-node-id>/<image-file-designator>
 *
 * The resulting size of the span will not include the null terminator.
 */
CHIP_ERROR MakeURI(NodeId nodeId, CharSpan fileDesignator, MutableCharSpan & uri);

} // namespace bdx
} // namespace chip
