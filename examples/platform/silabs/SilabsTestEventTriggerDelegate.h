/*
 *
 * SPDX-FileCopyrightText: 2023 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <app/TestEventTriggerDelegate.h>

namespace chip {

class SilabsTestEventTriggerDelegate : public TestEventTriggerDelegate
{
public:
    explicit SilabsTestEventTriggerDelegate(const ByteSpan & enableKey) : mEnableKey(enableKey) {}

    /**
     * @brief Checks to see if `enableKey` provided matches value chosen by the manufacturer.
     * @param enableKey Buffer of the key to verify.
     * @return True or False.
     */
    bool DoesEnableKeyMatch(const ByteSpan & enableKey) const override;

    /**
     * @brief User handler for handling the test event trigger based on `eventTrigger` provided.
     * @param eventTrigger Event trigger to handle.
     * @return CHIP_NO_ERROR on success or CHIP_ERROR_INVALID_ARGUMENT on failure.
     */
    CHIP_ERROR HandleEventTrigger(uint64_t eventTrigger) override;

private:
    ByteSpan mEnableKey;
};

} // namespace chip

/**
 * @brief User handler for handling the test event trigger
 *
 * @note If TestEventTrigger is enabled, it needs to be implemented in the app
 *
 * @param eventTrigger Event trigger to handle
 *
 * @retval true on success
 * @retval false if error happened
 */
bool emberAfHandleEventTrigger(uint64_t eventTrigger);
