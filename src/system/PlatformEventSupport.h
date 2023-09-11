/*
 * SPDX-FileCopyrightText: 2021 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <lib/core/CHIPError.h>
#include <system/SystemEvent.h>
#include <system/SystemLayer.h>

namespace chip {
namespace System {

class Layer;
class Object;

class PlatformEventing
{
public:
    static CHIP_ERROR ScheduleLambdaBridge(System::Layer & aLayer, LambdaBridge && bridge);
    static CHIP_ERROR StartTimer(System::Layer & aLayer, System::Clock::Timeout aTimeout);
};

} // namespace System
} // namespace chip
