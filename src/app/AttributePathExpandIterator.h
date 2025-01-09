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

#include <app/AttributePathParams.h>
#include <app/ConcreteAttributePath.h>
#include <app/data-model-provider/Provider.h>
#include <lib/core/DataModelTypes.h>
#include <lib/support/LinkedList.h>

namespace chip {
namespace app {

/// Handles attribute path expansions
/// Usage:
///
/// - Start iterating by creating an iteration state
///
///      AttributePathExpandIterator::Position position = AttributePathExpandIterator::Position::StartIterating(path);
///
/// - Use the iteration state in a for loop:
///
///      ConcreteAttributePath path;
///      for (AttributePathExpandIterator iterator(position); iterator->Next(path);) {
///         // use `path` here`
///      }
///
///   OR:
///
///      ConcreteAttributePath path;
///      AttributePathExpandIterator iterator(position);
///
///      while (iterator.Next(path)) {
///         // use `path` here`
///      }
///
/// Usage requirements and assumptions:
///
///    - An ` AttributePathExpandIterator::Position` can only be used by a single AttributePathExpandIterator at a time.
///
///    - `position` is automatically updated by the AttributePathExpandIterator, so
///      calling `Next` on the iterator will update the position cursor variable.
///
class AttributePathExpandIterator
{
public:
    class Position
    {
    public:
        // Position is treated as a direct member access by the AttributePathExpandIterator, however it is opaque (except copying)
        // for external code. We allow friendship here to not have specific get/set for methods (clearer interface and less
        // likelihood of extra code usage).
        friend class AttributePathExpandIterator;

        /// External callers can only ever start iterating on a new path from the beginning
        static Position StartIterating(SingleLinkedListNode<AttributePathParams> * path) { return Position(path); }

        /// Copies are allowed
        Position(const Position &)             = default;
        Position & operator=(const Position &) = default;

        Position() : mAttributePath(nullptr) {}

        /// Reset the iterator to the beginning of current cluster if we are in the middle of expanding a wildcard attribute id for
        /// some cluster.
        ///
        /// When attributes are changed in the middle of expanding a wildcard attribute, we need to reset the iterator, to provide
        /// the client with a consistent state of the cluster.
        void IterateFromTheStartOfTheCurrentCluster()
        {
            VerifyOrReturn(mAttributePath != nullptr && mAttributePath->mValue.HasWildcardAttributeId());
            mOutputPath.mAttributeId = kInvalidAttributeId;
        }

    protected:
        Position(SingleLinkedListNode<AttributePathParams> * path) :
            mAttributePath(path), mOutputPath(kInvalidEndpointId, kInvalidClusterId, kInvalidAttributeId)
        {}

        SingleLinkedListNode<AttributePathParams> * mAttributePath;
        ConcreteAttributePath mOutputPath;
    };

    AttributePathExpandIterator(DataModel::Provider * dataModel, Position & position) :
        mDataModelProvider(dataModel), mPosition(position)
    {}

    // This class may not be copied. A new one should be created when needed and they
    // should not overlap.
    AttributePathExpandIterator(const AttributePathExpandIterator &)             = delete;
    AttributePathExpandIterator & operator=(const AttributePathExpandIterator &) = delete;

    /// Get the next path of the expansion (if one exists).
    ///
    /// On success, true is returned and `path` is filled with the next path in the
    /// expansion.
    /// On iteration completion, false is returned and the content of path IS NOT DEFINED.
    bool Next(ConcreteAttributePath & path);

private:
    DataModel::Provider * mDataModelProvider;
    Position & mPosition;

    /// Move to the next endpoint/cluster/attribute triplet that is valid given
    /// the current mOutputPath and mpAttributePath.
    ///
    /// returns true if such a next value was found.
    bool AdvanceOutputPath();

    /// Get the next attribute ID in mOutputPath(endpoint/cluster) if one is available.
    /// Will start from the beginning if current mOutputPath.mAttributeId is kInvalidAttributeId
    ///
    /// Respects path expansion/values in mpAttributePath
    ///
    /// Handles Global attributes (which are returned at the end)
    std::optional<AttributeId> NextAttributeId();

    /// Get the next cluster ID in mOutputPath(endpoint) if one is available.
    /// Will start from the beginning if current mOutputPath.mClusterId is kInvalidClusterId
    ///
    /// Respects path expansion/values in mpAttributePath
    std::optional<ClusterId> NextClusterId();

    /// Get the next endpoint ID in mOutputPath if one is available.
    /// Will start from the beginning if current mOutputPath.mEndpointId is kInvalidEndpointId
    ///
    /// Respects path expansion/values in mpAttributePath
    std::optional<ClusterId> NextEndpointId();

    /// Checks if the given attributeId is valid for the current mOutputPath(endpoint/cluster)
    ///
    /// Meaning that it is known to the data model OR it is a always-there global attribute.
    bool IsValidAttributeId(AttributeId attributeId);
};

/// PeekAttributePathExpandIterator is an AttributePathExpandIterator wrapper that rolls back the Next()
/// call whenever a new `MarkCompleted()` method is not called (until Next() returns false). This is useful
/// to allow pausing iteration in cases the next path was not ready to be used yet and iteration needs to
/// continue later.
///
/// Example use cases:
///
/// - Iterate over all attributes and process one-by-one, however when the iteration fails, resume at
///   the last failure point:
///
///      PeekAttributePathExpandIterator iterator(....);
///      ConcreteAttributePath path;
///
///      while (iterator.Next(path)) {
///         if (!CanProcess(path)) {
///             // iterator state IS PRESERVED so that Next() will return the SAME path on the next call.
///             return CHIP_ERROR_TRY_AGAIN_LATER;
///         }
///      }
///      // this will make Next() NOT return the previous value.
///      iterator.MarkCompleted();
///
/// -  Grab what the next output path would be WITHOUT advancing a state;
///
///      {
///        PeekAttributePathExpandIterator iterator(...., state);
///        if (iterator.Next(...)) { ... }
///      }
///      // state here is ROLLED BACK (i.e. next calls the same value)
///
///
class PeekAttributePathExpandIterator
{
public:
    PeekAttributePathExpandIterator(DataModel::Provider * dataModel, AttributePathExpandIterator::Position & position) :
        mAttributePathExpandIterator(dataModel, position), mPositionTarget(position), mOldPosition(position)
    {}
    ~PeekAttributePathExpandIterator() { mPositionTarget = mOldPosition; }

    bool Next(ConcreteAttributePath & path)
    {
        mOldPosition = mPositionTarget;
        return mAttributePathExpandIterator.Next(path);
    }

    /// Marks the current iteration completed (so peek does not actually roll back)
    void MarkCompleted() { mOldPosition = mPositionTarget; }

private:
    AttributePathExpandIterator mAttributePathExpandIterator;
    AttributePathExpandIterator::Position & mPositionTarget;
    AttributePathExpandIterator::Position mOldPosition;
};

} // namespace app
} // namespace chip
