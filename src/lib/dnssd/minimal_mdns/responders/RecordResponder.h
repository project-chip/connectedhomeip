/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
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

#include <lib/dnssd/minimal_mdns/responders/Responder.h>
namespace mdns {
namespace Minimal {

// This is a container class for the various record responders (PTR, SRV, TXT, A, AAAA etc.)
// This class is used to restrict the set of possible responders added to a QueryResponder.
class RecordResponder : public Responder
{
public:
    RecordResponder(QType qType, const FullQName & qName) : Responder(qType, qName) {}
};

} // namespace Minimal
} // namespace mdns
