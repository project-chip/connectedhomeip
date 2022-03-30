/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
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

#include "app/util/attribute-storage.h"
#include "lighting_service/lighting_service.rpc.pb.h"
#include "pigweed/rpc_services/internal/StatusUtils.h"
#include <app-common/zap-generated/attribute-id.h>
#include <app-common/zap-generated/attribute-type.h>
#include <app-common/zap-generated/cluster-id.h>
#include <platform/PlatformManager.h>

namespace chip {
namespace rpc {

class Lighting : public pw_rpc::nanopb::Lighting::Service<Lighting>
{
public:
    Lighting(bool support_level = true, bool support_color = true) : mSupportLevel(support_level), mSupportColor(support_color) {}

    virtual ~Lighting() = default;

    virtual pw::Status Set(const chip_rpc_LightingState & request, pw_protobuf_Empty & response)
    {
        DeviceLayer::StackLock lock;

        uint8_t on = request.on;
        RETURN_STATUS_IF_NOT_OK(
            emberAfWriteServerAttribute(1, ZCL_ON_OFF_CLUSTER_ID, ZCL_ON_OFF_ATTRIBUTE_ID, &on, ZCL_BOOLEAN_ATTRIBUTE_ID));
        if (mSupportLevel && request.has_level)
        {
            // Clip level to max
            uint8_t level = std::min(request.level, static_cast<uint32_t>(std::numeric_limits<uint8_t>::max()));
            RETURN_STATUS_IF_NOT_OK(emberAfWriteServerAttribute(kEndpoint, ZCL_LEVEL_CONTROL_CLUSTER_ID,
                                                                ZCL_CURRENT_LEVEL_ATTRIBUTE_ID, &level, ZCL_INT8U_ATTRIBUTE_TYPE));
        }

        if (mSupportColor && request.has_color)
        {
            constexpr uint32_t kColorMax = 0xFE;
            // Clip color to max
            uint8_t hue        = std::min(request.color.hue, kColorMax);
            uint8_t saturation = std::min(request.color.saturation, kColorMax);
            RETURN_STATUS_IF_NOT_OK(emberAfWriteServerAttribute(
                1, ZCL_COLOR_CONTROL_CLUSTER_ID, ZCL_COLOR_CONTROL_CURRENT_HUE_ATTRIBUTE_ID, &hue, ZCL_INT8U_ATTRIBUTE_TYPE));
            RETURN_STATUS_IF_NOT_OK(emberAfWriteServerAttribute(kEndpoint, ZCL_COLOR_CONTROL_CLUSTER_ID,
                                                                ZCL_COLOR_CONTROL_CURRENT_SATURATION_ATTRIBUTE_ID, &saturation,
                                                                ZCL_INT8U_ATTRIBUTE_TYPE));
        }
        return pw::OkStatus();
    }

    virtual pw::Status Get(const pw_protobuf_Empty & request, chip_rpc_LightingState & response)
    {
        DeviceLayer::StackLock lock;

        uint8_t on;
        uint8_t level;
        uint8_t hue;
        uint8_t saturation;
        RETURN_STATUS_IF_NOT_OK(emberAfReadServerAttribute(1, ZCL_ON_OFF_CLUSTER_ID, ZCL_ON_OFF_ATTRIBUTE_ID, &on, sizeof(on)));
        response.on = on;

        if (mSupportLevel)
        {
            RETURN_STATUS_IF_NOT_OK(
                emberAfReadServerAttribute(1, ZCL_LEVEL_CONTROL_CLUSTER_ID, ZCL_CURRENT_LEVEL_ATTRIBUTE_ID, &level, sizeof(level)));
            response.level     = level;
            response.has_level = true;
        }

        if (mSupportColor)
        {
            RETURN_STATUS_IF_NOT_OK(emberAfReadServerAttribute(kEndpoint, ZCL_COLOR_CONTROL_CLUSTER_ID,
                                                               ZCL_COLOR_CONTROL_CURRENT_HUE_ATTRIBUTE_ID, &hue, sizeof(hue)));
            RETURN_STATUS_IF_NOT_OK(emberAfReadServerAttribute(kEndpoint, ZCL_COLOR_CONTROL_CLUSTER_ID,
                                                               ZCL_COLOR_CONTROL_CURRENT_SATURATION_ATTRIBUTE_ID, &saturation,
                                                               sizeof(saturation)));
            response.color.hue        = hue;
            response.color.saturation = saturation;
            response.has_color        = true;
        }

        return pw::OkStatus();
    }

protected:
    static constexpr EndpointId kEndpoint = 1;
    bool mSupportLevel;
    bool mSupportColor;
};

} // namespace rpc
} // namespace chip
