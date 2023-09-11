/*
 *
 * SPDX-FileCopyrightText: 2020-2021 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <memory>

#include <app/CommandSender.h>
#include <lib/support/CHIPMem.h>

namespace chip {
namespace Controller {

class CommandSenderPlatformDeleter
{
public:
    void operator()(app::CommandSender * commandSender) const { chip::Platform::Delete(commandSender); }
};
} // namespace Controller
} // namespace chip
