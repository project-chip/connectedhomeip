/*
 *
 *    Copyright (c) 2024 Project CHIP Authors
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

#include <app-common/zap-generated/cluster-objects.h>
#include <app/util/attribute-storage.h>

#include <string>

class BridgedDevice
{
public:
    /// Defines all attributes that we keep track of for a bridged device
    struct BridgedAttributes
    {
        std::string uniqueId;
        std::string vendorName;
        uint16_t vendorId = 0;
        std::string productName;
        uint16_t productId = 0;
        std::string nodeLabel;
        uint16_t hardwareVersion = 0;
        std::string hardwareVersionString;
        uint32_t softwareVersion = 0;
        std::string softwareVersionString;
    };

    struct AdminCommissioningAttributes
    {
        chip::app::Clusters::AdministratorCommissioning::CommissioningWindowStatusEnum commissioningWindowStatus =
            chip::app::Clusters::AdministratorCommissioning::CommissioningWindowStatusEnum::kWindowNotOpen;
        std::optional<chip::FabricIndex> openerFabricIndex = std::nullopt;
        std::optional<chip::VendorId> openerVendorId       = std::nullopt;
    };

    BridgedDevice(chip::ScopedNodeId scopedNodeId);
    virtual ~BridgedDevice() = default;

    [[nodiscard]] bool IsReachable() const { return mReachable; }
    void SetReachable(bool reachable);

    void LogActiveChangeEvent(uint32_t promisedActiveDurationMs);

    [[nodiscard]] bool IsIcd() const { return mIsIcd; }
    void SetIcd(bool icd) { mIsIcd = icd; }

    inline void SetEndpointId(chip::EndpointId id) { mEndpointId = id; };
    inline chip::EndpointId GetEndpointId() { return mEndpointId; };
    inline chip::ScopedNodeId GetScopedNodeId() { return mScopedNodeId; };
    inline void SetParentEndpointId(chip::EndpointId id) { mParentEndpointId = id; };
    inline chip::EndpointId GetParentEndpointId() { return mParentEndpointId; };

    [[nodiscard]] const BridgedAttributes & GetBridgedAttributes() const { return mAttributes; }
    void SetBridgedAttributes(const BridgedAttributes & value) { mAttributes = value; }

    void SetAdminCommissioningAttributes(const AdminCommissioningAttributes & aAdminCommissioningAttributes);
    const AdminCommissioningAttributes & GetAdminCommissioningAttributes() const { return mAdminCommissioningAttributes; }

    /// Convenience method to set just the unique id of a bridged device as it
    /// is one of the few attributes that is not always bulk-set
    void SetUniqueId(const std::string & value) { mAttributes.uniqueId = value; }

protected:
    bool mReachable = false;
    bool mIsIcd     = false;

    chip::ScopedNodeId mScopedNodeId;
    chip::EndpointId mEndpointId       = 0;
    chip::EndpointId mParentEndpointId = 0;

    BridgedAttributes mAttributes;
    AdminCommissioningAttributes mAdminCommissioningAttributes;
};
