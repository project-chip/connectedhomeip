/*
 *
 *    Copyright (c) 2026 Project CHIP Authors
 *    All rights reserved.
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

#include <app/ConcreteAttributePath.h>
#include <app/MessageDef/StatusIB.h>
#include <app/ReadClient.h>
#include <app/ReadPrepareParams.h>
#include <lib/core/CHIPError.h>
#include <lib/core/DataModelTypes.h>
#include <lib/core/NodeId.h>
#include <lib/core/TLV.h>
#include <messaging/ExchangeMgr.h>

#include <cstdint>

namespace chip {
namespace Silabs {

/**
 * @brief Reusable subscription manager built on top of chip::app::ReadClient.
 *
 * Owns a fixed-size pool of active subscription slots and forwards lifecycle
 * events (data, established, error, terminated) to an optional Delegate.
 */
class SubscriptionManager
{
public:
    using Handle                                         = uint8_t;
    static constexpr Handle kInvalidHandle               = 0xFF;
    static constexpr uint8_t kMaxSubscriptions           = 10;
    static constexpr uint16_t kDefaultMinIntervalSeconds = 0;
    static constexpr uint16_t kDefaultMaxIntervalSeconds = 60;

    struct Info
    {
        chip::FabricIndex fabricIndex = 0;
        chip::NodeId nodeId           = 0;
        chip::EndpointId endpointId   = 0;
        chip::ClusterId clusterId     = 0;
        chip::AttributeId attributeId = 0;
        uint64_t timeSinceLastReport  = 0;
    };

    /**
     * @brief Delegate interface for consumers (shell, RPC, app logic).
     *
     * All callbacks run on the Matter event loop thread.
     */
    class Delegate
    {
    public:
        virtual ~Delegate() = default;

        virtual void OnAttributeData(Handle /*handle*/, const Info & /*info*/,
                                     const chip::app::ConcreteDataAttributePath & /*path*/,
                                     chip::TLV::TLVReader * /*data*/,
                                     const chip::app::StatusIB & /*status*/)
        {}

        virtual void OnSubscriptionEstablished(Handle /*handle*/, const Info & /*info*/,
                                               chip::SubscriptionId /*subscriptionId*/)
        {}

        virtual void OnError(Handle /*handle*/, const Info & /*info*/, CHIP_ERROR /*error*/) {}

        virtual void OnSubscriptionTerminated(Handle /*handle*/, const Info & /*info*/) {}
    };

    static SubscriptionManager & Instance();

    void SetDelegate(Delegate * delegate) { mDelegate = delegate; }

    /**
     * @brief Create an attribute subscription using auto-resubscribe.
     *
     * @param info               Target fabric/node/endpoint/cluster/attribute.
     * @param outHandle          Optional output: handle of the allocated slot.
     * @param minIntervalSeconds Min reporting interval floor.
     * @param maxIntervalSeconds Max reporting interval ceiling.
     */
    CHIP_ERROR Subscribe(const Info & info, Handle * outHandle = nullptr,
                         uint16_t minIntervalSeconds = kDefaultMinIntervalSeconds,
                         uint16_t maxIntervalSeconds = kDefaultMaxIntervalSeconds);

    uint8_t ActiveCount() const;
    bool IsActive(Handle handle) const;
    const Info * GetInfo(Handle handle) const;

private:
    SubscriptionManager();
    SubscriptionManager(const SubscriptionManager &)             = delete;
    SubscriptionManager & operator=(const SubscriptionManager &) = delete;

    class SlotCallback : public chip::app::ReadClient::Callback
    {
    public:
        void Init(SubscriptionManager * owner, Handle handle);

        void OnAttributeData(const chip::app::ConcreteDataAttributePath & path, chip::TLV::TLVReader * data,
                             const chip::app::StatusIB & status) override;
        void OnSubscriptionEstablished(chip::SubscriptionId subscriptionId) override;
        void OnError(CHIP_ERROR error) override;
        void OnDone(chip::app::ReadClient * client) override;
        void OnDeallocatePaths(chip::app::ReadPrepareParams && readPrepareParams) override;

    private:
        SubscriptionManager * mOwner = nullptr;
        Handle mHandle               = kInvalidHandle;
    };

    struct Slot
    {
        chip::app::ReadClient * client = nullptr;
        Info info;
        SlotCallback callback;
    };

    Handle FindFreeSlot();
    void ReleaseSlot(Handle handle);

    Slot mSlots[kMaxSubscriptions];
    Delegate * mDelegate = nullptr;
};

} // namespace Silabs
} // namespace chip
