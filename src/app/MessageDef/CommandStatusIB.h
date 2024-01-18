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

#include "CommandPathIB.h"
#include "StructBuilder.h"

#include "StatusIB.h"
#include "StructParser.h"

#include <app/AppConfig.h>
#include <app/util/basic-types.h>
#include <lib/core/CHIPCore.h>
#include <lib/core/TLV.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/logging/CHIPLogging.h>

namespace chip {
namespace app {
namespace CommandStatusIB {
enum class Tag : uint8_t
{
    kPath        = 0,
    kErrorStatus = 1,
    kRef         = 2,
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
     *  @param [in] apPath    A pointer to apPath
     *
     *  @return #CHIP_NO_ERROR on success
     *          #CHIP_ERROR_WRONG_TLV_TYPE if there is such element but it's not a Path
     *          #CHIP_END_OF_TLV if there is no such element
     */
    CHIP_ERROR GetPath(CommandPathIB::Parser * const apPath) const;

    /**
     *  @brief Get a TLVReader for the StatusIB. Next() must be called before accessing them.
     *
     *  @param [in] apErrorStatus    A pointer to apErrorStatus
     *
     *  @return #CHIP_NO_ERROR on success
     *          # CHIP_ERROR_WRONG_TLV_TYPE if there is such element but it's not a structure
     *          #CHIP_END_OF_TLV if there is no such element
     */
    CHIP_ERROR GetErrorStatus(StatusIB::Parser * const apErrorStatus) const;

    /**
     *  @brief Get the provided command reference associated with the CommandStatus
     *
     *  @param [out] apRef    A pointer to apRef
     *
     *  @return #CHIP_NO_ERROR on success
     *          #CHIP_ERROR_WRONG_TLV_TYPE if there is such element but it's not any of the defined unsigned integer types
     *          #CHIP_END_OF_TLV if there is no such element
     */
    CHIP_ERROR GetRef(uint16_t * const apRef) const;
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
     *  @brief Initialize a StatusIB::Builder for writing into the TLV stream
     *
     *  @return A reference to StatusIB::Builder
     */
    StatusIB::Builder & CreateErrorStatus();

    /**
     *  @brief Inject Command Ref into the TLV stream.
     *
     *  @param [in] aRef refer to the CommandRef to set in CommandStatusIB.
     *
     *  @return #CHIP_NO_ERROR on success
     */
    CHIP_ERROR Ref(const uint16_t aRef);

    /**
     *  @brief Mark the end of this CommandStatusIB
     *
     *  @return The builder's final status.
     */
    CHIP_ERROR EndOfCommandStatusIB();

private:
    CommandPathIB::Builder mPath;
    StatusIB::Builder mErrorStatus;
};
} // namespace CommandStatusIB
} // namespace app
} // namespace chip
