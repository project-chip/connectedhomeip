/*
 *
 * SPDX-FileCopyrightText: 2022 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <app/TestEventTriggerDelegate.h>

namespace chip {

class DefaultTestEventTriggerDelegate : public TestEventTriggerDelegate
{
public:
    static constexpr uint64_t kQueryTrigger = 1234;

    explicit DefaultTestEventTriggerDelegate(const ByteSpan & enableKey) : mEnableKey(enableKey) {}

    bool DoesEnableKeyMatch(const ByteSpan & enableKey) const override;
    CHIP_ERROR HandleEventTrigger(uint64_t eventTrigger) override;

private:
    ByteSpan mEnableKey;
};

} // namespace chip
