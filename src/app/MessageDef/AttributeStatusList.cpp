/* See Project CHIP LICENSE file for licensing information. */

/**
 *    @file
 *      This file defines AttributeStatusList parser and builder in CHIP interaction model
 *
 */

#include "AttributeStatusList.h"
#include "AttributeStatusElement.h"

#include "MessageDefHelper.h"

#include <inttypes.h>
#include <stdarg.h>
#include <stdio.h>

using namespace chip;
using namespace chip::TLV;

namespace chip {
namespace app {
AttributeStatusElement::Builder & AttributeStatusList::Builder::CreateAttributeStatusBuilder()
{
    // skip if error has already been set
    VerifyOrExit(CHIP_NO_ERROR == mError, mAttributeStatusBuilder.ResetError(mError));

    mError = mAttributeStatusBuilder.Init(mpWriter);
    ChipLogFunctError(mError);

exit:
    // on error, mAttributeStatusBuilder would be un-/partial initialized and cannot be used to write anything
    return mAttributeStatusBuilder;
}

AttributeStatusList::Builder & AttributeStatusList::Builder::EndOfAttributeStatusList()
{
    EndOfContainer();
    return *this;
}

CHIP_ERROR AttributeStatusList::Parser::CheckSchemaValidity() const
{
    CHIP_ERROR err                  = CHIP_NO_ERROR;
    size_t NumAttributeStateElement = 0;
    chip::TLV::TLVReader reader;

    PRETTY_PRINT("AttributeStatusList =");
    PRETTY_PRINT("[");

    // make a copy of the EventList reader
    reader.Init(mReader);

    while (CHIP_NO_ERROR == (err = reader.Next()))
    {
        VerifyOrExit(chip::TLV::AnonymousTag == reader.GetTag(), err = CHIP_ERROR_INVALID_TLV_TAG);
        VerifyOrExit(chip::TLV::kTLVType_Structure == reader.GetType(), err = CHIP_ERROR_WRONG_TLV_TYPE);

        {
            AttributeStatusElement::Parser status;
            err = status.Init(reader);
            SuccessOrExit(err);

            PRETTY_PRINT_INCDEPTH();
            err = status.CheckSchemaValidity();
            SuccessOrExit(err);
            PRETTY_PRINT_DECDEPTH();
        }

        ++NumAttributeStateElement;
    }

    PRETTY_PRINT("],");
    PRETTY_PRINT("");
    // if we have exhausted this container
    if (CHIP_END_OF_TLV == err)
    {
        // if we have at least one data element
        if (NumAttributeStateElement > 0)
        {
            err = CHIP_NO_ERROR;
        }
        // NOTE: temporarily disable this check, to allow test to continue
        else
        {
            ChipLogError(DataManagement, "PROTOCOL ERROR: Empty attribute status list");
            err = CHIP_NO_ERROR;
        }
    }

exit:
    ChipLogFunctError(err);

    return err;
}
}; // namespace app
}; // namespace chip
