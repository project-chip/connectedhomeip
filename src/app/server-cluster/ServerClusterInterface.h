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

#include <app/AttributeValueDecoder.h>
#include <app/AttributeValueEncoder.h>
#include <app/CommandHandler.h>
#include <app/ConcreteClusterPath.h>
#include <app/data-model-provider/ActionReturnStatus.h>
#include <app/data-model-provider/MetadataList.h>
#include <app/data-model-provider/MetadataTypes.h>
#include <app/data-model-provider/OperationTypes.h>
#include <lib/core/CHIPError.h>
#include <lib/core/DataModelTypes.h>
#include <lib/support/BitFlags.h>

namespace chip {
namespace app {

namespace detail {
template <typename SELF>
class IntrusiveSingleLinkedList
{
public:
    IntrusiveSingleLinkedList() : mNext(static_cast<SELF *>(this)) {}

    // IMPLEMENTATION DETAILS:
    //   Since `mNext == this` is used as a marker for "is in a list",
    //   the assignment of these interfaces is overloaded even for the move operator.
    IntrusiveSingleLinkedList(IntrusiveSingleLinkedList && other) :
        mNext((other.mNext == &other) ? static_cast<SELF *>(this) : other.mNext)
    {}
    IntrusiveSingleLinkedList(const IntrusiveSingleLinkedList & other) :
        mNext((other.mNext == &other) ? static_cast<SELF *>(this) : other.mNext)
    {}
    IntrusiveSingleLinkedList & operator=(const IntrusiveSingleLinkedList & other)
    {
        if (&other != this)
        {
            mNext = (other.mNext == &other) ? static_cast<SELF *>(this) : other.mNext;
        }
        return *this;
    }
    IntrusiveSingleLinkedList & operator=(const IntrusiveSingleLinkedList && other)
    {
        mNext = (other.mNext == &other) ? static_cast<SELF *>(this) : other.mNext;
        return *this;
    }

    // Internal details for processing

    /// This is NOT a STABLE API.
    ///
    /// Determines if this object is part of a linked list already or not.
    [[nodiscard]] bool IsInList() const { return (mNext != this); }

    /// This is NOT a STABLE API.
    ///
    /// Marks this object as not being part of a linked list
    void SetNotInList() { mNext = static_cast<SELF *>(this); }

    /// This is NOT a STABLE API. It allows interfaces to be stored within a registry, however
    /// usage of this method MAY be changed in the future.
    ///
    /// Returns a "next" pointer when the ServerClusterInterface is assumed to be
    /// part of a SINGLE linked list.
    [[nodiscard]] SELF * GetNextListItem() const
    {
        VerifyOrDie(mNext != this);
        return mNext;
    }

    /// This is NOT a STABLE API. It allows interfaces to be stored within a registry, however
    /// usage of this method MAY be changed in the future.
    ///
    /// Sets the "next" pointer when the SELF is assumed to be
    /// part of a SINGLE linked list.
    ///
    /// Returns the old value of "next"
    SELF * SetNextListItem(SELF * value)
    {
        VerifyOrDie(value != this);
        auto previousValue = mNext;
        mNext              = value;
        return previousValue;
    }

private:
    // The mNext pointer has 2 (!) states:
    //  - `this` means this item is NOT part of a linked list (used since we would generally
    //    not allow loops)
    //  - OTHER values, including nullptr, when this is part of a REAL LIST
    SELF * mNext; /* = this (in constructor) */
};

} // namespace detail

/// Defines an active cluster on an endpoint.
///
/// Provides metadata as well as interaction processing (attribute read/write and command handling).
///
/// Implementation note:
///   - this class is highly coupled with ServerClusterInterfaceRegistry. The fact that it
///     derives from `detail::IntrusiveSingleLinkedList` is NOT a public API and is only done
///     for `ServerClusterInterfaceRegistry` usage. Code may be updated to support different
///     implementations for storing interface.
class ServerClusterInterface : public detail::IntrusiveSingleLinkedList<ServerClusterInterface>
{
public:
    ServerClusterInterface();
    virtual ~ServerClusterInterface() = default;

    ///////////////////////////////////// Cluster Metadata Support //////////////////////////////////////////////////
    [[nodiscard]] virtual ClusterId GetClusterId() const = 0;

