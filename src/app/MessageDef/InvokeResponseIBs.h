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
#include "InvokeResponseIB.h"

namespace chip {
namespace app {
namespace InvokeResponseIBs {
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
     *  EndOfInvokeResponses() with underlying TLVWriter.
     */
    CHIP_ERROR InitWithEndBufferReserved(TLV::TLVWriter * const apWriter, const uint8_t aContextTagToUse);

    /**
     *  @brief Initialize a InvokeResponseIB::Builder for writing into the TLV stream
     *
     *  @return A reference to InvokeResponseIB::Builder
     */
    InvokeResponseIB::Builder & CreateInvokeResponse();

    /**
     *  @return A reference to InvokeResponseIB::Builder
     */
    InvokeResponseIB::Builder & GetInvokeResponse() { return mInvokeResponse; };

    /**
     *  @brief Mark the end of this InvokeResponseIBs
     *
     *  @return The builder's final status.
     */
    CHIP_ERROR EndOfInvokeResponses();

    /**
     *  @brief Get number of bytes required in the buffer by EndOfInvokeResponses()
     *
     *  @return Expected number of bytes required in the buffer by EndOfInvokeResponses()
     */
    uint32_t GetSizeToEndInvokeResponses();

private:
    InvokeResponseIB::Builder mInvokeResponse;
    bool mIsEndBufferReserved = false;
};
} // namespace InvokeResponseIBs
} // namespace app
} // namespace chip
