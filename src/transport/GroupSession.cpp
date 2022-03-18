/*
 *    Copyright (c) 2022 Project CHIP Authors
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

#include "GroupSession.h"

namespace chip {
namespace Transport {

IncomingGroupSession::~IncomingGroupSession()
{
    NotifySessionReleased();
#ifndef NDEBUG
    VerifyOrDie(GetReferenceCount() == 0);
#endif
}

void IncomingGroupSession::Retain()
{
#ifndef NDEBUG
    ReferenceCounted<IncomingGroupSession, GroupSessionDeleter, 0>::Retain();
#endif
}

void IncomingGroupSession::Release()
{
#ifndef NDEBUG
    ReferenceCounted<IncomingGroupSession, GroupSessionDeleter, 0>::Release();
#endif
}

OutgoingGroupSession::~OutgoingGroupSession()
{
    NotifySessionReleased();
#ifndef NDEBUG
    VerifyOrDie(GetReferenceCount() == 0);
#endif
}

void OutgoingGroupSession::Retain()
{
#ifndef NDEBUG
    ReferenceCounted<OutgoingGroupSession, GroupSessionDeleter, 0>::Retain();
#endif
}

void OutgoingGroupSession::Release()
{
#ifndef NDEBUG
    ReferenceCounted<OutgoingGroupSession, GroupSessionDeleter, 0>::Release();
#endif
}

} // namespace Transport
} // namespace chip