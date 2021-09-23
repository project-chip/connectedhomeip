/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
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

#include "QueryResponder.h"

#include <lib/mdns/minimal/records/Ptr.h>

#include <lib/support/logging/CHIPLogging.h>

namespace mdns {
namespace Minimal {

const QNamePart kDnsSdQueryPath[] = { "_services", "_dns-sd", "_udp", "local" };

QueryResponderBase::QueryResponderBase(chip::GenericAllocatorBase<Internal::QueryResponderInfo> * infos) :
    Responder(QType::PTR, FullQName(kDnsSdQueryPath)), mResponderInfos(infos)
{}

void QueryResponderBase::Init()
{
    for (auto it = mResponderInfos->begin(); it != mResponderInfos->end();)
    {
        it = mResponderInfos->Free(it);
    }
    Internal::QueryResponderInfo * serviceResponderInfo = mResponderInfos->Allocate();
    if (serviceResponderInfo == nullptr)
    {
        ChipLogError(Discovery, "Insufficient memory in query responder pool");
        return;
    }
    serviceResponderInfo->responder = this;
}

QueryResponderSettings QueryResponderBase::AddResponder(RecordResponder * responder)
{
    if (responder == nullptr)
    {
        return QueryResponderSettings();
    }
    Internal::QueryResponderInfo * info = mResponderInfos->Allocate();
    if (info != nullptr)
    {
        info->responder = responder;
        return QueryResponderSettings(info);
    }
    return QueryResponderSettings();
}

void QueryResponderBase::ResetAdditionals()
{
    for (auto & i : *mResponderInfos)
    {
        i.reportNowAsAdditional = false;
    }
}

size_t QueryResponderBase::MarkAdditional(const FullQName & qname)
{
    size_t count = 0;
    for (auto & i : *mResponderInfos)
    {
        if (i.responder == nullptr)
        {
            continue; // not a valid entry
        }

        if (i.reportNowAsAdditional)
        {
            continue; // already marked
        }

        if (i.responder->GetQName() == qname)
        {
            i.reportNowAsAdditional = true;
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
    for (auto & i : *mResponderInfos)
    {
        if (!i.reportService)
        {
            continue;
        }

        if (i.responder == nullptr)
        {
            continue;
        }

        delegate->AddResponse(PtrResourceRecord(GetQName(), i.responder->GetQName()));
    }
}

void QueryResponderBase::ClearBroadcastThrottle()
{
    for (auto & i : *mResponderInfos)
    {
        i.lastMulticastTime = 0;
    }
}

} // namespace Minimal
} // namespace mdns
