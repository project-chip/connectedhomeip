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

#include "Builder.h"
#include "Parser.h"
#include <app/util/basic-types.h>
#include <lib/core/CHIPCore.h>
#include <lib/core/TLV.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/logging/CHIPLogging.h>

namespace chip {
namespace app {
class ListBuilder : public Builder
{
public:
    /**
     * Init the TLV array container with an particular context tag.
     * Required to implement arrays of arrays, and to test ListBuilder.
     *
     * @param[in]   apWriter    Pointer to the TLVWriter that is encoding the message.
     * @param[in]   aContextTagToUse    A contextTag to use.
     *
     * @return                  CHIP_ERROR codes returned by chip::TLV objects.
     */

    CHIP_ERROR Init(TLV::TLVWriter * const apWriter, const uint8_t aContextTagToUse);
    /**
     * Init the TLV array container with an anonymous tag.
     * Required to implement arrays of arrays, and to test ListBuilder.
     *
     * @param[in]   apWriter    Pointer to the TLVWriter that is encoding the message.
     *
     * @return                  CHIP_ERROR codes returned by chip::TLV objects.
     */
    CHIP_ERROR Init(TLV::TLVWriter * const apWriter);
};
} // namespace app
} // namespace chip
