/*
 *
 *    Copyright (c) 2023 Project CHIP Authors
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

#include <lib/core/TLV.h>
#include <messaging/ReliableMessageProtocolConfig.h>

namespace chip {

// TODO We should get part of this from constexpr that is in ReliableMessageProtocolConfig.h

class SessionParameters
{
public:
    SessionParameters(ReliableMessageProtocolConfig mrpConfig = GetDefaultMRPConfig()) : mMRPConfig(mrpConfig) {}

    // This estimated TLV size calc is here instead of messaging/ReliableMessageProtocolConfig.h
    // because we would need to add `include <lib/core/TLV.h>`. While we could make it all work
    // from a build standpoint, if any new MRP config gets added accessors will still need to be
    // added here so having this calc done here isn't problematic.
    static constexpr size_t kSizeOfSessionIdleInterval      = sizeof(uint32_t);
    static constexpr size_t kSizeOfSessionActiveInterval    = sizeof(uint32_t);
    static constexpr size_t kSizeOfSessionActiveThreshold   = sizeof(uint16_t);
    static constexpr size_t kSizeOfDataModelRevision        = sizeof(uint16_t);
    static constexpr size_t kSizeOfInteractionModelRevision = sizeof(uint16_t);
    static constexpr size_t kSizeOfSpecificationVersion     = sizeof(uint32_t);
    static constexpr size_t kSizeOfMaxPathsPerInvoke        = sizeof(uint16_t);

    static constexpr size_t kEstimatedTLVSize = TLV::EstimateStructOverhead(
        kSizeOfSessionIdleInterval, kSizeOfSessionActiveInterval, kSizeOfSessionActiveThreshold, kSizeOfDataModelRevision,
        kSizeOfInteractionModelRevision, kSizeOfSpecificationVersion, kSizeOfMaxPathsPerInvoke);

    // From Section 4.12.8 "Parameters and Constants" in chapter "Secure Channel".
    enum Tag : uint32_t
    {
        kSessionIdleInterval      = 1,
        kSessionActiveInterval    = 2,
        kSessionActiveThreshold   = 3,
        kDataModelRevision        = 4,
        kInteractionModelRevision = 5,
        kSpecificationVersion     = 6,
        kMaxPathsPerInvoke        = 7,
    };

    const ReliableMessageProtocolConfig & GetMRPConfig() const { return mMRPConfig; }
    void SetMRPConfig(const ReliableMessageProtocolConfig & config) { mMRPConfig = config; }
    void SetMRPIdleRetransTimeout(const System::Clock::Milliseconds32 idleRetransTimeout)
    {
        mMRPConfig.mIdleRetransTimeout = idleRetransTimeout;
    }
    void SetMRPActiveRetransTimeout(const System::Clock::Milliseconds32 activeRetransTimeout)
    {
        mMRPConfig.mActiveRetransTimeout = activeRetransTimeout;
    }
    void SetMRPActiveThresholdTime(const System::Clock::Milliseconds16 activeThresholdTime)
    {
        mMRPConfig.mActiveThresholdTime = activeThresholdTime;
    }

    const Optional<uint16_t> & GetDataModelRevision() const { return mDataModelRevision; }
    void SetDataModelRevision(const uint16_t dataModelRevision) { mDataModelRevision = MakeOptional(dataModelRevision); }

    const Optional<uint16_t> & GetInteractionModelRevision() const { return mInteractionModelRevision; }
    void SetInteractionModelRevision(const uint16_t interactionModelRevision)
    {
        mInteractionModelRevision = MakeOptional(interactionModelRevision);
    }

    const Optional<uint32_t> & GetSpecificationVersion() const { return mSpecificationVersion; }
    void SetSpecificationVersion(const uint32_t specificationVersion)
    {
        mSpecificationVersion = MakeOptional(specificationVersion);
    }

    uint16_t GetMaxPathsPerInvoke() const { return mMaxPathsPerInvoke; }
    void SetMaxPathsPerInvoke(const uint16_t maxPathsPerInvoke) { mMaxPathsPerInvoke = maxPathsPerInvoke; }

private:
    ReliableMessageProtocolConfig mMRPConfig;
    // For legacy reasons if we do not get DataModelRevision it means either 16 or 17. But there isn't
    // a way to know for certain.
    Optional<uint16_t> mDataModelRevision;
    // For legacy reasons if we do not get InteractionModelRevision it means either 10 or 11. But there
    // isn't a way to know for certain.
    Optional<uint16_t> mInteractionModelRevision;
    // For legacy reasons if we do not get SpecificationVersion it means that version is less than
    // 0x01030000. But there isn't a way to know for certain.
    Optional<uint32_t> mSpecificationVersion;
    // When maxPathsPerInvoke is not provided legacy is always 1
    uint16_t mMaxPathsPerInvoke = 1;
};

} // namespace chip
