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

/// This class implements an intrusive single linked list
///
/// The class is an implementation detail for the use of ServerClusterInterfaceRegistry and
/// is NOT considered public API. No API-compatibility is guaranteed across different
/// SDK releases.
template <typename SELF>
class IntrusiveSingleLinkedList
{
public:
    IntrusiveSingleLinkedList() : mNext(static_cast<SELF *>(this)) {}
    ~IntrusiveSingleLinkedList() { VerifyOrDie(!IsInList()); }

    // IMPLEMENTATION DETAILS:
    //   Since `mNext == this` is used as a marker for "is in a list",
    //   the assignment of these interfaces is overloaded even for the move operator.
    IntrusiveSingleLinkedList(IntrusiveSingleLinkedList && other) :
        mNext((other.mNext == &other) ? static_cast<SELF *>(this) : other.mNext)
    {
        other.SetNotInList();
    }
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
    IntrusiveSingleLinkedList & operator=(IntrusiveSingleLinkedList && other)
    {
        mNext = (other.mNext == &other) ? static_cast<SELF *>(this) : other.mNext;
        other.SetNotInList();
        return *this;
    }

    /// Determines if this object is part of a linked list already or not.
    [[nodiscard]] bool IsInList() const { return (mNext != this); }

    /// Marks this object as not being part of a linked list
    void SetNotInList() { mNext = static_cast<SELF *>(this); }

    /// Returns a "next" pointer when the ServerClusterInterface is assumed to be
    /// part of a SINGLE linked list.
    [[nodiscard]] SELF * GetNextListItem() const
    {
        VerifyOrDie(mNext != this);
        return mNext;
    }

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
    ServerClusterInterface()          = default;
    virtual ~ServerClusterInterface() = default;

    ServerClusterInterface(const ServerClusterInterface & other)             = default;
    ServerClusterInterface(ServerClusterInterface && other)                  = default;
    ServerClusterInterface & operator=(const ServerClusterInterface & other) = default;
    ServerClusterInterface & operator=(ServerClusterInterface && other)      = default;

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
    [[nodiscard]] virtual DataVersion GetDataVersion() const = 0;

    /// Cluster flags can be overridden, however most clusters likely have a default of "nothing special".
    ///
    /// Default implementation returns a 0/empty quality list.
    [[nodiscard]] virtual BitFlags<DataModel::ClusterQualityFlags> GetClusterFlags() const = 0;

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
    virtual DataModel::ActionReturnStatus WriteAttribute(const DataModel::WriteAttributeRequest & request,
                                                         AttributeValueDecoder & decoder) = 0;

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
    virtual CHIP_ERROR Attributes(const ConcreteClusterPath & path,
                                  DataModel::ListBuilder<DataModel::AttributeEntry> & builder) = 0;

    ///////////////////////////////////// Command Support /////////////////////////////////////////////////////////

    virtual std::optional<DataModel::ActionReturnStatus>
    InvokeCommand(const DataModel::InvokeRequest & request, chip::TLV::TLVReader & input_arguments, CommandHandler * handler) = 0;

    virtual CHIP_ERROR AcceptedCommands(const ConcreteClusterPath & path,
                                        DataModel::ListBuilder<DataModel::AcceptedCommandEntry> & builder) = 0;

    virtual CHIP_ERROR GeneratedCommands(const ConcreteClusterPath & path, DataModel::ListBuilder<CommandId> & builder) = 0;
};

} // namespace app
} // namespace chip
