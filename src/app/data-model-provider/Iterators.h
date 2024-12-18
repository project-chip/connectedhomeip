/*
 *    Copyright (c) 2024 Project CHIP Authors
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

#include <optional>

namespace chip {
namespace app {
namespace DataModel {

/// Supports one-way iteration over a specific element list.
///
template <typename T>
class ElementIterator
{
public:
    virtual ~ElementIterator() = default;

    /// Return the next element in the given sequence.
    ///
    /// returns std::nullopt once the end is reached.
    virtual std::optional<T> Next() = 0;
};

/// An iterator that supports seeking to a specific location.
///
/// These iterators MUST return data in a predictable/consistent order
/// so that SeekTo can be used for resuming iterations.
///
/// Useful to directly locate a specific element in an iteration.
template <typename T>
class FastForwardIterator : public ElementIterator<T>
{
public:
    virtual ~FastForwardIterator() = default;

    /// Moves the iterator up to the value `T`. Use as it may be
    /// more efficient than calling Next() repeateadly.
    ///
    /// Returns true if the seek was successful and the element is found.
    /// Returns false otherwise and the iterator will be invalidated (next will return std::nullopt)
    virtual bool SeekTo(const T & value) = 0;
};

/// An iterator that also supports returning extra associated
/// metadata.
template <typename T, typename Meta>
class MetaDataIterator : public FastForwardIterator<T>
{
public:
    virtual ~MetaDataIterator() = default;

    /// Fetch the extra metadata associated with the current position
    /// of the iterator.
    ///
    /// returns std::nullopt if the current iterator position is
    /// before the start (i.e. was just created and Next was never called)
    /// or if the iterator has reached its end.
    virtual std::optional<Meta> GetMetadata() = 0;
};

template <typename T>
class NullFastForwardIterator : public FastForwardIterator<T>
{
public:
    std::optional<T> Next() override { return std::nullopt; }
    bool SeekTo(const T & value) override { return false; }
};

/// Null iterator, generally just used for tests
template <typename T, typename Meta>
class NullMetadataIterator : public NullFastForwardIterator<T>
{
public:
    std::optional<Meta> GetMetadata() override { return std::nullopt; }
};

} // namespace DataModel
} // namespace app
} // namespace chip
