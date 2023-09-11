/*
 *
 * SPDX-FileCopyrightText: 2023 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#include <app/clusters/laundry-washer-controls-server/laundry-washer-controls-server.h>
#include <app/util/config.h>
#include <laundry-washer-controls-delegate-impl.h>

using namespace chip;
using namespace chip::app::Clusters::LaundryWasherControls;

const CharSpan LaundryWasherControlDelegate::spinSpeedsNameOptions[] = {
    CharSpan::fromCharString("Off"),
    CharSpan::fromCharString("Low"),
    CharSpan::fromCharString("Medium"),
    CharSpan::fromCharString("High"),
};

const NumberOfRinsesEnum LaundryWasherControlDelegate::supportRinsesOptions[] = {
    NumberOfRinsesEnum::kNormal,
    NumberOfRinsesEnum::kExtra,
};

LaundryWasherControlDelegate LaundryWasherControlDelegate::instance;

CHIP_ERROR LaundryWasherControlDelegate::GetSpinSpeedAtIndex(size_t index, MutableCharSpan & spinSpeed)
{
    if (index >= ArraySize(spinSpeedsNameOptions))
    {
        return CHIP_ERROR_PROVIDER_LIST_EXHAUSTED;
    }
    return chip::CopyCharSpanToMutableCharSpan(LaundryWasherControlDelegate::spinSpeedsNameOptions[index], spinSpeed);
}

CHIP_ERROR LaundryWasherControlDelegate::GetSupportedRinseAtIndex(size_t index, NumberOfRinsesEnum & supportedRinse)
{
    if (index >= ArraySize(supportRinsesOptions))
    {
        return CHIP_ERROR_PROVIDER_LIST_EXHAUSTED;
    }
    supportedRinse = LaundryWasherControlDelegate::supportRinsesOptions[index];
    return CHIP_NO_ERROR;
}
