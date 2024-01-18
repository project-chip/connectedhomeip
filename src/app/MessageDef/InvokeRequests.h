/**
 *
 *    Copyright (c) 2021 Project CHIP Authors
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

#include <app/AppConfig.h>
#include <app/util/basic-types.h>
#include <lib/core/CHIPCore.h>
#include <lib/core/TLV.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/logging/CHIPLogging.h>

#include "ArrayBuilder.h"
#include "ArrayParser.h"
#include "CommandDataIB.h"

namespace chip {
namespace app {
namespace InvokeRequests {
class Parser : public ArrayParser
{
public:
#if CHIP_CONFIG_IM_PRETTY_PRINT
    CHIP_ERROR PrettyPrint() const;
#endif // CHIP_CONFIG_IM_PRETTY_PRINT
};

class Builder : public ArrayBuilder
{
public:
    /**
     *  @brief Performs underlying StructBuilder::Init, but reserves memory need in
     *  EndOfInvokeRequests() with underlying TLVWriter.
     */
    CHIP_ERROR InitWithEndBufferReserved(TLV::TLVWriter * const apWriter, const uint8_t aContextTagToUse);

    /**
     *  @brief Initialize a CommandDataIB::Builder for writing into the TLV stream
     *
     *  @return A reference to CommandDataIB::Builder
     */
    CommandDataIB::Builder & CreateCommandData();

    /**
     *  @return A reference to CommandDataIB::Builder
     */
    CommandDataIB::Builder & GetCommandData() { return mCommandData; };

    /**
     *  @brief Mark the end of this InvokeRequests
     *
     *  @return The builder's final status.
     */
    CHIP_ERROR EndOfInvokeRequests();

    /**
     *  @brief Get number of bytes required in the buffer by EndOfInvokeRequests()
     *
     *  @return Expected number of bytes required in the buffer by EndOfInvokeRequests()
     */
    uint32_t GetSizeToEndInvokeRequests();

private:
    CommandDataIB::Builder mCommandData;
    bool mIsEndBufferReserved = false;
};
} // namespace InvokeRequests
} // namespace app
} // namespace chip
