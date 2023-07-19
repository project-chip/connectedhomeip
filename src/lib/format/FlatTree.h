/*
 *
 *    Copyright (c) 2023 Project CHIP Authors
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

#include <stddef.h>

#include <array>
#include <limits>

namespace chip {
namespace FlatTree {

// A flat tree allows for tree data to be stored in a single flat
// array.

/// Invalid indexes in a tree
static constexpr size_t kInvalidNodeIndex = std::numeric_limits<size_t>::max();

/// An entry represents a single element identified by a key and containing a
/// value
///
/// In a tree representation, every entry may potentially have a child node,
/// whose index is located in [node_index].
template <typename CONTENT>
struct Entry
{
    CONTENT data;

    // Node index is a valid index inside a node array if a entry has
    // child elements, it is kInvalidNodeIndex otherwise;
    size_t node_index;
};

template <typename CONTENT>
struct Node
{
    size_t entry_count;             // number of items in [entries]
    const Entry<CONTENT> * entries; // child items of [entry_count] size

    /// Attempt to find the entry with given matcher.
    ///
    /// Returns nullptr if no matches can be found.
    template <typename MATCHER>
    const Entry<CONTENT> * find_entry(MATCHER matcher) const
    {
        for (size_t i = 0; i < entry_count; i++)
        {
            if (matcher(entries[i].data))
            {
                return &entries[i];
            }
        }
        return nullptr;
    }
};

/// Search for a given entry in a sized array
///
/// [data] is the flat tree array
/// [idx] is the index of the node to search. If out of bounds, nullptr is returned
/// [matcher] is the match function.
template <typename CONTENT, typename MATCHER>
inline const Entry<CONTENT> * FindEntry(const Node<CONTENT> * content, size_t content_size, size_t idx, MATCHER matcher)
{
    if (idx >= content_size)
    {
        return nullptr;
    }
    return content[idx].find_entry(matcher);
}

/// Search for a given entry in an array (array will do bounds check)
///
/// [data] is the flat tree array
/// [idx] is the index of the node to search. If out of bounds, nullptr is returned
/// [matcher] is the match function.
template <typename CONTENT, typename MATCHER, size_t N>
inline const Entry<CONTENT> * FindEntry(const std::array<Node<CONTENT>, N> & data, size_t idx, MATCHER matcher)
{
    return FindEntry(data.data(), N, idx, matcher);
}

} // namespace FlatTree
} // namespace chip
