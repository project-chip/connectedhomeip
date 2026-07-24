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
#include <app-common/zap-generated/attributes/Accessors.h>
#include <app-common/zap-generated/ids/Attributes.h>
#include <app-common/zap-generated/ids/Clusters.h>
#include <platform/PlatformManager.h>

// Color support here drives the code-driven Color Control cluster through its ColorControlServer facade.
// That facade (CodegenIntegration.cpp) is compiled only when the app's data model includes Color Control,
// so guard all color code on the generated per-endpoint presence macro (from endpoint_config.h, pulled in
// via attribute-storage.h above). This keeps this shared header linkable in apps/device types that omit
// Color Control but still build the Lighting RPC service (e.g. non-color chef devices).
#if defined(MATTER_DM_COLOR_CONTROL_CLUSTER_SERVER_ENDPOINT_COUNT) && (MATTER_DM_COLOR_CONTROL_CLUSTER_SERVER_ENDPOINT_COUNT > 0)
#define CHIP_RPC_LIGHTING_SUPPORTS_COLOR 1
#include <app/clusters/color-control-server/color-control-server.h>
#else
#define CHIP_RPC_LIGHTING_SUPPORTS_COLOR 0
#endif

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
        RETURN_STATUS_IF_NOT_OK(app::Clusters::OnOff::Attributes::OnOff::Set(1, on));
        if (mSupportLevel && request.has_level)
        {
            // Clip level to max
            uint8_t level = std::min(request.level, static_cast<uint32_t>(std::numeric_limits<uint8_t>::max()));
            RETURN_STATUS_IF_NOT_OK(app::Clusters::LevelControl::Attributes::CurrentLevel::Set(kEndpoint, level));
        }

        if (mSupportColor && request.has_color)
        {
#if CHIP_RPC_LIGHTING_SUPPORTS_COLOR
            constexpr uint32_t kColorMax = 0xFE;
            // Clip color to max
            uint8_t hue        = std::min(request.color.hue, kColorMax);
            uint8_t saturation = std::min(request.color.saturation, kColorMax);
            // ColorControl is code-driven and has no direct hue/saturation attribute setter (hue is
            // transition-only). Drive the color through the legacy ColorControlServer facade so we do not
            // depend on the internal cluster type; an immediate (transitionTime = 0) MoveToHueAndSaturation
            // applies the requested color.
            RETURN_STATUS_IF_NOT_OK(ColorControlServer::Instance().moveToHueAndSaturationCommand(
                kEndpoint, hue, saturation, /*transitionTime=*/0, /*optionsMask=*/{}, /*optionsOverride=*/{},
                /*isEnhanced=*/false));
#endif // CHIP_RPC_LIGHTING_SUPPORTS_COLOR
        }
        return pw::OkStatus();
    }

    virtual pw::Status Get(const pw_protobuf_Empty & request, chip_rpc_LightingState & response)
    {
        DeviceLayer::StackLock lock;

        bool on;
        app::DataModel::Nullable<uint8_t> level;
        RETURN_STATUS_IF_NOT_OK(app::Clusters::OnOff::Attributes::OnOff::Get(1, &on));
        response.on = on;

        if (mSupportLevel)
        {
            RETURN_STATUS_IF_NOT_OK(app::Clusters::LevelControl::Attributes::CurrentLevel::Get(1, level));
            if (!level.IsNull())
            {
                response.level     = level.Value();
                response.has_level = true;
            }
        }

        if (mSupportColor)
        {
#if CHIP_RPC_LIGHTING_SUPPORTS_COLOR
            // ColorControl is code-driven and no longer exposes generated Ember Get() accessors; read the
            // live hue/saturation through the legacy ColorControlServer facade (out-param + Status shape) so
            // we do not depend on the internal cluster type.
            uint8_t hue        = 0;
            uint8_t saturation = 0;
            RETURN_STATUS_IF_NOT_OK(ColorControlServer::Instance().GetCurrentHue(kEndpoint, hue));
            RETURN_STATUS_IF_NOT_OK(ColorControlServer::Instance().GetCurrentSaturation(kEndpoint, saturation));
            response.color.hue        = hue;
            response.color.saturation = saturation;
            response.has_color        = true;
#endif // CHIP_RPC_LIGHTING_SUPPORTS_COLOR
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
