/*
 *
 * SPDX-FileCopyrightText: 2020 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "QueryResponder.h"

#include <lib/dnssd/minimal_mdns/core/QNameString.h>
#include <lib/dnssd/minimal_mdns/records/Ptr.h>

#include <lib/support/logging/CHIPLogging.h>

namespace mdns {
namespace Minimal {

const QNamePart kDnsSdQueryPath[] = { "_services", "_dns-sd", "_udp", "local" };

QueryResponderBase::QueryResponderBase(Internal::QueryResponderInfo * infos, size_t infoSizes) :
    Responder(QType::PTR, FullQName(kDnsSdQueryPath)), mResponderInfos(infos), mResponderInfoSize(infoSizes)
{}

void QueryResponderBase::Init()
{
    for (size_t i = 0; i < mResponderInfoSize; i++)
    {
        mResponderInfos[i].Clear();
    }

    if (mResponderInfoSize > 0)
    {
        // reply to queries about services available
        mResponderInfos[0].responder = this;
    }

    if (mResponderInfoSize < 2)
    {
        // Nothing useful really
        ChipLogError(Discovery, "Query responder storage size too small");
    }
}

QueryResponderSettings QueryResponderBase::AddResponder(RecordResponder * responder)
{
    if (responder == nullptr)
    {
        return QueryResponderSettings();
    }
    ChipLogDetail(Discovery, "Responding with %s", QNameString(responder->GetQName()).c_str());

    for (size_t i = 0; i < mResponderInfoSize; i++)
    {
        if (mResponderInfos[i].responder == nullptr)
        {
            mResponderInfos[i].Clear();
            mResponderInfos[i].responder = responder;

            return QueryResponderSettings(&mResponderInfos[i]);
        }
    }
    return QueryResponderSettings();
}

void QueryResponderBase::ResetAdditionals()
{

    for (size_t i = 0; i < mResponderInfoSize; i++)
    {
        mResponderInfos[i].reportNowAsAdditional = false;
    }
}

size_t QueryResponderBase::MarkAdditional(const FullQName & qname)
{
    size_t count = 0;
    for (size_t i = 0; i < mResponderInfoSize; i++)
    {
        if (mResponderInfos[i].responder == nullptr)
        {
            continue; // not a valid entry
        }

        if (mResponderInfos[i].reportNowAsAdditional)
        {
            continue; // already marked
        }

        if (mResponderInfos[i].responder->GetQName() == qname)
        {
            mResponderInfos[i].reportNowAsAdditional = true;
            count++;
        }
    }

    return count;
}

void QueryResponderBase::MarkAdditionalRepliesFor(QueryResponderIterator it)
{
    Internal::QueryResponderInfo * info = it.GetInternal();

    if (!info->alsoReportAdditionalQName)
    {
        return; // nothing additional to report
    }

    if (MarkAdditional(info->additionalQName) == 0)
    {
        return; // nothing additional added
    }

    // something additionally added. Iterate and re-add until no more additional items were added
    bool keepAdding = true;
    while (keepAdding)
    {
        keepAdding = false;

        QueryResponderRecordFilter filter;
        filter.SetIncludeAdditionalRepliesOnly(true);

        for (auto ait = begin(&filter); ait != end(); ait++)
        {
            if (ait.GetInternal()->alsoReportAdditionalQName)
            {
                keepAdding = keepAdding || (MarkAdditional(ait.GetInternal()->additionalQName) != 0);
            }
        }
    }
}

void QueryResponderBase::AddAllResponses(const chip::Inet::IPPacketInfo * source, ResponderDelegate * delegate,
                                         const ResponseConfiguration & configuration)
{
    if (!delegate->ShouldSend(*this))
    {
        return;
    }

    // reply to dns-sd service list request
    for (size_t i = 0; i < mResponderInfoSize; i++)
    {
        if (!mResponderInfos[i].reportService)
        {
            continue;
        }

        if (mResponderInfos[i].responder == nullptr)
        {
            continue;
        }

        PtrResourceRecord record(GetQName(), mResponderInfos[i].responder->GetQName());
        configuration.Adjust(record);
        delegate->AddResponse(record);
    }

    delegate->ResponsesAdded(*this);
}

void QueryResponderBase::ClearBroadcastThrottle()
{
    for (size_t i = 0; i < mResponderInfoSize; i++)
    {
        mResponderInfos[i].lastMulticastTime = chip::System::Clock::kZero;
    }
}

} // namespace Minimal
} // namespace mdns
