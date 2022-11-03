/*
 * SPDX-FileCopyrightText: (c) 2020 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

// App framework
#include <app/util/af.h>
#include <app/util/attribute-storage.h>
#include <app/util/attribute-table.h>
#include <app/util/util.h>

#include <messaging/ExchangeContext.h>

// the variables used to setup and send responses to cluster messages
extern EmberApsFrame emberAfResponseApsFrame;
extern uint8_t appResponseData[EMBER_AF_RESPONSE_BUFFER_LEN];
extern uint16_t appResponseLength;
extern chip::Messaging::ExchangeContext * emberAfResponseDestination;
