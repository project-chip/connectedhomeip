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
