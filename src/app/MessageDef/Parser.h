/**
 *
 * SPDX-FileCopyrightText: 2020 Project CHIP Authors
 * SPDX-FileCopyrightText: 2016-2017 Nest Labs, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */
/**
 *    @file
 *      This file defines parser in CHIP interaction model
 *
 */

#pragma once

#include <app/data-model/Nullable.h>
#include <app/util/basic-types.h>
#include <lib/core/CHIPCore.h>
#include <lib/core/TLV.h>
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
    CHIP_ERROR GetReaderOnTag(const TLV::Tag aTagToFind, TLV::TLVReader * const apReader) const;

    /**
     *  @brief Get the TLV Reader
     *
     *  @param [in] apReader A pointer to a TLVReader
     *
     */
    void GetReader(chip::TLV::TLVReader * const apReader);

    /**
     *  @brief Iterate to next element
     *
     *  @return #CHIP_NO_ERROR on success
     */
    CHIP_ERROR Next();

protected:
    chip::TLV::TLVReader mReader;
    chip::TLV::TLVType mOuterContainerType;
    Parser();

    /**
     * Gets a unsigned integer value with the given tag, the value is not touched when the tag is not found in the TLV.
     *
     *  @return #CHIP_NO_ERROR on success
     *          #CHIP_ERROR_WRONG_TLV_TYPE if there is such element but it's not any of the defined unsigned integer types
     *          #CHIP_END_OF_TLV if there is no such element
     */
    template <typename T>
    CHIP_ERROR GetUnsignedInteger(const uint8_t aContextTag, T * const apLValue) const
    {
        return GetSimpleValue(aContextTag, chip::TLV::kTLVType_UnsignedInteger, apLValue);
    };

    /**
     * Gets a unsigned integer or null value with the given tag, the value is not touched when the tag is not found in the TLV.
     *
     *  @return #CHIP_NO_ERROR on success
     *          #CHIP_ERROR_WRONG_TLV_TYPE if there is such element but it's not any of the defined unsigned integer types and is
     *                                     not null
     *          #CHIP_END_OF_TLV if there is no such element
     */
    template <typename T>
    CHIP_ERROR GetNullableUnsignedInteger(const uint8_t aContextTag, DataModel::Nullable<T> * const apLValue) const
    {
        return GetSimpleNullableValue(aContextTag, chip::TLV::kTLVType_UnsignedInteger, apLValue);
    };

    /**
     * Gets a scalar value with the given tag, the value is not touched when the tag is not found in the TLV.
     *
     *  @return #CHIP_NO_ERROR on success
     *          #CHIP_ERROR_WRONG_TLV_TYPE if there is such element but it's not any of the defined unsigned integer types
     *          #CHIP_END_OF_TLV if there is no such element
     */
    template <typename T>
    CHIP_ERROR GetSimpleValue(const uint8_t aContextTag, const chip::TLV::TLVType aTLVType, T * const apLValue) const
    {
        CHIP_ERROR err = CHIP_NO_ERROR;
        chip::TLV::TLVReader reader;

        err = mReader.FindElementWithTag(chip::TLV::ContextTag(aContextTag), reader);
        SuccessOrExit(err);

        *apLValue = 0;

        VerifyOrExit(aTLVType == reader.GetType(), err = CHIP_ERROR_WRONG_TLV_TYPE);

        err = reader.Get(*apLValue);
        SuccessOrExit(err);

    exit:
        ChipLogIfFalse((CHIP_NO_ERROR == err) || (CHIP_END_OF_TLV == err));

        return err;
    };

    /**
     * Gets a scalar value with the given tag, the value is not touched when the tag is not found in the TLV.
     *
     *  @return #CHIP_NO_ERROR on success
     *          #CHIP_ERROR_WRONG_TLV_TYPE if there is such element but it's not any of the defined unsigned integer types
     *          #CHIP_END_OF_TLV if there is no such element
     */
    template <typename T>
    CHIP_ERROR GetSimpleNullableValue(const uint8_t aContextTag, const chip::TLV::TLVType aTLVType,
                                      DataModel::Nullable<T> * const apLValue) const
    {
        CHIP_ERROR err = CHIP_NO_ERROR;
        chip::TLV::TLVReader reader;

        err = mReader.FindElementWithTag(chip::TLV::ContextTag(aContextTag), reader);
        SuccessOrExit(err);

        apLValue->SetNull();

        VerifyOrExit(aTLVType == reader.GetType() || TLV::TLVType::kTLVType_Null == reader.GetType(),
                     err = CHIP_ERROR_WRONG_TLV_TYPE);

        if (reader.GetType() == aTLVType)
        {
            T value;
            err = reader.Get(value);
            SuccessOrExit(err);
            apLValue->SetNonNull(value);
        }

    exit:
        ChipLogIfFalse((CHIP_NO_ERROR == err) || (CHIP_END_OF_TLV == err));

        return err;
    };
};
} // namespace app
} // namespace chip
