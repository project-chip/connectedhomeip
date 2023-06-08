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
template <typename KEY, typename VALUE>
struct Entry
{
    KEY key;
    VALUE value;

    // Node index is a valid index inside a node array if a entry has
    // child elements, it is kInvalidNodeIndex otherwise;
    size_t node_index;
};

template <typename KEY, typename VALUE>
struct Node
{
    size_t entry_count;          // number of items in [entries]
    Entry<KEY, VALUE> * entries; // child items of [entry_count] size

    /// Attempt to find the entry with the given key. Returns nullptr
    /// if the entry with this key cannot be found.
    const Entry<KEY, VALUE> *find_entry(const KEY & key) const
    {
        for (size_t i = 0; i < entry_count; i++)
        {
            if (entries[i].key == key)
            {
                return &entries[i];
            }
        }
        return nullptr;
    }
};

template <typename KEY, typename VALUE, size_t N>
const Entry<KEY, VALUE> * FindEntry(const std::array<Node<KEY, VALUE>, N> & data, size_t idx, KEY key)
{
    if (idx >= data.size())
    {
        return nullptr;
    }
    return data[idx].find_entry(key);
}

} // namespace FlatTree
} // namespace chip
