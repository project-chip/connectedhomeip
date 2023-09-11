/**
 *
 * SPDX-FileCopyrightText: 2023 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <lib/core/CHIPError.h>
#include <lib/core/TLV.h>

namespace chip {
namespace scenes {

static constexpr uint8_t kInvalidPosition         = 0xff;
static constexpr uint8_t kMaxClustersPerScene     = CHIP_CONFIG_SCENES_MAX_CLUSTERS_PER_SCENE;
static constexpr uint8_t kMaxFieldBytesPerCluster = CHIP_CONFIG_SCENES_MAX_EXTENSION_FIELDSET_SIZE_PER_CLUSTER;

/// @brief A way to serialize and deserialize all cluster attribute data for a scene.
class ExtensionFieldSets
{
public:
    ExtensionFieldSets(){};
    virtual ~ExtensionFieldSets() = default;

    virtual CHIP_ERROR Serialize(TLV::TLVWriter & writer, TLV::Tag structTa) const = 0;
    virtual CHIP_ERROR Deserialize(TLV::TLVReader & reader, TLV::Tag structTa)     = 0;
    virtual void Clear()                                                           = 0;
    virtual bool IsEmpty() const                                                   = 0;
    /// @brief Gets a count of how many initialized fields sets are in the object
    /// @return The number of initialized field sets the object
    /// @note Field set refers to extension field sets, from the scene cluster (see 1.4.6.2 ExtensionFieldSet in Matter Application
    /// Clusters)
    virtual uint8_t GetFieldSetCount() const = 0;
};
} // namespace scenes
} // namespace chip
