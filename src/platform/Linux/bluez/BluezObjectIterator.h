/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
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

#include <iterator>

#include <glib.h>

#include <platform/CHIPDeviceConfig.h>
#include <platform/Linux/dbus/bluez/DBusBluez.h>

namespace chip {
namespace DeviceLayer {
namespace Internal {

/**
 *  Helper class to iterate over a list of Bluez objects.
 */
class BluezObjectIterator
{
public:
    using iterator_category = std::forward_iterator_tag;
    using difference_type   = std::ptrdiff_t;
    using value_type        = BluezObject;
    using pointer           = BluezObject *;
    using reference         = BluezObject &;

    BluezObjectIterator() = default;
    explicit BluezObjectIterator(GList * position) : mPosition(position) {}

    reference operator*() const { return *reinterpret_cast<BluezObject *>(mPosition->data); }
    pointer operator->() const { return reinterpret_cast<BluezObject *>(mPosition->data); }
    bool operator==(const BluezObjectIterator & other) const { return mPosition == other.mPosition; }
    bool operator!=(const BluezObjectIterator & other) const { return mPosition != other.mPosition; }

    BluezObjectIterator & operator++()
    {
        mPosition = mPosition->next;
        return *this;
    }

    BluezObjectIterator operator++(int)
    {
        const auto currentPosition = mPosition;
        mPosition                  = mPosition->next;
        return BluezObjectIterator(currentPosition);
    }

private:
    GList * mPosition = nullptr;
};

} // namespace Internal
} // namespace DeviceLayer
} // namespace chip
