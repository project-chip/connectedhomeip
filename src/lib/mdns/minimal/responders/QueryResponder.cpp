/* See Project CHIP LICENSE file for licensing information. */

#include "QueryResponder.h"

#include <mdns/minimal/records/Ptr.h>

#include <support/logging/CHIPLogging.h>

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
        // Nothing usefull really
        ChipLogError(Discovery, "Query responder storage size too small");
    }
}

QueryResponderSettings QueryResponderBase::AddResponder(Responder * responder)
{
    if (responder == nullptr)
    {
        return QueryResponderSettings();
    }

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

void QueryResponderBase::AddAllResponses(const chip::Inet::IPPacketInfo * source, ResponderDelegate * delegate)
{
    ChipLogProgress(Discovery, "Replying to DNS-SD service listing request");

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

        delegate->AddResponse(PtrResourceRecord(GetQName(), mResponderInfos[i].responder->GetQName()));
    }
}

} // namespace Minimal
} // namespace mdns
