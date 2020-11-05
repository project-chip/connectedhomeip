/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
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
 *  @file
 *    This file contains definitions for a base Cluster class. This class will
 *    be derived by various ZCL clusters supported by CHIP. The objects of the
 *    ZCL cluster class will be used by Controller applications to interact with
 *    the CHIP device.
 */

#pragma once

#include <controller/CHIPDevice.h>

namespace chip {
namespace Controller {

class DLL_EXPORT ClusterBase
{
public:
    virtual ~ClusterBase() {}

    CHIP_ERROR Associate(Device * device, uint8_t endpoint);

    void Dissociate();

    const uint16_t GetClusterId() const { return mClusterId; }

protected:
    ClusterBase(uint16_t cluster) : mClusterId(cluster) {}
    const uint16_t mClusterId;
    Device * mDevice;
    uint8_t mEndpoint;
};

} // namespace Controller
} // namespace chip
