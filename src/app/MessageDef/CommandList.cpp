/* See Project CHIP LICENSE file for licensing information. */

/**
 *    @file
 *      This file defines CommandList parser and builder in CHIP interaction model
 *
 */

#include "CommandList.h"

#include "MessageDefHelper.h"

#include <inttypes.h>
#include <stdarg.h>
#include <stdio.h>

using namespace chip;
using namespace chip::TLV;

namespace chip {
namespace app {
#if CHIP_CONFIG_IM_ENABLE_SCHEMA_CHECK
CHIP_ERROR CommandList::Parser::CheckSchemaValidity() const
{
    CHIP_ERROR err     = CHIP_NO_ERROR;
    size_t NumCommands = 0;
    chip::TLV::TLVReader reader;

    PRETTY_PRINT("CommandList =");
    PRETTY_PRINT("[");

    // make a copy of the reader
    reader.Init(mReader);

    while (CHIP_NO_ERROR == (err = reader.Next()))
    {
        VerifyOrExit(chip::TLV::AnonymousTag == reader.GetTag(), err = CHIP_ERROR_INVALID_TLV_TAG);
        VerifyOrExit(chip::TLV::kTLVType_Structure == reader.GetType(), err = CHIP_ERROR_WRONG_TLV_TYPE);

        {
            CommandDataElement::Parser data;
            err = data.Init(reader);
            SuccessOrExit(err);

            PRETTY_PRINT_INCDEPTH();
            err = data.CheckSchemaValidity();
            SuccessOrExit(err);

            PRETTY_PRINT_DECDEPTH();
        }

        ++NumCommands;
    }

    PRETTY_PRINT("],");
    PRETTY_PRINT("");

    // if we have exhausted this container
    if (CHIP_END_OF_TLV == err)
    {
        // if we have at least one data element
        if (NumCommands > 0)
        {
            err = CHIP_NO_ERROR;
        }
    }

exit:
    ChipLogFunctError(err);

    return err;
}
#endif // CHIP_CONFIG_IM_ENABLE_SCHEMA_CHECK

CommandDataElement::Builder & CommandList::Builder::CreateCommandDataElementBuilder()
{
    // skip if error has already been set
    VerifyOrExit(CHIP_NO_ERROR == mError, mCommandDataElementBuilder.ResetError(mError));

    mError = mCommandDataElementBuilder.Init(mpWriter);
    ChipLogFunctError(mError);

exit:
    // on error, mCommandDataElementBuilder would be un-/partial initialized and cannot be used to write anything
    return mCommandDataElementBuilder;
}

CommandList::Builder & CommandList::Builder::EndOfCommandList()
{
    EndOfContainer();
    return *this;
}
}; // namespace app
}; // namespace chip
