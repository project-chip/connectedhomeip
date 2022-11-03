/*
 * SPDX-FileCopyrightText: (c) 2021 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "application-basic/ApplicationBasicManager.h"
#include "application-launcher/ApplicationLauncherManager.h"
#include "audio-output/AudioOutputManager.h"
#include "channel/ChannelManager.h"
#include "content-launcher/ContentLauncherManager.h"
#include "media-input/MediaInputManager.h"
#include "target-navigator/TargetNavigatorManager.h"
#include "wake-on-lan/WakeOnLanManager.h"

#include <app-common/zap-generated/attribute-id.h>
#include <app-common/zap-generated/cluster-id.h>
#include <app-common/zap-generated/cluster-objects.h>
#include <app-common/zap-generated/ids/Attributes.h>
#include <app-common/zap-generated/ids/Clusters.h>
#include <app/AttributeAccessInterface.h>
#include <app/util/attribute-storage.h>

using namespace chip;

namespace {
template <typename Manager, typename AttrTypeInfo, CHIP_ERROR (Manager::*Getter)(uint16_t, app::AttributeValueEncoder &)>
class TvAttrAccess : public app::AttributeAccessInterface
{
public:
    TvAttrAccess() : app::AttributeAccessInterface(Optional<EndpointId>::Missing(), AttrTypeInfo::GetClusterId()) {}

    CHIP_ERROR Read(const app::ConcreteReadAttributePath & aPath, app::AttributeValueEncoder & aEncoder) override
    {
        if (aPath.mAttributeId == AttrTypeInfo::GetAttributeId())
        {
            return (Manager().*Getter)(aPath.mEndpointId, aEncoder);
        }

        return CHIP_NO_ERROR;
    }
};

} // anonymous namespace
