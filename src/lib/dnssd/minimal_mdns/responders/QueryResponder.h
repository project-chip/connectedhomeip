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

#include "RecordResponder.h"
#include "ReplyFilter.h"
#include "Responder.h"

#include <system/SystemClock.h>

namespace mdns {
namespace Minimal {

/// Represents available data (replies) for mDNS queries.
struct QueryResponderRecord
{
    Responder * responder                            = nullptr; // what response/data is available
    bool reportService                               = false;   // report as a service when listing dnssd services
    chip::System::Clock::Timestamp lastMulticastTime = chip::System::Clock::kZero; // last time this record was multicast
};

namespace Internal {

/// Internal information for query responder records.
struct QueryResponderInfo : public QueryResponderRecord
{
    bool reportNowAsAdditional; // report as additional data required

    bool alsoReportAdditionalQName = false; // report more data when this record is listed
    FullQName additionalQName;              // if alsoReportAdditionalQName is set, send this extra data

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

    /// This record should be part of dns-sd service listing requests
    QueryResponderSettings & SetReportInServiceListing(bool reportService)
    {
        if (IsValid())
        {
            mInfo->reportService = reportService;
        }
        return *this;
    }

    /// When this record is send back, additional records should also be provided.
    ///
    /// This is useful to avoid chattyness by sending back referenced records
    /// (e.g. when sending a PTR record, send the corresponding SRV and when sending
    ///  SRV, send back the corresponding A/AAAA records).
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

/// Determines what query records should be included in a response.
///
/// Provides an 'Accept' method to determine if a reply is to be sent or not.
class QueryResponderRecordFilter
{
public:
    /// Default contstructor accepts everything that is not null
    QueryResponderRecordFilter() {}
    QueryResponderRecordFilter(const QueryResponderRecordFilter & other)             = default;
    QueryResponderRecordFilter & operator=(const QueryResponderRecordFilter & other) = default;

    /// Set if to include only items marked as 'additional reply' or everything.
    QueryResponderRecordFilter & SetIncludeAdditionalRepliesOnly(bool includeAdditionalRepliesOnly)
    {
        mIncludeAdditionalRepliesOnly = includeAdditionalRepliesOnly;
        return *this;
    }

    /// Filter out anything rejected by the given reply filter.
    /// If replyFilter is nullptr, no such filtering is applied.
    QueryResponderRecordFilter & SetReplyFilter(ReplyFilter * replyFilter)
    {
        mReplyFilter = replyFilter;
        return *this;
    }

    /// Filter out anything that was multicast past ms.
    /// If ms is 0, no filtering is done
    QueryResponderRecordFilter & SetIncludeOnlyMulticastBeforeMS(chip::System::Clock::Timestamp time)
    {
        mIncludeOnlyMulticastBefore = time;
        return *this;
    }

    bool Accept(Internal::QueryResponderInfo * record) const
    {
        if (record->responder == nullptr)
        {
            return false;
        }

        if (mIncludeAdditionalRepliesOnly && !record->reportNowAsAdditional)
        {
            return false;
        }

        if ((mIncludeOnlyMulticastBefore > chip::System::Clock::kZero) &&
            (record->lastMulticastTime >= mIncludeOnlyMulticastBefore))
        {
            return false;
        }

        if ((mReplyFilter != nullptr) &&
            !mReplyFilter->Accept(record->responder->GetQType(), record->responder->GetQClass(), record->responder->GetQName()))
        {
            return false;
        }
        return true;
    }

private:
    bool mIncludeAdditionalRepliesOnly                         = false;
    ReplyFilter * mReplyFilter                                 = nullptr;
    chip::System::Clock::Timestamp mIncludeOnlyMulticastBefore = chip::System::Clock::kZero;
};

/// Iterates over an array of QueryResponderRecord items, providing only 'valid' ones, where
/// valid is based on the provided filter.
class QueryResponderIterator
{
public:
    using value_type = QueryResponderRecord;
    using pointer    = QueryResponderRecord *;
    using reference  = QueryResponderRecord &;

    QueryResponderIterator() : mCurrent(nullptr), mRemaining(0) {}
    QueryResponderIterator(QueryResponderRecordFilter * recordFilter, Internal::QueryResponderInfo * pos, size_t size) :
        mFilter(recordFilter), mCurrent(pos), mRemaining(size)
    {
        SkipInvalid();
    }
    QueryResponderIterator(const QueryResponderIterator & other)             = default;
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
    /// Skips invalid/not useful values.
    /// ensures that if mRemaining is 0, mCurrent is nullptr;
    void SkipInvalid()
    {
        while ((mRemaining > 0) && !mFilter->Accept(mCurrent))
        {
            mRemaining--;
            mCurrent++;
        }
        if (mRemaining == 0)
        {
            mCurrent = nullptr;
        }
    }

    QueryResponderRecordFilter * mFilter;
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
    ~QueryResponderBase() override {}

    /// Setup initial settings (clears all infos and sets up dns-sd query replies)
    void Init();

    /// Add a new responder to be processed
    ///
    /// Return valid QueryResponderSettings on add success.
    QueryResponderSettings AddResponder(RecordResponder * responder);

    /// Implementation of the responder delegate.
    ///
    /// Adds responses for all known _dns-sd services.
    void AddAllResponses(const chip::Inet::IPPacketInfo * source, ResponderDelegate * delegate,
                         const ResponseConfiguration & configuration) override;

    QueryResponderIterator begin(QueryResponderRecordFilter * filter)
    {
        return QueryResponderIterator(filter, mResponderInfos, mResponderInfoSize);
    }
    QueryResponderIterator end() { return QueryResponderIterator(); }

    /// Clear any items marked as 'additional'.
    void ResetAdditionals();

    /// Marks queries matching this qname as 'to be additionally reported'
    /// @return the number of items marked new as 'additional data'.
    size_t MarkAdditional(const FullQName & qname);

    /// Flag any additional responses required for the given iterator
    void MarkAdditionalRepliesFor(QueryResponderIterator it);

    /// Resets the internal broadcast throttle setting to allow re-broadcasting
    /// of all packets without a timedelay.
    void ClearBroadcastThrottle();

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
