/**
 *
 *    Copyright (c) 2020-2021 Project CHIP Authors
 *    Copyright (c) 2016-2017 Nest Labs, Inc.
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

#include "ArrayBuilder.h"
#include "ArrayParser.h"
#include "AttributeStatusIB.h"

#include <app/AppConfig.h>
#include <app/util/basic-types.h>
#include <lib/core/CHIPCore.h>
#include <lib/core/TLV.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/logging/CHIPLogging.h>

namespace chip {
namespace app {
namespace AttributeStatusIBs {
class Builder : public ArrayBuilder
{
public:
    /**
     *  @brief Initialize a AttributeStatusIB::Builder for writing into the TLV stream
     *
     *  @return A reference to AttributeStatusIB::Builder
     */
    AttributeStatusIB::Builder & CreateAttributeStatus();

    /**
     *  @brief Mark the end of this AttributeStatusIBs
     *
     *  @return The builder's final status.
     */
    CHIP_ERROR EndOfAttributeStatuses();

private:
    AttributeStatusIB::Builder mAttributeStatus;
};

class Parser : public ArrayParser
{
public:
#if CHIP_CONFIG_IM_PRETTY_PRINT
    CHIP_ERROR PrettyPrint() const;
#endif // CHIP_CONFIG_IM_PRETTY_PRINT
};
} // namespace AttributeStatusIBs
} // namespace app
} // namespace chip
