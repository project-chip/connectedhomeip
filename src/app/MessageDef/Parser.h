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
/**
 *    @file
 *      This file defines parser in CHIP interaction model
 *
 */

#pragma once

#include <app/util/basic-types.h>
#include <lib/core/CHIPCore.h>
#include <lib/core/CHIPTLV.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/logging/CHIPLogging.h>

namespace chip {
namespace app {
class Parser
{
public:
    /**
     *  @brief Initialize the Builder object with TLVReader and ContainerType
     *
     *  @param [in] aReader TLVReader
     *  @param [in] aOuterContainerType outer container type
     *
     */
    void Init(const chip::TLV::TLVReader & aReader, chip::TLV::TLVType aOuterContainerType);

    /**
     *  @brief Initialize a TLVReader to point to the beginning of any tagged element in this request
     *
     *  @param [in]  aTagToFind Tag to find in the request
     *  @param [out] apReader   A pointer to TLVReader, which will be initialized at the specified TLV element
     *                          on success
     *
     *  @return #CHIP_NO_ERROR on success
     */
    CHIP_ERROR GetReaderOnTag(const uint64_t aTagToFind, chip::TLV::TLVReader * const apReader) const;

    /**
     *  @brief Get the TLV Reader
     *
     *  @param [in] apReader A pointer to a TLVReader
     *
     */
    void GetReader(chip::TLV::TLVReader * const apReader);

protected:
    chip::TLV::TLVReader mReader;
    chip::TLV::TLVType mOuterContainerType;
    Parser();

    template <typename T>
    CHIP_ERROR GetUnsignedInteger(const uint8_t aContextTag, T * const apLValue) const
    {
        return GetSimpleValue(aContextTag, chip::TLV::kTLVType_UnsignedInteger, apLValue);
    };

    template <typename T>
    CHIP_ERROR GetSimpleValue(const uint8_t aContextTag, const chip::TLV::TLVType aTLVType, T * const apLValue) const
    {
        CHIP_ERROR err = CHIP_NO_ERROR;
        chip::TLV::TLVReader reader;

        *apLValue = 0;

        err = mReader.FindElementWithTag(chip::TLV::ContextTag(aContextTag), reader);
        SuccessOrExit(err);

        VerifyOrExit(aTLVType == reader.GetType(), err = CHIP_ERROR_WRONG_TLV_TYPE);

        err = reader.Get(*apLValue);
        SuccessOrExit(err);

    exit:
        ChipLogIfFalse((CHIP_NO_ERROR == err) || (CHIP_END_OF_TLV == err));

        return err;
    };
};
}; // namespace app
}; // namespace chip
