/*
 *   Copyright (c) 2023 Project CHIP Authors
 *   All rights reserved.
 *
 *   Licensed under the Apache License, Version 2.0 (the "License");
 *   you may not use this file except in compliance with the License.
 *   You may obtain a copy of the License at
 *
 *       http://www.apache.org/licenses/LICENSE-2.0
 *
 *   Unless required by applicable law or agreed to in writing, software
 *   distributed under the License is distributed on an "AS IS" BASIS,
 *   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *   See the License for the specific language governing permissions and
 *   limitations under the License.
 *
 */

#pragma once

#include <lib/core/CHIPError.h>
#include <lib/core/DataModelTypes.h>
#include <lib/core/NodeId.h>
#include <lib/support/Span.h>

namespace chip {
namespace bdx {

/**
 * A proxy object to control the state of a specific BDX transfer.
 */

class BDXTransferProxy
{
public:
    virtual ~BDXTransferProxy(){};

    virtual CHIP_ERROR Accept()                 = 0;
    virtual CHIP_ERROR Reject(CHIP_ERROR error) = 0;
    virtual CHIP_ERROR Continue()               = 0;

    virtual CharSpan GetFileDesignator() const = 0;
    virtual FabricIndex GetFabricIndex() const = 0;
    virtual NodeId GetPeerNodeId() const       = 0;
};

} // namespace bdx
} // namespace chip
