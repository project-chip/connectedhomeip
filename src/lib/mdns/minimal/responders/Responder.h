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

#include <lib/mdns/minimal/core/QName.h>
#include <lib/mdns/minimal/records/ResourceRecord.h>

#include <inet/InetLayer.h>

namespace mdns {
namespace Minimal {

class ResponderDelegate
{
public:
    virtual ~ResponderDelegate() {}

    /// Add the specified resource record to the response
    virtual void AddResponse(const ResourceRecord & record) = 0;
};

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

    /// Report all reponses maintained by this responder
    ///
    /// Responses are associated with the objects type/class/qname.
    virtual void AddAllResponses(const chip::Inet::IPPacketInfo * source, ResponderDelegate * delegate) = 0;

private:
    const QType mQType;
    const FullQName mQName;
};

} // namespace Minimal
} // namespace mdns
