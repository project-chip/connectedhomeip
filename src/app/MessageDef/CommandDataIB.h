/**
 *
 *    Copyright (c) 2020 Project CHIP Authors
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

#include "CommandPathIB.h"
#include "StatusIB.h"
#include "StructBuilder.h"
#include "StructParser.h"

#include <app/AppBuildConfig.h>
#include <app/util/basic-types.h>
#include <lib/core/CHIPCore.h>
#include <lib/core/TLV.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/logging/CHIPLogging.h>

namespace chip {
namespace app {
namespace CommandDataIB {
enum class Tag : uint8_t
{
    kPath   = 0,
    kFields = 1,
};

class Parser : public StructParser
{
public:
#if CHIP_CONFIG_IM_PRETTY_PRINT
    CHIP_ERROR PrettyPrint() const;
#endif // CHIP_CONFIG_IM_PRETTY_PRINT

    /**
     *  @brief Get a TLVReader for the CommandPathIB. Next() must be called before accessing them.
     *
     *  @param [in] apPath    A pointer to apCommandPath
     *
     *  @return #CHIP_NO_ERROR on success
     *          #CHIP_ERROR_WRONG_TLV_TYPE if there is such element but it's not a Path
     *          #CHIP_END_OF_TLV if there is no such element
     */
    CHIP_ERROR GetPath(CommandPathIB::Parser * const apPath) const;

    /**
     *  @brief Get a TLVReader for the Fields. Next() must be called before accessing them.
     *
     *  @param [in] apReader    A pointer to apReader
     *
     *  @return #CHIP_NO_ERROR on success
     *          #CHIP_END_OF_TLV if there is no such element
     */
    CHIP_ERROR GetFields(TLV::TLVReader * const apReader) const;
};

class Builder : public StructBuilder
{
public:
    /**
     *  @brief Initialize a CommandPathIB::Builder for writing into the TLV stream
     *
     *  @return A reference to CommandPathIB::Builder
     */
    CommandPathIB::Builder & CreatePath();

    /**
     *  @brief Mark the end of this CommandDataIB
     *
     *  @return A reference to *this
     */
    CommandDataIB::Builder & EndOfCommandDataIB();

private:
    CommandPathIB::Builder mPath;
};
} // namespace CommandDataIB
} // namespace app
} // namespace chip
