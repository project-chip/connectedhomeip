/*
 *    Copyright (c) 2025 Project CHIP Authors
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

#include <access/Privilege.h>
#include <app/ConcreteClusterPath.h>
#include <app/server-cluster/ServerClusterInterface.h>
#include <lib/core/CHIPError.h>

#include <optional>

namespace chip {
namespace app {

/// Provides an implementation of most methods for a `ServerClusterInterface`
/// to make it easier to implement spec-compliant classes.
///
/// In particular it does:
///   - handles a SINGLE cluster path that is set at construction time
///   - maintains a data version and provides `IncreaseDataVersion`. Ensures this
///     version is spec-compliant initialized (with a random value)
///   - Provides default implementations for most virtual methods EXCEPT:
///       - ReadAttribute (since that one needs to handle featuremap and revision)
///
class DefaultServerCluster : public ServerClusterInterface
{
public:
    DefaultServerCluster(const ConcreteClusterPath & path);
    ~DefaultServerCluster() override = default;

    //////////////////////////// ServerClusterInterface implementation ////////////////////////////////////////

    /// Startup allows only a single initialization per cluster and will
    /// fail with CHIP_ERROR_ALREADY_INITIALIZED if the object has already
    /// been initialized.
    ///
    /// Call Shutdown to de-initialize the object.
    CHIP_ERROR Startup(ServerClusterContext & context) override;
    void Shutdown() override;

    [[nodiscard]] Span<const ConcreteClusterPath> GetPaths() const override { return { &mPath, 1 }; }

    [[nodiscard]] DataVersion GetDataVersion(const ConcreteClusterPath &) const override { return mDataVersion; }
    [[nodiscard]] BitFlags<DataModel::ClusterQualityFlags> GetClusterFlags(const ConcreteClusterPath &) const override;

    /// Default implementation errors out with an unsupported write on every attribute.
    DataModel::ActionReturnStatus WriteAttribute(const DataModel::WriteAttributeRequest & request,
                                                 AttributeValueDecoder & decoder) override;

    /// Must only be implemented if support for any non-global attributes
    /// is required.
    ///
    /// Default implementation just returns the global attributes required by the API contract.
    CHIP_ERROR Attributes(const ConcreteClusterPath & path, ReadOnlyBufferBuilder<DataModel::AttributeEntry> & builder) override;

    /// Must only be implemented if event readability is relevant
    CHIP_ERROR EventInfo(const ConcreteEventPath & path, DataModel::EventEntry & eventInfo) override
    {
        eventInfo.readPrivilege = Access::Privilege::kView;
        return CHIP_NO_ERROR;
    }

    ///////////////////////////////////// Command Support /////////////////////////////////////////////////////////

    /// Must only be implemented if commands are supported by the cluster
    ///
    /// Default implementation errors out with an UnsupportedCommand error.
    std::optional<DataModel::ActionReturnStatus> InvokeCommand(const DataModel::InvokeRequest & request,
                                                               chip::TLV::TLVReader & input_arguments,
                                                               CommandHandler * handler) override;

    /// Must only be implemented if commands are supported by the cluster
    ///
    /// Default implementation is a NOOP (no list items generated)
    CHIP_ERROR AcceptedCommands(const ConcreteClusterPath & path,
                                ReadOnlyBufferBuilder<DataModel::AcceptedCommandEntry> & builder) override;

    /// Must only be implemented if commands that return values are supported by the cluster.
    ///
    /// Default implementation is a NOOP (no list items generated)
    CHIP_ERROR GeneratedCommands(const ConcreteClusterPath & path, ReadOnlyBufferBuilder<CommandId> & builder) override;

    /// Returns all global attributes that the spec defines in `7.13 Global Elements / Table 93: Global Attributes`
    static Span<const DataModel::AttributeEntry> GlobalAttributes();

protected:
    const ConcreteClusterPath mPath;
    ServerClusterContext * mContext = nullptr;

    void IncreaseDataVersion() { mDataVersion++; }

    /// Marks that a specific attribute has changed value
    ///
    /// This increases cluster data version and if a cluster context is available it will
    /// notify that the attribute has changed.
    void NotifyAttributeChanged(AttributeId attributeId);

private:
    DataVersion mDataVersion; // will be random-initialized as per spec
};

} // namespace app
} // namespace chip
