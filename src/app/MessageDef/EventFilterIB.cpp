/**
 *
 * SPDX-FileCopyrightText: 2021 Project CHIP Authors
 * SPDX-License-Identifier: Apache-2.0
 */
/**
 *    @file
 *      This file defines EventFilterIB parser and builder in CHIP interaction model
 *
 */

#include "EventFilterIB.h"

#include "MessageDefHelper.h"

#include <inttypes.h>
#include <stdarg.h>
#include <stdio.h>

#include <app/AppConfig.h>

namespace chip {
namespace app {
#if CHIP_CONFIG_IM_PRETTY_PRINT
CHIP_ERROR EventFilterIB::Parser::PrettyPrint() const
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    TLV::TLVReader reader;

    PRETTY_PRINT("EventFilterIB =");
    PRETTY_PRINT("{");

    // make a copy of the reader
    reader.Init(mReader);

    while (CHIP_NO_ERROR == (err = reader.Next()))
    {
        if (!TLV::IsContextTag(reader.GetTag()))
        {
            continue;
        }
        uint32_t tagNum = TLV::TagNumFromTag(reader.GetTag());
        switch (tagNum)
        {
        case to_underlying(Tag::kNode):
#if CHIP_DETAIL_LOGGING
        {
            NodeId node;
            ReturnErrorOnFailure(reader.Get(node));
            PRETTY_PRINT("\tNode = 0x%" PRIx64 ",", node);
        }
#endif // CHIP_DETAIL_LOGGING
        break;
        case to_underlying(Tag::kEventMin):
#if CHIP_DETAIL_LOGGING
        {
            uint64_t eventMin;
            ReturnErrorOnFailure(reader.Get(eventMin));
            PRETTY_PRINT("\tEventMin = 0x%" PRIx64 ",", eventMin);
        }
#endif // CHIP_DETAIL_LOGGING
        break;
        default:
            PRETTY_PRINT("Unknown tag num %" PRIu32, tagNum);
            break;
        }
    }

    PRETTY_PRINT("},");
    PRETTY_PRINT_BLANK_LINE();

    if (CHIP_END_OF_TLV == err)
    {
        err = CHIP_NO_ERROR;
    }

    ReturnErrorOnFailure(err);
    return reader.ExitContainer(mOuterContainerType);
}
#endif // CHIP_CONFIG_IM_PRETTY_PRINT

CHIP_ERROR EventFilterIB::Parser::GetNode(NodeId * const apNode) const
{
    return GetUnsignedInteger(to_underlying(Tag::kNode), apNode);
}

CHIP_ERROR EventFilterIB::Parser::GetEventMin(uint64_t * const apEventMin) const
{
    return GetUnsignedInteger(to_underlying(Tag::kEventMin), apEventMin);
}

EventFilterIB::Builder & EventFilterIB::Builder::Node(const NodeId aNode)
{
    // skip if error has already been set
    if (mError == CHIP_NO_ERROR)
    {
        mError = mpWriter->Put(TLV::ContextTag(Tag::kNode), aNode);
    }
    return *this;
}

EventFilterIB::Builder & EventFilterIB::Builder::EventMin(const uint64_t aEventMin)
{
    // skip if error has already been set
    if (mError == CHIP_NO_ERROR)
    {
        mError = mpWriter->Put(TLV::ContextTag(Tag::kEventMin), aEventMin);
    }
    return *this;
}

CHIP_ERROR EventFilterIB::Builder::EndOfEventFilterIB()
{
    EndOfContainer();
    return GetError();
}
}; // namespace app
}; // namespace chip