    // Every cluster must have a data version. Base class implementation to avoid
    // code duplication
    //
    // SPEC - 7.10.3. Cluster Data Version
    //   A cluster data version is a metadata increment-only counter value, maintained for each cluster instance.
    //   [...]
    //   A cluster data version SHALL increment or be set (wrap) to zero
    //   if incrementing would exceed its maximum value. A cluster data version
    //   SHALL be maintained for each cluster instance.
    //   [...]
    //   A cluster data version SHALL be incremented if any attribute data changes.
    //
    [[nodiscard]] DataVersion GetDataVersion() const { return mDataVersion; }
    void IncreaseDataVersion() { mDataVersion++; }

    /// Cluster flags can be overridden, however most clusters likely have a default of "nothing special".
    ///
    /// Default implementation returns a 0/empty quality list.
    [[nodiscard]] virtual BitFlags<DataModel::ClusterQualityFlags> GetClusterFlags() const;

    ///////////////////////////////////// Attribute Support ////////////////////////////////////////////////////////

    /// ReadAttribute MUST be done on a valid attribute path. `request.path` is expected to have `GetClusterId` as the cluster
    /// id as well as an attribute that is included in a `Attributes` call.
    ///
    /// This MUST HANDLE the following global attributes:
    ///   - FeatureMap::Id      - generally 0 as a default
    ///   - ClusterRevision::Id - this is implementation defined
    ///
    /// This call WILL NOT be called for attributes that can be built out of cluster metadata.
    /// Specifically this WILL NOT be called (and does not need to implement handling for) the
    /// following attribute IDs:
    ///     - AcceptedCommandList::Id
    ///     - AttributeList::Id
    ///     - GeneratedCommandList::Id
    virtual DataModel::ActionReturnStatus ReadAttribute(const DataModel::ReadAttributeRequest & request,
                                                        AttributeValueEncoder & encoder) = 0;

    /// WriteAttribute MUST be done on a valid attribute path. `request.path` is expected to have `GetClusterId` as the cluster
    /// id as well as an attribute that is included in a `Attributes` call.
    ///
    /// Must only be implemented if writable attributes are defined.
    ///
    /// Default implementation errors out with an unsupported write on every attribute.
    virtual DataModel::ActionReturnStatus WriteAttribute(const DataModel::WriteAttributeRequest & request,
                                                         AttributeValueDecoder & decoder);

    /// Attribute list MUST contain global attributes.
    ///
    /// Specifically these attributes MUST always exist in the list for all clusters:
    ///     - ClusterRevision::Id
    ///     - FeatureMap::Id
    ///     - AcceptedCommandList::Id
    ///     - AttributeList::Id
    ///     - GeneratedCommandList::Id
    /// See SPEC 7.13 Global Elements: `Global Attributes` table
    ///
    /// Must only be implemented if support for any non-global attributes
    /// is required.
    ///
    /// Default implementation just returns the above global attributes.
    virtual CHIP_ERROR Attributes(const ConcreteClusterPath & path, DataModel::ListBuilder<DataModel::AttributeEntry> & builder);

    ///////////////////////////////////// Command Support /////////////////////////////////////////////////////////

    /// Must only be implemented if commands are supported by the cluster
    ///
    /// Default implementation errors out with an UnspportedCommand error.
    virtual std::optional<DataModel::ActionReturnStatus> Invoke(const DataModel::InvokeRequest & request,
                                                                chip::TLV::TLVReader & input_arguments, CommandHandler * handler);

    /// Must only be implemented if commands are supported by the cluster
    ///
    /// Default implementation is a NOOP (no list items generated)
    virtual CHIP_ERROR AcceptedCommands(const ConcreteClusterPath & path,
                                        DataModel::ListBuilder<DataModel::AcceptedCommandEntry> & builder);

    /// Must only be implemented if commands are supported by the cluster that return values
    ///
    /// Default implementation is a NOOP (no list items generated)
    virtual CHIP_ERROR GeneratedCommands(const ConcreteClusterPath & path, DataModel::ListBuilder<CommandId> & builder);

private:
    DataVersion mDataVersion; // will be random-initialized as per spec

    // The mNext pointer has 2 (!) states:
    //  - `this` means this item is NOT part of a linked list (used since we would generally
    //    not allow loops)
    //  - OTHER values, including nullptr, when this is part of a REAL LIST
    ServerClusterInterface * mNext; /* = this (in constructor) */
};

} // namespace app
} // namespace chip
