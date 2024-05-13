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

#pragma once

#include <lib/dnssd/minimal_mdns/responders/ReplyFilter.h>

#include "Parser.h"
#include "Query.h"

namespace mdns {
namespace Minimal {

/// A reply filter implementation based on a received/parsed query.
class QueryReplyFilter : public ReplyFilter
{
public:
    QueryReplyFilter(const QueryData & queryData) : mQueryData(queryData) {}

    bool Accept(QType qType, QClass qClass, FullQName qname) override
    {
        if (!AcceptableQueryType(qType))
        {
            return false;
        }

        if (!AcceptableQueryClass(qClass))
        {
            return false;
        }

        return AcceptablePath(qname);
    }

    /// Ignore qname matches during Accept calls (if set to true, only qtype and qclass are matched).
    ///
    /// Ignoring qname is useful when sending related data replies: cliens often query for PTR
    /// records however to avoid chattyness, servers generally send all of PTR, SRV (pointed to by PTR)
    /// and A/AAAA (pointed to by SRV).
    QueryReplyFilter & SetIgnoreNameMatch(bool ignore)
    {
        mIgnoreNameMatch = ignore;
        return *this;
    }

    QueryReplyFilter & SetSendingAdditionalItems(bool additional)
    {
        mSendingAdditionalItems = additional;
        return *this;
    }

private:
    bool AcceptableQueryType(QType qType)
    {
        if (mSendingAdditionalItems)
        {
            return true;
        }

        return ((mQueryData.GetType() == QType::ANY) || (mQueryData.GetType() == qType));
    }

    bool AcceptableQueryClass(QClass qClass)
    {
        return ((mQueryData.GetClass() == QClass::ANY) || (mQueryData.GetClass() == qClass));
    }

    bool AcceptablePath(FullQName qname)
    {
        if (mIgnoreNameMatch || mQueryData.IsAnnounceBroadcast())
        {
            return true;
        }

        return (mQueryData.GetName() == qname);
    }

    const QueryData & mQueryData;
    bool mIgnoreNameMatch        = false;
    bool mSendingAdditionalItems = false;
};

} // namespace Minimal
} // namespace mdns
