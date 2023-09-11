/*
 *
 * SPDX-FileCopyrightText: 2021 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 * @file
 *   This file defines the API for the handler for echo messages.
 */

#pragma once

#include <messaging/ExchangeMgr.h>
#include <system/SystemPacketBuffer.h>

/**
 * Register an unsolicited message handler for Echo protocol to be ready to process
 * Echo messages.
 *
 */
CHIP_ERROR InitEchoHandler(chip::Messaging::ExchangeManager * exchangeMgr);
