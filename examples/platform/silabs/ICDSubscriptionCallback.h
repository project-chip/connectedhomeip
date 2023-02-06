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
