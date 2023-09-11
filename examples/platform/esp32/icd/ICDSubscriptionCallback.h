/*
 *
 * SPDX-FileCopyrightText: 2023 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#pragma once

#include <app/ReadHandler.h>

/**
 * @brief The goal of the ICDSubscriptionCallback class is to negotiate the max interval subscription to match the idle interval of
 * the IC device. When a subscription is requested, the device will change the requested max interval to match its idle time
 * interval through the OnSubscriptionRequested function.
 */
class ICDSubscriptionCallback : public chip::app::ReadHandler::ApplicationCallback
{
    /**
     * @brief Function called when a subscription is requested.
     *        An ICD will use this function to negotiate the subscription max interval to match its idle time interval
     */
    CHIP_ERROR OnSubscriptionRequested(chip::app::ReadHandler & aReadHandler,
                                       chip::Transport::SecureSession & aSecureSession) override;
};
