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

/**
 *    @file
 *      This file defines a Session Parameters that are passed during session estalishment.
 *
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
    static constexpr size_t kMrpConfigEstimatedTLVSize =
        TLV::EstimateStructOverhead(sizeof(uint32_t), sizeof(uint32_t), sizeof(uint16_t));

    static constexpr size_t kEstimatedTLVSize = kMrpConfigEstimatedTLVSize +
        TLV::EstimateStructOverhead(sizeof(uint16_t), sizeof(uint16_t), sizeof(uint32_t), sizeof(uint16_t));

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

    const Optional<uint16_t> & GetDataModelRev() const { return mDataModelRev; }
    void SetDataModelRev(const uint16_t dataModelRev) { mDataModelRev = MakeOptional(dataModelRev); }

    const Optional<uint16_t> & GetInteractionModelRev() const { return mInteractionModelRev; }
    void SetInteractionModelRev(const uint16_t interactionModelRev) { mInteractionModelRev = MakeOptional(interactionModelRev); }

    const Optional<uint32_t> & GetSpecificationVersion() const { return mSpecificationVersion; }
    void SetSpecificationVersion(const uint32_t specVersion) { mSpecificationVersion = MakeOptional(specVersion); }

    uint16_t GetMaxPathPerInvoke() const { return mMaxPathPerInvoke; }
    void SetMaxPathPerInvoke(const uint16_t maxPathPerInvoke) { mMaxPathPerInvoke = maxPathPerInvoke; }

private:
    ReliableMessageProtocolConfig mMRPConfig;
    // For legacy reason if we do not get DataModelRev it means that either 16 or 17. But there isn't
    // a way to know for certain.
    Optional<uint16_t> mDataModelRev = NullOptional;
    // For legacy reason if we do not get InterationModelRev it means that either 10 or 11. But there
    // isn't a way to know for certain.
    Optional<uint16_t> mInteractionModelRev = NullOptional;
    // For legacy reason if we do not get Specification Version it means that version is less than
    // 0x01030000. But there isn't a way to know for certain.
    Optional<uint32_t> mSpecificationVersion = NullOptional;
    // When maxPathPerInvoke is not provide legacy is always 1
    uint16_t mMaxPathPerInvoke = 1;
};

} // namespace chip
