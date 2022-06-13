/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
 *    All rights reserved.
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
