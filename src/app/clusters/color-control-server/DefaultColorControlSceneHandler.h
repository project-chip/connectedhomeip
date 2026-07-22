/**
 *
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

#pragma once

#include <cstdint>

#include "ColorControlCluster.h"
#include "ColorControlColorState.h"
#include <lib/support/TypeTraits.h> // chip::to_underlying

#ifdef MATTER_DM_PLUGIN_SCENES_MANAGEMENT
#include <app/clusters/scenes-server/CodegenAttributeValuePairValidator.h> // nogncheck
#include <app/clusters/scenes-server/SceneHandlerImpl.h>                   // nogncheck
#endif

namespace chip::app::Clusters {
// Provided by CodegenIntegration: resolves the ColorControlCluster instance for an endpoint.
ColorControlCluster * FindClusterOnEndpoint(EndpointId endpoint);
} // namespace chip::app::Clusters

#if defined(MATTER_DM_PLUGIN_SCENES_MANAGEMENT) && CHIP_CONFIG_SCENES_USE_DEFAULT_HANDLERS
namespace chip::app::Clusters {

// The handler lives in chip::app::Clusters so that chip / chip::app / chip::app::Clusters names
// (scenes::, EndpointId, ByteSpan, DataModel::, ScenesManagement::, ...) resolve through enclosing-namespace
// lookup. ColorControl attribute/enum/color-state names are pulled in per-function via `using namespace
// ColorControl;` (a namespace-scope using-directive in a header trips the lint check).

class DefaultColorControlSceneHandler : public scenes::DefaultSceneHandlerImpl
{
public:
    // As per spec, 9 attributes are scenable in the color control cluster. If new scenable
    // attributes are added, this value should be updated.
    static constexpr uint8_t kColorControlScenableAttributesCount = 10;

    DefaultColorControlSceneHandler() : scenes::DefaultSceneHandlerImpl(scenes::CodegenAttributeValuePairValidator::Instance()) {}
    ~DefaultColorControlSceneHandler() override = default;

    // Supports ColorControl on any endpoint that has a ColorControlCluster instance.
    bool SupportsCluster(EndpointId endpoint, ClusterId clusterId) override
    {
        return clusterId == ColorControl::Id && FindClusterOnEndpoint(endpoint) != nullptr;
    }

    /// @brief Serialize the cluster's current color state into a scene EFS.
    CHIP_ERROR SerializeSave(EndpointId endpoint, ClusterId clusterId, MutableByteSpan & serializedBytes) override
    {
        using namespace ColorControl; // Feature, Attributes, EnhancedColorModeEnum, ...
        using AttributeValuePair = ScenesManagement::Structs::AttributeValuePairStruct::Type;

        auto * cluster = FindClusterOnEndpoint(endpoint);
        VerifyOrReturnError(cluster != nullptr, CHIP_ERROR_NOT_FOUND);

        AttributeValuePair pairs[kColorControlScenableAttributesCount];
        size_t attributeCount = 0;

        // Values are read from the cluster (the source of truth); no Ember access, no clamping,
        // and no mutation (SerializeSave must never change the live color).
        if (cluster->HasFeature(Feature::kXy))
        {
            AddAttributeValuePair<uint16_t>(pairs, Attributes::CurrentX::Id, cluster->CurrentX(), attributeCount);
            AddAttributeValuePair<uint16_t>(pairs, Attributes::CurrentY::Id, cluster->CurrentY(), attributeCount);
        }
        if (cluster->HasFeature(Feature::kHueAndSaturation))
        {
            // When EnhancedHue is supported, EnhancedCurrentHue is the 16-bit superset and
            // CurrentHue is just a projection of it (hue == enhancedHue >> 8).
            if (cluster->HasFeature(Feature::kEnhancedHue))
            {
                AddAttributeValuePair<uint16_t>(pairs, Attributes::EnhancedCurrentHue::Id, cluster->EnhancedHue(), attributeCount);
            }
            else
            {
                AddAttributeValuePair<uint8_t>(pairs, Attributes::CurrentHue::Id, cluster->CurrentHue(), attributeCount);
            }
            AddAttributeValuePair<uint8_t>(pairs, Attributes::CurrentSaturation::Id, cluster->Saturation(), attributeCount);
        }
        if (cluster->HasFeature(Feature::kColorLoop))
        {
            AddAttributeValuePair<uint8_t>(pairs, Attributes::ColorLoopActive::Id, cluster->ColorLoopActive(), attributeCount);
            AddAttributeValuePair<uint8_t>(pairs, Attributes::ColorLoopDirection::Id, cluster->ColorLoopDirection(),
                                           attributeCount);
            AddAttributeValuePair<uint16_t>(pairs, Attributes::ColorLoopTime::Id, cluster->ColorLoopTime(), attributeCount);
        }
        if (cluster->HasFeature(Feature::kColorTemperature))
        {
            AddAttributeValuePair<uint16_t>(pairs, Attributes::ColorTemperatureMireds::Id, cluster->ColorTempMireds(),
                                            attributeCount);
        }

        // EnhancedColorMode is mandatory and disambiguates which mode the scene captured (§3.2.7.1).
        AddAttributeValuePair<uint8_t>(pairs, Attributes::EnhancedColorMode::Id,
                                       static_cast<uint8_t>(to_underlying(cluster->GetEnhancedColorMode())), attributeCount);

        DataModel::List<AttributeValuePair> attributeValueList(pairs, attributeCount);
        return EncodeAttributeValueList(attributeValueList, serializedBytes);
    }

    /// @brief Apply a stored scene: decode the EFS and hand the target to the cluster.
    CHIP_ERROR ApplyScene(EndpointId endpoint, ClusterId, const ByteSpan & serializedBytes,
                          scenes::TransitionTimeMs timeMs) override
    {
        using namespace ColorControl; // Attributes, EnhancedColorModeEnum, ColorValue, XYColor, ...

        auto * cluster = FindClusterOnEndpoint(endpoint);
        VerifyOrReturnError(cluster != nullptr, CHIP_ERROR_NOT_FOUND);

        DataModel::DecodableList<ScenesManagement::Structs::AttributeValuePairStruct::DecodableType> attributeValueList;
        ReturnErrorOnFailure(DecodeAttributeValueList(serializedBytes, attributeValueList));

        size_t attributeCount = 0;
        ReturnErrorOnFailure(attributeValueList.ComputeSize(&attributeCount));
        VerifyOrReturnError(attributeCount <= kColorControlScenableAttributesCount, CHIP_ERROR_BUFFER_TOO_SMALL);

        // Decode into flat locals. We build the ColorValue ONCE, after the loop: the EFS carries
        // values for several modes in arbitrary order, and a std::variant holds only one alternative,
        // so assigning per-case would drop fields (X then Y) and could build the wrong alternative
        // before EnhancedColorMode is even seen. No clamping here — the cluster clamps in HandleApplyScene.
        auto targetColorMode = EnhancedColorModeEnum::kCurrentHueAndCurrentSaturation;
        uint16_t x = 0, y = 0, enhancedHue = 0, mireds = 0;
        uint8_t hue = 0, saturation = 0;
        bool hasCurrentHue = false;
        ColorLoopState loop;

        auto it = attributeValueList.begin();
        while (it.Next())
        {
            auto & p = it.GetValue();
            switch (p.attributeID)
            {
            case Attributes::CurrentX::Id:
                VerifyOrReturnError(p.valueUnsigned16.HasValue(), CHIP_ERROR_INVALID_ARGUMENT);
                x = p.valueUnsigned16.Value();
                break;
            case Attributes::CurrentY::Id:
                VerifyOrReturnError(p.valueUnsigned16.HasValue(), CHIP_ERROR_INVALID_ARGUMENT);
                y = p.valueUnsigned16.Value();
                break;
            case Attributes::EnhancedCurrentHue::Id:
                VerifyOrReturnError(p.valueUnsigned16.HasValue(), CHIP_ERROR_INVALID_ARGUMENT);
                enhancedHue = p.valueUnsigned16.Value();
                break;
            case Attributes::CurrentHue::Id:
                // HS-only devices scene the 8-bit hue directly (they have no EnhancedCurrentHue).
                VerifyOrReturnError(p.valueUnsigned8.HasValue(), CHIP_ERROR_INVALID_ARGUMENT);
                hue           = p.valueUnsigned8.Value();
                hasCurrentHue = true;
                break;
            case Attributes::CurrentSaturation::Id:
                VerifyOrReturnError(p.valueUnsigned8.HasValue(), CHIP_ERROR_INVALID_ARGUMENT);
                saturation = p.valueUnsigned8.Value();
                break;
            case Attributes::ColorTemperatureMireds::Id:
                VerifyOrReturnError(p.valueUnsigned16.HasValue(), CHIP_ERROR_INVALID_ARGUMENT);
                mireds = p.valueUnsigned16.Value();
                break;
            case Attributes::ColorLoopActive::Id:
                VerifyOrReturnError(p.valueUnsigned8.HasValue(), CHIP_ERROR_INVALID_ARGUMENT);
                loop.active = p.valueUnsigned8.Value();
                break;
            case Attributes::ColorLoopDirection::Id:
                VerifyOrReturnError(p.valueUnsigned8.HasValue(), CHIP_ERROR_INVALID_ARGUMENT);
                loop.direction = p.valueUnsigned8.Value();
                break;
            case Attributes::ColorLoopTime::Id:
                VerifyOrReturnError(p.valueUnsigned16.HasValue(), CHIP_ERROR_INVALID_ARGUMENT);
                loop.time = p.valueUnsigned16.Value();
                break;
            case Attributes::EnhancedColorMode::Id:
                VerifyOrReturnError(p.valueUnsigned8.HasValue(), CHIP_ERROR_INVALID_ARGUMENT);
                VerifyOrReturnError(p.valueUnsigned8.Value() <=
                                        to_underlying(EnhancedColorModeEnum::kEnhancedCurrentHueAndCurrentSaturation),
                                    CHIP_ERROR_INVALID_ARGUMENT);
                targetColorMode = static_cast<EnhancedColorModeEnum>(p.valueUnsigned8.Value());
                break;
            default:
                return CHIP_ERROR_INVALID_ARGUMENT;
            }
        }
        ReturnErrorOnFailure(it.GetStatus());

        // Build the single alternative matching the scene's declared mode.
        // Since color loop can be active in any mode we will initilaize it separately
        ColorValue target;
        switch (targetColorMode)
        {
        case EnhancedColorModeEnum::kCurrentXAndCurrentY:
            target = XYColor{ .x = x, .y = y };
            break;
        case EnhancedColorModeEnum::kEnhancedCurrentHueAndCurrentSaturation:
            target = EnhancedHueSatColor{ .enhancedHue = enhancedHue, .saturation = saturation };
            break;
        case EnhancedColorModeEnum::kCurrentHueAndCurrentSaturation:
            // Prefer a directly-scened CurrentHue (HS-only devices); otherwise project from the
            // 16-bit EnhancedCurrentHue that EnhancedHue devices save.
            target = HueSatColor{ .hue = hasCurrentHue ? hue : static_cast<uint8_t>(enhancedHue >> 8), .saturation = saturation };
            break;
        case EnhancedColorModeEnum::kColorTemperatureMireds:
            target = CTColor{ .mireds = mireds };
            break;
        default:
            return CHIP_ERROR_INVALID_ARGUMENT;
        }

        VerifyOrReturnError(cluster->SupportsMode(targetColorMode), CHIP_ERROR_INVALID_ARGUMENT);

        // The cluster clamps values, switches mode, (re)starts the color loop if active, and
        // starts the transition over the scene's transition time.
        return cluster->HandleApplyScene(targetColorMode, target, loop, static_cast<uint16_t>(timeMs / 100));
    }

private:
    /// @brief Append one attribute/value pair to the array and advance the count.
    /// @brief Helper function to add an attribute value pair to the attribute value pair array in the color control SceneHandler
    /// @param pairs list of attribute value pairs
    /// @param id attribute id
    /// @param value attribute value
    /// @param attributeCount number of attributes in the list, incremented by this function, used to keep track of how many
    /// attributes from the array are being used for the list to encode
    template <typename Type>
    void AddAttributeValuePair(ScenesManagement::Structs::AttributeValuePairStruct::Type * pairs, AttributeId id, Type value,
                               size_t & attributeCount)
    {
        static_assert((std::is_same_v<Type, uint8_t>) || (std::is_same_v<Type, uint16_t>), "Type must be uint8_t or uint16_t");

        pairs[attributeCount].attributeID = id;
        if constexpr (std::is_same_v<Type, uint8_t>)
        {
            pairs[attributeCount].valueUnsigned8.SetValue(value);
        }
        else if constexpr (std::is_same_v<Type, uint16_t>)
        {
            pairs[attributeCount].valueUnsigned16.SetValue(value);
        }
        attributeCount++;
    }
};

inline DefaultColorControlSceneHandler sColorControlSceneHandler;

} // namespace chip::app::Clusters
#endif // defined(MATTER_DM_PLUGIN_SCENES_MANAGEMENT) && CHIP_CONFIG_SCENES_USE_DEFAULT_HANDLERS
