/*
 *
 *    Copyright (c) 2022 Project CHIP Authors
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

#include "Device.h"

#include <memory>
#include <vector>

// This file provides implementations that use dynamic storage

struct DynamicAttributeList
{
    template <typename T>
    void Add(const T & attrib)
    {
        Add(T::kId, T::kMatterType, T::kMaxSize, T::kMask);
    }

    void Add(EmberAfAttributeMetadata metadata);

    void Add(chip::AttributeId id, EmberAfAttributeType type, uint16_t maxSize, EmberAfClusterMask mask);

    std::vector<EmberAfAttributeMetadata> mAttributeDecls;
};

template <typename T>
struct DynamicCluster : public T
{
    DynamicCluster(const chip::CommandId * incomingCommands = nullptr, const chip::CommandId * outgoingCommands = nullptr) :
        mIncomingCommands(incomingCommands), mOutgoingCommands(outgoingCommands)
    {
        this->AddAllAttributes(&mAttributes);
    }

    DynamicAttributeList mAttributes;
    const chip::CommandId * mIncomingCommands;
    const chip::CommandId * mOutgoingCommands;
};

class DynamicDeviceImpl
{
public:
    DynamicDeviceImpl();

    template <typename... Types>
    DynamicDeviceImpl(Types &... clusters)
    {}

    virtual ~DynamicDeviceImpl() = default;

    void AddDeviceType(EmberAfDeviceType type);

    // Add each cluster that the device will ever use.
    DynamicDeviceImpl & AddCluster(std::unique_ptr<CommonCluster> cluster, const DynamicAttributeList & attribs,
                                   const chip::CommandId * incomingCommands, const chip::CommandId * outgoingCommands);
    DynamicDeviceImpl & AddCluster(CommonCluster * cluster, const DynamicAttributeList & attribs,
                                   const chip::CommandId * incomingCommands, const chip::CommandId * outgoingCommands);
    template <typename T>
    DynamicDeviceImpl & AddCluster(DynamicCluster<T> & cluster)
    {
        return AddCluster(&cluster, cluster.mAttributes, cluster.mIncomingCommands, cluster.mOutgoingCommands);
    }

    // After setting all options, create the device.
    Device CreateDevice();

    const std::vector<CommonCluster *> & clusters() const { return mClusterRawPtrs; }

private:
    std::vector<std::unique_ptr<CommonCluster>> mClusters;
    std::vector<CommonCluster *> mClusterRawPtrs;
    std::vector<ClusterImpl *> mClusterBaseRawPtrs;

    std::vector<EmberAfCluster> mClusterDecls;

    std::vector<chip::DataVersion> mVersions;

    std::vector<EmberAfDeviceType> mDeviceTypes;
};

class DynamicSwitchDevice : public DynamicDeviceImpl
{
public:
    DynamicSwitchDevice() { AddCluster(mBridgedDevice).AddCluster(mSwitch).AddCluster(mFixedLabel).AddCluster(mDescriptor); }
    ~DynamicSwitchDevice() override = default;

    void SetNumberOfPositions(uint8_t aNumberOfPositions)
    {
        mSwitch.mNumberOfPositions.WriteFromBridge(aNumberOfPositions, &mSwitch);
    }
    void SetCurrentPosition(uint8_t aCurrentPosition) { mSwitch.mNumberOfPositions.WriteFromBridge(aCurrentPosition, &mSwitch); }
    void SetMultiPressMax(uint8_t aMultiPressMax) { mSwitch.mNumberOfPositions.WriteFromBridge(aMultiPressMax, &mSwitch); }

    inline uint8_t GetNumberOfPositions() { return mSwitch.mNumberOfPositions.Peek(); };
    inline uint8_t GetCurrentPosition() { return mSwitch.mCurrentPosition.Peek(); };
    inline uint8_t GetMultiPressMax() { return mSwitch.mMultiPressMax.Peek(); };
    inline uint32_t GetFeatureMap() { return mSwitch.mFeatureMap.Peek(); };

    using DeviceCallback_fn = std::function<void(DynamicSwitchDevice *, chip::AttributeId)>;
    void SetChangeCallback(DeviceCallback_fn aChanged_CB)
    {
        mCallback = std::move(aChanged_CB);
        if (mCallback)
        {
            if (!mInternalCallback)
            {
                mInternalCallback = [this](CommonCluster *, chip::EndpointId, chip::ClusterId, chip::AttributeId attr,
                                           const uint8_t *) -> EmberAfStatus {
                    mCallback(this, attr);
                    return EMBER_ZCL_STATUS_SUCCESS;
                };
                mSwitch.SetCallback(&mInternalCallback);
            }
        }
        else
        {
            mSwitch.SetCallback(nullptr);
        }
    }

private:
    DynamicCluster<clusters::BridgedDeviceBasicCluster> mBridgedDevice;
    DynamicCluster<clusters::SwitchCluster> mSwitch;
    DynamicCluster<clusters::FixedLabelCluster> mFixedLabel;
    DynamicCluster<clusters::DescriptorCluster> mDescriptor;

    DeviceCallback_fn mCallback;
    PropagateWriteCB mInternalCallback;
};
