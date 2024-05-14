/*
 *
 *    Copyright (c) 2024 Project CHIP Authors
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

#include <app-common/zap-generated/cluster-enums.h>
#include <app-common/zap-generated/cluster-objects.h>
#include <inet/IPAddress.h>
#include <lib/core/CHIPError.h>
#include <lib/core/Optional.h>
#include <lib/support/ScopedBuffer.h>
#include <lib/support/Span.h>
#include <lib/support/ThreadOperationalDataset.h>

namespace chip {
namespace app {
namespace Clusters {
namespace ThreadBorderRouterManagement {

constexpr size_t kBorderRouterNameMaxLength = 63;
constexpr size_t kBorderAgentIdLength       = 16;

class Delegate
{
public:
    Delegate()          = default;
    virtual ~Delegate() = default;

    using RouteTableEntry = Structs::RouteTableStruct::Type;
    using ChildTableEntry = Structs::ChildTableStruct::Type;
    class ThreadNode : public Structs::ThreadNodeStruct::Type
    {
    public:
        Platform::ScopedMemoryBuffer<chip::Inet::IPAddress> Ipv6Addresses;
        Platform::ScopedMemoryBuffer<chip::ByteSpan> Ipv6AddressesSpans;
        Platform::ScopedMemoryBuffer<RouteTableEntry> Routes;
        Platform::ScopedMemoryBuffer<ChildTableEntry> Children;
    };
    class Callback
    {
    public:
        virtual void OnTopologyRequestFinished(Protocols::InteractionModel::Status status, uint8_t snapshot,
                                               const Span<ThreadNode> & threadNodes) = 0;

        virtual ~Callback() = default;
    };

    virtual CHIP_ERROR Init() = 0;

    virtual CHIP_ERROR GetPanChangeSupported(bool & panChangeSupported) = 0;

    virtual CHIP_ERROR GetBorderRouterName(MutableCharSpan & borderRouterName) = 0;

    virtual CHIP_ERROR GetBorderAgentId(MutableByteSpan & borderAgentId) = 0;

    virtual CHIP_ERROR GetThreadVersion(uint16_t & threadVersion) = 0;

    virtual CHIP_ERROR GetInterfaceEnabled(bool & interfaceEnabled) = 0;

    virtual CHIP_ERROR GetThreadNode(ThreadNode & threadNode) = 0;

    virtual CHIP_ERROR GetActiveDataset(chip::Thread::OperationalDataset & activeDataset) = 0;

    virtual CHIP_ERROR GetPendingDataset(chip::Thread::OperationalDataset & pendingDataset) = 0;

    virtual CHIP_ERROR SetActiveDataset(const chip::Thread::OperationalDataset & activeDataset) = 0;

    virtual CHIP_ERROR SetPendingDataset(const chip::Thread::OperationalDataset & pendingDataset) = 0;

    virtual CHIP_ERROR GetTopology(uint8_t snapshot, Callback * callback) = 0;
};

} // namespace ThreadBorderRouterManagement
} // namespace Clusters
} // namespace app
} // namespace chip
