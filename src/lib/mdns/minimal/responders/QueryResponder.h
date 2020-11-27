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

#include "Responder.h"

#include <inet/InetLayer.h>

#include <iterator>

namespace mdns {
namespace Minimal {

struct QueryResponderRecord
{
    Responder * responder = nullptr; // where the response is sent
    bool reportService    = false;   // report as a service when listing dnssd services
};

namespace Internal {

// Includes internally used tracking information
struct QueryResponderInfo : public QueryResponderRecord
{
    bool reportNowAsAdditional; // report as additional data required

    bool alsoReportAdditionalQName = false; // report more data when this record is listed
    FullQName additionalQName;

    void Clear()
    {
        responder                 = nullptr;
        reportService             = false;
        reportNowAsAdditional     = false;
        alsoReportAdditionalQName = false;
    }
};

} // namespace Internal

/// Allows building query responder configuration
class QueryResponderSettings
{
public:
    QueryResponderSettings() : mInfo(nullptr) {}
    QueryResponderSettings(Internal::QueryResponderInfo * info) : mInfo(info) {}
    QueryResponderSettings(const QueryResponderSettings & other) = default;

    QueryResponderSettings & SetReportInServiceListing(bool reportService)
    {
        if (IsValid())
        {
            mInfo->reportService = reportService;
        }
        return *this;
    }

    QueryResponderSettings & SetReportAdditional(const FullQName & qname)
    {
        if (IsValid())
        {
            mInfo->alsoReportAdditionalQName = true;
            mInfo->additionalQName           = qname;
        }
        return *this;
    }

    bool IsValid() const { return mInfo != nullptr; }

private:
    Internal::QueryResponderInfo * mInfo;
};

class QueryResponderIterator : public std::iterator<std::input_iterator_tag, QueryResponderRecord>
{
public:
    QueryResponderIterator() : mAdditionalOnly(false), mCurrent(nullptr), mRemaining(0) {}
    QueryResponderIterator(bool additionalOnly, Internal::QueryResponderInfo * pos, size_t size) :
        mAdditionalOnly(additionalOnly), mCurrent(pos), mRemaining(size)
    {
        SkipInvalid();
    }
    QueryResponderIterator(const QueryResponderIterator & other) = default;
    QueryResponderIterator & operator=(const QueryResponderIterator & other) = default;

    QueryResponderIterator & operator++()
    {
        if (mRemaining != 0)
        {
            mCurrent++;
            mRemaining--;
        }
        SkipInvalid();
        return *this;
    }

    QueryResponderIterator operator++(int)
    {
        QueryResponderIterator tmp(*this);
        operator++();
        return tmp;
    }

    bool operator==(const QueryResponderIterator & rhs) const { return mCurrent == rhs.mCurrent; }
    bool operator!=(const QueryResponderIterator & rhs) const { return mCurrent != rhs.mCurrent; }

    QueryResponderRecord & operator*() { return *mCurrent; }
    QueryResponderRecord * operator->() { return mCurrent; }

    Internal::QueryResponderInfo * GetInternal() { return mCurrent; }
    const Internal::QueryResponderInfo * GetInternal() const { return mCurrent; }

private:
    /// Checks if the value pointed to by 'mCurrent' is a valid reply value.
    ///
    /// Valid is being described as
    ///   - has a responder
    ///   - record type matches the 'addtional only' setting for the iterator.
    bool isCurrentValid() const
    {
        if (mCurrent->responder == nullptr)
        {
            return false;
        }

        if (mAdditionalOnly && !mCurrent->reportNowAsAdditional)
        {
            return false;
        }

        return true;
    }

    /// Skips invalid/not useful values.
    /// ensures that if mRemaining is 0, mCurrent is nullptr;
    void SkipInvalid()
    {
        while ((mRemaining > 0) && !isCurrentValid())
        {
            mRemaining--;
            mCurrent++;
        }
        if (mRemaining == 0)
        {
            mCurrent = nullptr;
        }
    }

    bool mAdditionalOnly;
    Internal::QueryResponderInfo * mCurrent;
    size_t mRemaining;
};

/// Responds to mDNS queries.
///
/// In particular:
///   - replies data as provided by the underlying responders
///   - replies to "_services._dns-sd._udp.local."
///
/// Maintains a stateful list of 'additional replies' that can be marked/unmarked
/// for query processing
class QueryResponderBase : public Responder // "_services._dns-sd._udp.local"
{
public:
    /// Builds a new responder with the given storage for the response infos
    QueryResponderBase(Internal::QueryResponderInfo * infos, size_t infoSizes);
    virtual ~QueryResponderBase() {}

    /// Setup initial settings (clears all infos and sets up dns-sd query replies)
    void Init();

    /// Add a new responder to be processed
    ///
    /// Return valid QueryResponderSettings on add success.
    QueryResponderSettings AddResponder(Responder * responder);

    /// Implementation of the responder delegate
    ///
    /// adds responses for all known _dns-sd services
    void AddAllResponses(const chip::Inet::IPPacketInfo * source, ResponderDelegate * delegate) override;

    QueryResponderIterator begin() { return QueryResponderIterator(false, mResponderInfos, mResponderInfoSize); }
    QueryResponderIterator end() { return QueryResponderIterator(); }

    // Keeping track of 'additional data to report'

    void ResetAdditionals();

    /// Marks queries matching this qname as 'to be additionally reported'
    /// @return the number of items marked new as 'additional data'.
    size_t MarkAdditional(const FullQName & qname);

    /// Flag any additional responses required for the given iterator
    void MarkAdditionalRepliesFor(QueryResponderIterator it);

    QueryResponderIterator additional_begin() { return QueryResponderIterator(true, mResponderInfos, mResponderInfoSize); }
    QueryResponderIterator additional_end() { return QueryResponderIterator(); }

private:
    Internal::QueryResponderInfo * mResponderInfos;
    size_t mResponderInfoSize;
};

template <size_t kSize>
class QueryResponder : public QueryResponderBase
{
public:
    QueryResponder() : QueryResponderBase(mData, kSize) { Init(); }

private:
    Internal::QueryResponderInfo mData[kSize];
};

} // namespace Minimal
} // namespace mdns
