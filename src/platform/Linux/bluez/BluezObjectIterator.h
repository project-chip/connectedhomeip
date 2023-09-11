/*
 *
 * SPDX-FileCopyrightText: 2021 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <glib.h>

#include <platform/CHIPDeviceConfig.h>
#include <platform/Linux/dbus/bluez/DbusBluez.h>

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

    reference operator*() const { return *BLUEZ_OBJECT(mPosition->data); }
    pointer operator->() const { return BLUEZ_OBJECT(mPosition->data); }
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
