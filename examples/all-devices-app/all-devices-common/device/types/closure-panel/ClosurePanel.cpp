/*
 *    Copyright (c) 2026 Project CHIP Authors
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

#include "ClosurePanel.h"
#include <device/types/closure-panel/ClosureSematicTags.h>
#include <devices/Types.h>

namespace {

CHIP_ERROR ValidateClosurePanelTagList(chip::Span<const chip::app::EndpointComposition::SemanticTag> tags)
{
    size_t panelTagCount = 0;
    for (auto & tag : tags)
    {
        VerifyOrReturnError(tag.namespaceID != chip::app::kClosureNamespaceId, CHIP_ERROR_INVALID_ARGUMENT);
        if (tag.namespaceID == chip::app::kClosurePanelNamespaceId)
        {
            ++panelTagCount;
        }
    }
    VerifyOrReturnError(panelTagCount == 1, CHIP_ERROR_INVALID_ARGUMENT);
    return CHIP_NO_ERROR;
}

} // namespace

namespace chip::app {

ClosurePanel::ClosurePanel(Clusters::ClosureDimension::ClosureDimensionClusterDelegate & dimensionDelegate, Config config) :
    SingleEndpoint(Span<const DataModel::DeviceTypeEntry>(&Device::Type::kClosurePanel, 1)), mConfig(config),
    mDimensionDelegate(dimensionDelegate)
{}

CHIP_ERROR ClosurePanel::Register(EndpointId endpoint, CodeDrivenDataModelProvider & provider, EndpointComposition composition)
{
    ReturnErrorOnFailure(ValidateClosurePanelTagList(composition.tagList));

    ReturnErrorOnFailure(RegisterDescriptor(endpoint, provider, composition));

    Clusters::ClosureDimension::ClosureDimensionCluster::Config dimensionConfig(endpoint, mDimensionDelegate);
    if (mConfig.withAccess)
    {
        dimensionConfig.WithAccess();
    }
    if (auto * translation = std::get_if<TranslationParams>(&mConfig.motion))
    {
        dimensionConfig.WithTranslation(translation->direction);
    }
    else if (auto * rotation = std::get_if<RotationParams>(&mConfig.motion))
    {
        dimensionConfig.WithRotation(rotation->axis, rotation->overflow);
    }
    else if (auto * modulation = std::get_if<ModulationParams>(&mConfig.motion))
    {
        dimensionConfig.WithModulation(modulation->type);
    }
    if (mConfig.positioning.has_value())
    {
        dimensionConfig.WithPositioning(mConfig.positioning->first, mConfig.positioning->second);
    }
    if (mConfig.motionLatching.has_value())
    {
        dimensionConfig.WithMotionLatching(mConfig.motionLatching.value());
    }

    mClosureDimensionCluster.Create(dimensionConfig);
    ReturnErrorOnFailure(provider.AddCluster(mClosureDimensionCluster.Registration()));
    ReturnErrorOnFailure(provider.AddEndpoint(mEndpointRegistration));
    return CHIP_NO_ERROR;
}

void ClosurePanel::Unregister(CodeDrivenDataModelProvider & provider)
{
    UnregisterDescriptor(provider);

    if (mClosureDimensionCluster.IsConstructed())
    {
        LogErrorOnFailure(provider.RemoveCluster(&mClosureDimensionCluster.Cluster()));
        mClosureDimensionCluster.Destroy();
    }
}

} // namespace chip::app
