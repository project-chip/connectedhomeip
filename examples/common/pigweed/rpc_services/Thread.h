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

#include <platform/CHIPDeviceConfig.h>

#include "platform/ConnectivityManager.h"
#include "thread_service/thread_service.rpc.pb.h"

#if CHIP_ENABLE_OPENTHREAD
#include <openthread/dataset.h>
#include <platform/ThreadStackManager.h>
#endif

namespace chip {
namespace rpc {

// Implementation class for chip.rpc.Thread.
class Thread : public pw_rpc::nanopb::Thread::Service<Thread>
{
public:
    ::pw::Status GetState(const pw_protobuf_Empty & request, chip_rpc_ThreadState & response)
    {
        response.is_provisioned = DeviceLayer::ConnectivityMgr().IsThreadProvisioned();
        response.is_enabled     = DeviceLayer::ConnectivityMgr().IsThreadEnabled();
        response.is_attached    = DeviceLayer::ConnectivityMgr().IsThreadAttached();
        response.device_type    = static_cast<chip_rpc_ThreadDeviceType>(DeviceLayer::ConnectivityMgr().GetThreadDeviceType());
        return pw::OkStatus();
    }

    ::pw::Status GetNetworkInfo(const pw_protobuf_Empty & request, chip_rpc_ThreadNetworkInfo & response)
    {
#if CHIP_ENABLE_OPENTHREAD
        otInstance * otInst = DeviceLayer::ThreadStackMgrImpl().OTInstance();
        otOperationalDataset data;
        if (OT_ERROR_NONE != otDatasetGetActive(otInst, &data))
        {
            return pw::Status::NotFound();
        }

        if (data.mComponents.mIsPanIdPresent)
        {
            response.pan_id = data.mPanId;
        }
        if (data.mComponents.mIsNetworkNamePresent)
        {
            snprintf(response.network_name, sizeof(response.network_name), "%s", data.mNetworkName.m8);
        }
        if (data.mComponents.mIsChannelPresent)
        {
            response.channel = data.mChannel;
        }
        if (data.mComponents.mIsExtendedPanIdPresent)
        {
            size_t size = std::min(sizeof(response.extended_pan_id.bytes), sizeof(data.mExtendedPanId));
            memcpy(response.extended_pan_id.bytes, data.mExtendedPanId.m8, size);
            response.extended_pan_id.size = size;
        }
        return pw::OkStatus();
#else  // CHIP_ENABLE_OPENTHREAD
        return ::pw::Status::Unimplemented();
#endif // CHIP_ENABLE_OPENTHREAD
    }
};

} // namespace rpc
} // namespace chip
