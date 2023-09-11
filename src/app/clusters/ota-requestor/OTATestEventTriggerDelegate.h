/*
 *
 * SPDX-FileCopyrightText: 2022 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <app/TestEventTriggerDelegate.h>

namespace chip {

class OTATestEventTriggerDelegate : public TestEventTriggerDelegate
{
public:
    static constexpr uint64_t kOtaQueryTrigger         = 0x0100'0000'0000'0100;
    static constexpr uint64_t kOtaQueryFabricIndexMask = 0xff;

    explicit OTATestEventTriggerDelegate(const ByteSpan & enableKey) : mEnableKey(enableKey) {}

    bool DoesEnableKeyMatch(const ByteSpan & enableKey) const override;
    CHIP_ERROR HandleEventTrigger(uint64_t eventTrigger) override;

private:
    ByteSpan mEnableKey;
};

} // namespace chip
