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

/**
 * @brief Defines an abstract interface for providing feedback to the user.
 *
 * For example, the feedback can be provided through a logging mechanism or
 * through LED animation.
 */
class UserInterfaceFeedback
{
public:
    /**
     * @brief Set of possible actions handled by the DisplayOnAction method.
     *
     */
    enum class Action : uint8_t
    {
        kFactoryReset = 0,
        kTriggerEffect,
        kIdentify
    };

    virtual ~UserInterfaceFeedback() = default;

    virtual void Init() = 0;

    /**
     * @brief Provide feedback in app task main loop.
     *
     * For a concrete example, this can be a LED animation command.
     */
    virtual void DisplayInLoop() = 0;

    /**
     * @brief Provide feedback on a specific action.
     *
     */
    virtual void DisplayOnAction(Action action) = 0;

    /**
     * @brief Restore feedback state based on a default config.
     *
     * For example, the restoration can be based on a cluster attribute
     * value, such as OnOff value in case of a LightingManager.
     */
    virtual void RestoreState() = 0;
};

/**
 * @brief Getter for the concrete class that implements the interface.
 *
 * It returns a reference to UserInterfaceFeedback, such that only the
 * public API can be used regardless of the actual concrete class.
 */
extern UserInterfaceFeedback & FeedbackMgr();
