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

#include "BdxTransferProxy.h"

#include <lib/core/CHIPError.h>

namespace chip {
namespace bdx {

class BDXTransferServerDelegate
{
public:
    virtual ~BDXTransferServerDelegate() {}

    /**
     * @brief
     *  This method is invoked when the transfer begins.
     */
    virtual CHIP_ERROR OnTransferBegin(BDXTransferProxy * transfer) = 0;

    /**
     * @brief
     *  This method is invoked when the transfer ends.
     */
    virtual CHIP_ERROR OnTransferEnd(BDXTransferProxy * transfer, CHIP_ERROR error) = 0;

    /**
     * @brief
     *  This method is invoked when a block is received.
     */
    virtual CHIP_ERROR OnTransferData(BDXTransferProxy * transfer, const ByteSpan & data) = 0;
};

} // namespace bdx
} // namespace chip
