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

#include <lib/dnssd/minimal_mdns/core/QName.h>
#include <lib/dnssd/minimal_mdns/records/ResourceRecord.h>

#include <inet/IPPacketInfo.h>
#include <lib/core/Optional.h>

namespace mdns {
namespace Minimal {

/// Controls specific options for responding to mDNS queries
///
class ResponseConfiguration
{
public:
    ResponseConfiguration() {}
    ~ResponseConfiguration() = default;

    chip::Optional<uint32_t> GetTtlSecondsOverride() const { return mTtlSecondsOverride; }
    ResponseConfiguration & SetTtlSecondsOverride(chip::Optional<uint32_t> override)
    {
        mTtlSecondsOverride = override;
        return *this;
    }

    ResponseConfiguration & SetTtlSecondsOverride(uint32_t value) { return SetTtlSecondsOverride(chip::MakeOptional(value)); }
    ResponseConfiguration & ClearTtlSecondsOverride() { return SetTtlSecondsOverride(chip::NullOptional); }

    /// Applies any adjustments to resource records before they are being serialized
    /// to some form of reply.
    void Adjust(ResourceRecord & record) const
    {
        if (mTtlSecondsOverride.HasValue())
        {
            record.SetTtl(mTtlSecondsOverride.Value());
        }
    }

private:
    chip::Optional<uint32_t> mTtlSecondsOverride;
};

// Delegates that responders can write themselves to
class ResponderDelegate;

/// Adds ability to respond with specific types of data
class Responder
{
public:
    Responder(QType qType, const FullQName & qName) : mQType(qType), mQName(qName) {}
    virtual ~Responder() {}

    QClass GetQClass() const { return QClass::IN; }
    QType GetQType() const { return mQType; }

    /// Full name as: "Instance"."Servicer"."Domain"
    /// Domain name is generally just 'local'
    FullQName GetQName() const { return mQName; }

    /// Report all responses maintained by this responder
    ///
    /// Responses are associated with the objects type/class/qname.
    virtual void AddAllResponses(const chip::Inet::IPPacketInfo * source, ResponderDelegate * delegate,
                                 const ResponseConfiguration & configuration) = 0;

private:
    const QType mQType;
    const FullQName mQName;
};

class ResponderDelegate
{
public:
    virtual ~ResponderDelegate() {}

    /// Add the specified resource record to the response
    virtual void AddResponse(const ResourceRecord & record) = 0;

    /// Accept to add responses for the particular responder.
    ///
    /// This will be called before responders serialize their records.
    virtual bool ShouldSend(const Responder &) const { return true; }

    /// Called when all responses were added for a particular responder
    ///
    /// Only called if a previous accept returned true.
    virtual void ResponsesAdded(const Responder &) {}
};

} // namespace Minimal
} // namespace mdns
