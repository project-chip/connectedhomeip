/*
 * Copyright (c) 2026 Project CHIP Authors
 * All rights reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#pragma once

#include <app/data-model/DecodableList.h>
#include <app/data-model/Nullable.h>
#include <clusters/DynamicLighting/Structs.h>
#include <lib/core/DataModelTypes.h>
#include <lib/core/TLV.h>

namespace chip::app::Clusters::DynamicLighting {

/**
 * Delegate interface for receiving attribute value notifications from a remote
 * Dynamic Lighting server cluster.
 *
 * Implement this interface and register an instance via
 * DynamicLightingClient::SetDelegate to receive decoded attribute values
 * whenever HandleAttributeData is called.
 */
class DynamicLightingClientDelegate
{
public:
    virtual ~DynamicLightingClientDelegate() = default;

    /**
     * Called when the AvailableEffects attribute value is received from the server.
     *
     * @param availableEffects  The decoded list of effects supported by the server.
     */
    virtual void
    OnAvailableEffectsChanged(const DataModel::DecodableList<Structs::EffectStruct::DecodableType> & availableEffects)
    {}

    /**
     * Called when the CurrentEffectID attribute value is received from the server.
     *
     * @param currentEffectId  The decoded current effect ID, or null when no effect is active.
     */
    virtual void OnCurrentEffectIDChanged(const DataModel::Nullable<uint16_t> & currentEffectId) {}

    /**
     * Called when the CurrentSpeed attribute value is received from the server.
     *
     * @param currentSpeed  The decoded current effect speed, or null when no effect is active.
     */
    virtual void OnCurrentSpeedChanged(const DataModel::Nullable<uint16_t> & currentSpeed) {}
};

/**
 * Client-side handler for the Dynamic Lighting cluster (cluster ID 0x0305).
 *
 * Decodes attribute data received from a remote Dynamic Lighting server and
 * dispatches the decoded values to a registered DynamicLightingClientDelegate.
 *
 * Typical usage:
 *   1. Instantiate DynamicLightingClient and call SetDelegate with an
 *      application-provided DynamicLightingClientDelegate.
 *   2. Subscribe to (or read) the desired attributes on the remote server
 *      using the SDK's ReadClient infrastructure.
 *   3. For each attribute element received in an attribute report, call
 *      HandleAttributeData to decode it and notify the delegate.
 */
class DynamicLightingClient
{
public:
    DynamicLightingClient()  = default;
    ~DynamicLightingClient() = default;

    /**
     * Sets the delegate to receive decoded attribute value notifications.
     *
     * The caller must ensure the delegate outlives this DynamicLightingClient
     * instance, or call SetDelegate(nullptr) before the delegate is destroyed.
     */
    void SetDelegate(DynamicLightingClientDelegate * delegate) { mDelegate = delegate; }

    /**
     * Returns the currently registered delegate, or nullptr if none is set.
     */
    DynamicLightingClientDelegate * GetDelegate() const { return mDelegate; }

    /**
     * Decodes a single attribute value from a TLVReader and notifies the delegate.
     *
     * The reader must be positioned at the attribute value element (i.e. after
     * a successful call to TLVReader::Next() has advanced it to the element).
     *
     * Unknown attribute IDs are silently ignored.  If no delegate is registered
     * the function returns CHIP_NO_ERROR without performing any decoding.
     *
     * @param attributeId  The ID of the attribute whose data is in @p reader.
     * @param reader       A TLVReader positioned at the attribute value element.
     *
     * @return CHIP_NO_ERROR on success, or a decode error if the TLV is malformed.
     */
    CHIP_ERROR HandleAttributeData(AttributeId attributeId, TLV::TLVReader & reader);

private:
    DynamicLightingClientDelegate * mDelegate = nullptr;
};

} // namespace chip::app::Clusters::DynamicLighting
