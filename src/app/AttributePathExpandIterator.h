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
#include <lib/support/LinkedList.h>

namespace chip {
namespace app {

/**
 * AttributePathExpandIterator is used to iterate over a linked list of AttributePathParams-s.
 * The AttributePathExpandIterator is copiable, however, the given cluster info must be valid when calling Next().
 *
 * AttributePathExpandIterator will expand attribute paths with wildcards, and only emit existing paths for
 * AttributePathParams with wildcards. For AttributePathParams with a concrete path (i.e. does not contain wildcards),
 * AttributePathExpandIterator will emit them as-is.
 *
 * The typical use of AttributePathExpandIterator may look like:
 * ConcreteAttributePath path;
 * for (AttributePathExpandIterator iterator(AttributePathParams); iterator.Get(path); iterator.Next()) {...}
 *
 * The iterator does not copy the given AttributePathParams. The given AttributePathParams must remain valid when using the
 * iterator. If the set of endpoints, clusters, or attributes that are supported changes, AttributePathExpandIterator must be
 * reinitialized.
 *
 * A initialized iterator will return the first valid path, no need to call Next() before calling Get() for the first time.
 *
 * Note: Next() and Get() are two separate operations by design since a possible call of this iterator might be:
 * - Get()
 * - Chunk full, return
 * - In a new chunk, Get()
 *
 * TODO: The AttributePathParams may support a group id, the iterator should be able to call group data provider to expand the group
 * id.
 */
class AttributePathExpandIterator
{
public:
    AttributePathExpandIterator(DataModel::Provider * provider, SingleLinkedListNode<AttributePathParams> * attributePath);

    /** A struct that MUST be aquired for Next() to be called
     *
     * NOTE: Next() can be only called during single-loop and this session
     *       MUST NOT be stored outside of a local stack frame
     *
     * Using a search session initializes caches for searching endpoints/clusters/attributes
     */
    struct SearchSession
    {
        std::unique_ptr<DataModel::MetaDataIterator<EndpointId, DataModel::EndpointInfo>> endpoints;
    };

    /**
     * Prepare a search session so that `Next` can be called.
     *
     * NOTE: Returned value MUST NOT be stored outside a single Next() loop.
     */
    SearchSession PrepareSearch();

    /**
     * Proceed the iterator to the next attribute path in the given cluster info.
     *
     * Returns false if AttributePathExpandIteratorDataModeDataModel has exhausted all paths in the given AttributePathParams list.
     */
    bool Next(SearchSession & session);

    /**
     * Fills the aPath with the path the iterator currently points to.
     * Returns false if the iterator is not pointing to a valid path (i.e. it has exhausted the cluster info).
     */
    bool Get(SearchSession & session, ConcreteAttributePath & aPath)
    {
        EnsureFirstNextCalled(session);
        aPath = mOutputPath;
        return (mpAttributePath != nullptr);
    }

    /**
     * Reset the iterator to the beginning of current cluster if we are in the middle of expanding a wildcard attribute id for some
     * cluster.
     *
     * When attributes are changed in the middle of expanding a wildcard attribute, we need to reset the iterator, to provide the
     * client with a consistent state of the cluster.
     */
    void ResetCurrentCluster(SearchSession & session);

    /** Start iterating over the given `paths` */
    inline void ResetTo(SearchSession & session, SingleLinkedListNode<AttributePathParams> * paths)
    {
        EnsureFirstNextCalled(session);
        *this = AttributePathExpandIterator(mDataModelProvider, paths);
    }

private:
    DataModel::Provider * mDataModelProvider;
    SingleLinkedListNode<AttributePathParams> * mpAttributePath;
    ConcreteAttributePath mOutputPath;

    // Iterator is expected to be positioned on the FIRST element right after
    // the constructor. Since Next requires a session, we defer the first Next call
    // to the first public API call
    bool mStartWithNext = true;

    /// this is to be called on EVERY public API to preserve
    /// API invariants (behave as if NEXT is called in the constructor)
    void EnsureFirstNextCalled(SearchSession & session)
    {
        if (!mStartWithNext)
        {
            return;
        }
        mStartWithNext = false;
        (void) Next(session); // NOTE: ignored return code
    }

    /// Move to the next endpoint/cluster/attribute triplet that is valid given
    /// the current mOutputPath and mpAttributePath
    ///
    /// returns true if such a next value was found.
    bool AdvanceOutputPath(SearchSession & session);

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
    std::optional<ClusterId> NextEndpointId(SearchSession & session);

    /// Checks if the given attributeId is valid for the current mOutputPath(endpoint/cluster)
    ///
    /// Meaning that it is known to the data model OR it is a always-there global attribute.
    bool IsValidAttributeId(AttributeId attributeId);
};

} // namespace app
} // namespace chip
