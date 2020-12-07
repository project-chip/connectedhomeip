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

#include <mdns/minimal/responders/Responder.h>

namespace mdns {
namespace Minimal {

class IPv4Responder : public Responder
{
public:
    IPv4Responder(const FullQName & qname) : Responder(QType::A, qname) {}

    void AddAllResponses(const chip::Inet::IPPacketInfo * source, ResponderDelegate * delegate) override;
};

class IPv6Responder : public Responder
{
public:
    IPv6Responder(const FullQName & qname) : Responder(QType::AAAA, qname) {}

    void AddAllResponses(const chip::Inet::IPPacketInfo * source, ResponderDelegate * delegate) override;
};

} // namespace Minimal
} // namespace mdns
