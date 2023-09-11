/*
 *
 * SPDX-FileCopyrightText: 2023 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <app/clusters/laundry-washer-controls-server/laundry-washer-controls-delegate.h>
#include <app/clusters/laundry-washer-controls-server/laundry-washer-controls-server.h>
#include <app/util/af.h>
#include <app/util/config.h>
#include <cstring>

namespace chip {
namespace app {
namespace Clusters {
namespace LaundryWasherControls {

/**
 * The application delegate to statically define the options.
 */

class LaundryWasherControlDelegate : public Delegate
{
    static const CharSpan spinSpeedsNameOptions[];
    static const NumberOfRinsesEnum supportRinsesOptions[];
    static LaundryWasherControlDelegate instance;

public:
    CHIP_ERROR GetSpinSpeedAtIndex(size_t index, MutableCharSpan & spinSpeed);
    CHIP_ERROR GetSupportedRinseAtIndex(size_t index, NumberOfRinsesEnum & supportedRinse);

    LaundryWasherControlDelegate()  = default;
    ~LaundryWasherControlDelegate() = default;

    static inline LaundryWasherControlDelegate & getLaundryWasherControlDelegate() { return instance; }
};

} // namespace LaundryWasherControls
} // namespace Clusters
} // namespace app
} // namespace chip
