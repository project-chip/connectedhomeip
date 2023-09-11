/*
 *
 * SPDX-FileCopyrightText: 2022 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <app/TestEventTriggerDelegate.h>

namespace chip {

class GenericFaultTestEventTriggerDelegate : public TestEventTriggerDelegate
{
public:
    static constexpr uint64_t kGenericFaultQueryTrigger         = 0xFFFF'FFFF'10D0'0001;
    static constexpr uint64_t kGenericFaultQueryFabricIndexMask = 0xff;

    explicit GenericFaultTestEventTriggerDelegate(const ByteSpan & enableKey) : mEnableKey(enableKey) {}

    bool DoesEnableKeyMatch(const ByteSpan & enableKey) const override;
    CHIP_ERROR HandleEventTrigger(uint64_t eventTrigger) override;

private:
    ByteSpan mEnableKey;
};

} // namespace chip
