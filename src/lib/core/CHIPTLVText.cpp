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

/**
 *    @file
 *      This file implements utility interfaces for managing and
 *      working with CHIP TLV.
 *
 */

#include <core/CHIPTLVDebug.hpp>
#include <core/CHIPTLVUtilities.hpp>
#include <support/CodeUtils.h>
#include <core/CHIPTLVTypes.h>
#include <cstdint>

namespace chip {

namespace TLV {

namespace Utilities {

void PrintTabs(int tabLevel)
{
    for (int i = 0; i < tabLevel; i++) {
        printf("\t");
    }
}

void PrintTag(uint64_t tag, int tabLevel)
{
    PrintTabs(tabLevel);

    if (IsContextTag(tag)) {
        printf("%" PRIu32 " = ", TagNumFromTag(tag));
    }
    else if (IsProfileTag(tag)) {
        printf("0x%" PRIx16 "::0x%" PRIx16 ":0x%" PRIu32 " = ", VendorIdFromTag(tag), ProfileNumFromTag(tag), TagNumFromTag(tag));
    }
}

#pragma GCC diagnostic ignored "-Wstack-usage="

CHIP_ERROR Print(TLV::TLVReader &reader, int tabLevel)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    PrintTag(reader.GetTag(), tabLevel);

    if (tabLevel == 0) {
        err = reader.Next();
        SuccessOrExit(err);
    }

    switch (reader.GetType()) {
        case kTLVType_Structure:
        case kTLVType_Array:
        case kTLVType_List:
        {
            TLVType containerType;
            const char *openingBrace, *closingBrace;

            if (reader.GetType() == kTLVType_Structure) {
                openingBrace = "{";
                closingBrace = "}";
            }
            else if (reader.GetType() == kTLVType_Array) {
                openingBrace = "[";
                closingBrace = "]";
            }
            else {
                openingBrace = "[[";
                closingBrace = "]]";
            }

            printf("\n");
            PrintTabs(tabLevel);
            printf("%s\n", openingBrace);

            err = reader.EnterContainer(containerType);
            SuccessOrExit(err);

            while ((err = reader.Next()) == CHIP_NO_ERROR) {
                err = Print(reader, tabLevel + 1);
                SuccessOrExit(err);
            }

            err = reader.ExitContainer(containerType);
            SuccessOrExit(err);

            PrintTabs(tabLevel);
            printf("%s\n", closingBrace);
            break;
        }

        case kTLVType_Boolean:
        {
            bool v;

            err = reader.Get(v);
            SuccessOrExit(err);

            printf("%s,\n", (v) ? "true" : "false");
            break;
        }

        case kTLVType_SignedInteger:
        {
            int64_t v;

            err = reader.Get(v);
            SuccessOrExit(err);

            printf("%" PRId64 ",\n", v);
            break;
        }

        case kTLVType_UnsignedInteger:
        {
            uint64_t v;

            err = reader.Get(v);
            SuccessOrExit(err);

            printf("%" PRIu64 ",\n", v);
            break;
        }

        case kTLVType_ByteString:
        {
            uint32_t sz = reader.GetLength();
            uint8_t buf[sz];

            err = reader.GetBytes(buf, sz);
            SuccessOrExit(err);

            for (uint32_t i = 0; i < sz; i++) {
                printf("%" PRIu8 " ", buf[i]);
            }

            printf("\n");
            break;
        }

        case kTLVType_UTF8String:
        {
            uint32_t sz = reader.GetLength();
            char buf[sz + 1];

            err = reader.GetString(buf, sz + 1);
            SuccessOrExit(err);

            buf[sz] = '\0';
            printf("\"%s\"\n", buf);
            break;
        }

        default:
        {
            printf("?,\n");
            break;
        }
    }

exit:
    return err;
}


} // namespace Utilities

} // namespace TLV

} // namespace chip
