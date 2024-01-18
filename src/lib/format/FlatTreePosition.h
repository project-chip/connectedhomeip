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

#include <lib/format/FlatTree.h>
#include <lib/support/Span.h>

namespace chip {
namespace FlatTree {

/// Represents a position inside a given tree, allowing for descending
/// and ascending.
///
/// A possition in the tree may be undefined if descending to a non-existing leaf,
/// however the position still allows moving back again.
///
/// DESCEND_DEPTH is the maximum remembered depth for going back up.
///
/// General usage:
///
///     Position<DataType, 10> position(tree, tree_size);
///
///     position.Enter(ByName("foo"));
///     position.Enter(ByName("bar"));
///     position.Enter(ByName("baz"));
///
///     position.Get()  /// content of foo::bar::baz if it exists
///
///     position.Exit();
///     position.Exit();
///
///     position.Get()  /// content of foo if it exists
///
///     position.Enter(ById(1234));
///
///     position.Get()  /// content of foo::1234
///
template <typename CONTENT, size_t DESCEND_DEPTH>
class Position
{
public:
    Position(const Node<CONTENT> * tree, size_t treeSize) : mTree(tree), mTreeSize(treeSize) {}

    template <size_t N>
    Position(const std::array<const Node<CONTENT>, N> & tree) : mTree(tree.data()), mTreeSize(N)
    {}

    // Move back to the top
    void ResetToTop()
    {
        mDescendDepth        = 0;
        mUnknownDescendDepth = 0;
    }

    /// Attempt to find a child of the current position that matches
    /// the given matcher
    template <typename MATCHER>
    void Enter(MATCHER matcher);

    /// Move up the tree, undoes an 'Enter' operation.
    void Exit();

    /// Fetch the value where the node is positioned on or nullptr if that
    /// value is not available;
    const CONTENT * Get() const;

    /// Returns the entries visited so far
    ///
    /// WILL RETURN EMPTY if the descend depth has been
    /// exceeded. Callers MUST handle empty return.
    ///
    /// Span valid until one of Enter/Exit functions are called
    /// and as long as the Position is valid (points inside the object).
    chip::Span<const Entry<CONTENT> *> CurrentPath();

    bool HasValidTree() const { return mTree != nullptr; }

    size_t DescendDepth() const { return mDescendDepth + mUnknownDescendDepth; }

private:
    // actual tree that we visit
    const Node<CONTENT> * mTree = nullptr;
    const size_t mTreeSize      = 0;

    // Keeping track of descending into the tree, to be able to move back
    // last element in the array is the "current" item
    const Entry<CONTENT> * mPositions[DESCEND_DEPTH] = { nullptr };
    size_t mDescendDepth                             = 0; // filled amount of mDescendPositions

    // Descend past remembering memory or in not-found entries.
    size_t mUnknownDescendDepth = 0; // depth in invalid positions
};

template <typename CONTENT, size_t DESCEND_DEPTH>
const CONTENT * Position<CONTENT, DESCEND_DEPTH>::Get() const
{
    if (mUnknownDescendDepth > 0)
    {
        return nullptr;
    }

    if (mDescendDepth == 0)
    {
        return nullptr;
    }

    return &mPositions[mDescendDepth - 1]->data;
}

template <typename CONTENT, size_t DESCEND_DEPTH>
template <typename MATCHER>
void Position<CONTENT, DESCEND_DEPTH>::Enter(MATCHER matcher)
{
    if (mUnknownDescendDepth > 0)
    {
        // keep descending into the unknown
        mUnknownDescendDepth++;
        return;
    }

    // To be able to descend, we have to be able to remember
    // the current position
    if (mDescendDepth == DESCEND_DEPTH)
    {
        mUnknownDescendDepth = 1;
        return;
    }

    size_t nodeIdx = 0; // assume root node
    if (mDescendDepth > 0)
    {
        nodeIdx = mPositions[mDescendDepth - 1]->node_index;
    }

    const Entry<CONTENT> * child = FindEntry(mTree, mTreeSize, nodeIdx, matcher);

    if (child == nullptr)
    {
        mUnknownDescendDepth = 1;
        return;
    }

    mPositions[mDescendDepth++] = child;
}

template <typename CONTENT, size_t DESCEND_DEPTH>
void Position<CONTENT, DESCEND_DEPTH>::Exit()
{
    if (mUnknownDescendDepth > 0)
    {
        mUnknownDescendDepth--;
        return;
    }

    if (mDescendDepth > 0)
    {
        mDescendDepth--;
    }
}

template <typename CONTENT, size_t DESCEND_DEPTH>
chip::Span<const Entry<CONTENT> *> Position<CONTENT, DESCEND_DEPTH>::CurrentPath()
{
    if (mUnknownDescendDepth > 0)
    {
        return chip::Span<const Entry<CONTENT> *>();
    }

    // const chip::FlatTree::Entry<{anonymous}::NamedTag>* const* to
    // const chip::FlatTree::Entry<{anonymous}::NamedTag>**
    typename chip::Span<const Entry<CONTENT> *>::pointer p = mPositions;

    // return chip::Span<const Entry<CONTENT> *>(mPositions, mDescendDepth);
    return chip::Span<const Entry<CONTENT> *>(p, mDescendDepth);
}

} // namespace FlatTree
} // namespace chip
