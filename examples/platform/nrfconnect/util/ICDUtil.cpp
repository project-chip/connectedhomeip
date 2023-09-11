/*
 *
 * SPDX-FileCopyrightText: 2023 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "ICDUtil.h"

ICDUtil ICDUtil::sICDUtil;

CHIP_ERROR ICDUtil::OnSubscriptionRequested(chip::app::ReadHandler & aReadHandler, chip::Transport::SecureSession & aSecureSession)
{
    uint16_t agreedMaxInterval    = CONFIG_CHIP_MAX_PREFERRED_SUBSCRIPTION_REPORT_INTERVAL;
    uint16_t requestedMinInterval = 0;
    uint16_t requestedMaxInterval = 0;
    aReadHandler.GetReportingIntervals(requestedMinInterval, requestedMaxInterval);

    if (requestedMaxInterval > agreedMaxInterval)
    {
        agreedMaxInterval = requestedMaxInterval;
    }
    else if (agreedMaxInterval > kSubscriptionMaxIntervalPublisherLimit)
    {
        agreedMaxInterval = kSubscriptionMaxIntervalPublisherLimit;
    }

    return aReadHandler.SetMaxReportingInterval(agreedMaxInterval);
}
