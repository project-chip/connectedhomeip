/*
 *
 *    Copyright (c) 2020-2021 Project CHIP Authors
 *    Copyright (c) 2013-2017 Nest Labs, Inc.
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

#include "IPPacketInfo.h"

namespace chip {
namespace Inet {

void IPPacketInfo::Clear()
{
    SrcAddress  = IPAddress::Any;
    DestAddress = IPAddress::Any;
    Interface   = InterfaceId::Null();
    SrcPort     = 0;
    DestPort    = 0;
}

} // namespace Inet
} // namespace chip
