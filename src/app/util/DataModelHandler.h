/*
 *
 * SPDX-FileCopyrightText: 2020 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 * @file
 *   This file defines the API for the handler for data model messages.
 */

#pragma once

#include <messaging/ExchangeContext.h>
#include <messaging/ExchangeMgr.h>
#include <system/SystemPacketBuffer.h>
#include <transport/raw/MessageHeader.h>

/**
 * Initialize the data model internal code to be ready to send and receive
 * data model messages.
 *
 */
void InitDataModelHandler();
