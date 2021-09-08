/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
 *    Copyright (c) 2015-2017 Nest Labs, Inc.
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

#include <cstdint>
#include <lib/core/CHIPTLVText.hpp>
#include <lib/core/CHIPTLVTypes.h>
#include <lib/core/CHIPTLVUtilities.hpp>
#include <lib/support/CodeUtils.h>

namespace chip {

namespace TLV {

namespace Debug {

static CHIP_ERROR Print(TLV::TLVReader & reader, PrinterFunc printerFunc, int tabLevel, bool isFirst);

CHIP_ERROR Print(TLV::TLVReader & reader, PrinterFunc printerFunc)
{
    return Print(reader, printerFunc, 0, true);
}

void PrintTabs(PrinterFunc printerFunc, int tabLevel)
{
    for (int i = 0; i < tabLevel; i++)
    {
        printerFunc("    ");
    }
}

/*
 * Prints out the tag along with an '=' symbol if non anonymous tag is present
 *
 * Returns true if the tag was printed out, false if it wasn't.
 */
bool PrintTag(PrinterFunc printerFunc, uint64_t tag, int tabLevel, bool isFirst)
{
    if (!isFirst)
    {
        printerFunc(",\n");
    }

    PrintTabs(printerFunc, tabLevel);

    if (IsContextTag(tag))
    {
        printerFunc("%" PRIu32 " = ", TagNumFromTag(tag));
        return true;
    }
    else if (IsProfileTag(tag))
    {
        printerFunc("0x%" PRIx16 "::0x%" PRIx16 ":0x%" PRIx16 " = ", VendorIdFromTag(tag), ProfileNumFromTag(tag),
                    TagNumFromTag(tag));
        return true;
    }

    return false;
}

void StdoutPrinter(const char * aFormat, ...)
{
    va_list args;

    va_start(args, aFormat);
    vprintf(aFormat, args);
    va_end(args);
}

CHIP_ERROR Print(TLV::TLVReader & reader, PrinterFunc printerFunc, int tabLevel, bool isFirst)
{
    CHIP_ERROR err  = CHIP_NO_ERROR;
    bool tagPrinted = false;

    if (tabLevel == 0)
    {
        err = reader.Next();
        SuccessOrExit(err);
    }

    tagPrinted = PrintTag(printerFunc, reader.GetTag(), tabLevel, isFirst);

    switch (reader.GetType())
    {
    case kTLVType_Structure:
    case kTLVType_Array:
    case kTLVType_List: {
        TLVType containerType;
        const char *openingBrace, *closingBrace;

        if (reader.GetType() == kTLVType_Structure)
        {
            openingBrace = "{";
            closingBrace = "}";
        }
        else if (reader.GetType() == kTLVType_Array)
        {
            openingBrace = "[";
            closingBrace = "]";
        }
        else
        {
            openingBrace = "[[";
            closingBrace = "]]";
        }

        if (tagPrinted)
        {
            printerFunc("\n");
            PrintTabs(printerFunc, tabLevel);
        }

        printerFunc("%s\n", openingBrace);

        err = reader.EnterContainer(containerType);
        SuccessOrExit(err);

        isFirst = true;
        while ((err = reader.Next()) == CHIP_NO_ERROR)
        {
            err = Print(reader, printerFunc, tabLevel + 1, isFirst);
            SuccessOrExit(err);
            isFirst = false;
        }

        err = reader.ExitContainer(containerType);
        SuccessOrExit(err);

        printerFunc("\n");
        PrintTabs(printerFunc, tabLevel);
        printerFunc("%s", closingBrace);

        if (tabLevel == 0)
        {
            printerFunc("\n");
        }

        break;
    }

    case kTLVType_Boolean: {
        bool v;

        err = reader.Get(v);
        SuccessOrExit(err);

        printerFunc("%s", (v) ? "true" : "false");
        break;
    }

    case kTLVType_SignedInteger: {
        int64_t v;

        err = reader.Get(v);
        SuccessOrExit(err);

        printerFunc("%" PRId64, v);
        break;
    }

    case kTLVType_FloatingPointNumber: {
        double f;

        err = reader.Get(f);
        SuccessOrExit(err);

        printerFunc("%.4f", f);
        break;
    }

    case kTLVType_UnsignedInteger: {
        uint64_t v;

        err = reader.Get(v);
        SuccessOrExit(err);

        printerFunc("%" PRIu64, v);
        break;
    }

    case kTLVType_ByteString: {
        chip::ByteSpan span;

        err = reader.Get(span);
        SuccessOrExit(err);

        for (uint32_t i = 0; i < span.size(); i++)
        {
            if (i != 0)
            {
                printerFunc(" ");
            }

            printerFunc("%" PRIx8 "", span.data()[i]);
        }

        break;
    }

    case kTLVType_UTF8String: {
        chip::ByteSpan span;

        err = reader.Get(span);
        SuccessOrExit(err);

        printerFunc("\"");

        for (uint32_t i = 0; i < span.size(); i++)
        {
            printerFunc("%c", span.data()[i]);
        }

        printerFunc("\"");
        break;
    }

    case kTLVType_Null: {
        printerFunc("null");
        break;
    }

    default: {
        printerFunc("?");
        break;
    }
    }

exit:
    return err;
}

} // namespace Debug

} // namespace TLV

} // namespace chip
