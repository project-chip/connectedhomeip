/* See Project CHIP LICENSE file for licensing information. */

/**
 *    @file
 *      This file defines AttributeDataVersionList parser and builder in CHIP interaction model
 *
 */

#include "AttributeDataVersionList.h"

#include "MessageDefHelper.h"

#include <inttypes.h>
#include <stdarg.h>
#include <stdio.h>

using namespace chip;
using namespace chip::TLV;

namespace chip {
namespace app {
#if CHIP_CONFIG_IM_ENABLE_SCHEMA_CHECK
CHIP_ERROR AttributeDataVersionList::Parser::CheckSchemaValidity() const
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    chip::TLV::TLVReader reader;
    chip::DataVersion version;
    size_t index = 0;

    PRETTY_PRINT("AttributeDataVersionList = ");
    PRETTY_PRINT("[");

    reader.Init(mReader);

    while (CHIP_NO_ERROR == (err = reader.Next()))
    {
        VerifyOrExit(chip::TLV::AnonymousTag == reader.GetTag(), err = CHIP_ERROR_INVALID_TLV_TAG);

        switch (reader.GetType())
        {
        case chip::TLV::kTLVType_Null:
            PRETTY_PRINT("\tNull,");
            break;

        case chip::TLV::kTLVType_UnsignedInteger:
            err = reader.Get(version);
            SuccessOrExit(err);

            PRETTY_PRINT("\t0x%" PRIx64 ",", version);
            break;

        default:
            ExitNow(err = CHIP_ERROR_WRONG_TLV_TYPE);
            break;
        }

        ++index;
    }

    PRETTY_PRINT("],");

    if (CHIP_END_OF_TLV == err)
    {
        err = CHIP_NO_ERROR;
    }

exit:
    ChipLogFunctError(err);

    return err;
}
#endif // CHIP_CONFIG_IM_ENABLE_SCHEMA_CHECK

// 1) current element is anonymous
// 2) current element is either unsigned integer or NULL
bool AttributeDataVersionList::Parser::IsElementValid(void)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    bool result    = false;

    VerifyOrExit(chip::TLV::AnonymousTag == mReader.GetTag(), err = CHIP_ERROR_INVALID_TLV_TAG);

    switch (mReader.GetType())
    {
    case chip::TLV::kTLVType_Null:
    case chip::TLV::kTLVType_UnsignedInteger:
        result = true;
        break;
    default:
        ExitNow();
        break;
    }

exit:
    ChipLogFunctError(err);

    return result;
}

bool AttributeDataVersionList::Parser::IsNull(void)
{
    return (chip::TLV::kTLVType_Null == mReader.GetType());
}

CHIP_ERROR AttributeDataVersionList::Parser::GetVersion(chip::DataVersion * const apVersion)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    if (mReader.GetType() == kTLVType_Null)
    {
        *apVersion = 0;
        ChipLogDetail(DataManagement, "Version is null in GetVersion");
    }
    else
    {
        err = mReader.Get(*apVersion);
    }
    return err;
}

AttributeDataVersionList::Builder & AttributeDataVersionList::Builder::AddVersion(const uint64_t aVersion)
{
    // skip if error has already been set
    SuccessOrExit(mError);

    mError = mpWriter->Put(chip::TLV::AnonymousTag, aVersion);
    ChipLogFunctError(mError);

exit:
    return *this;
}

AttributeDataVersionList::Builder & AttributeDataVersionList::Builder::AddNull(void)
{
    // skip if error has already been set
    SuccessOrExit(mError);

    mError = mpWriter->PutNull(chip::TLV::AnonymousTag);
    ChipLogFunctError(mError);

exit:
    return *this;
}

// Mark the end of this array and recover the type for outer container
AttributeDataVersionList::Builder & AttributeDataVersionList::Builder::EndOfAttributeDataVersionList(void)
{
    EndOfContainer();
    return *this;
}
}; // namespace app
}; // namespace chip
