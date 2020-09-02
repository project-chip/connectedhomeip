/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
 *    Copyright (c) 2013-2017 Nest Labs, Inc.
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

/**
 *    @file
 *      Extern declarations for various CHIP global objects.
 *
 */

#ifndef CHIP_GLOBALS_H_
#define CHIP_GLOBALS_H_

#include <core/CHIPConfig.h>
#include <core/CHIPCore.h>
#include <message/CHIPExchangeMgr.h>
#include <message/CHIPFabricState.h>
#include <message/CHIPMessageLayer.h>

namespace chip {

extern ChipFabricState FabricState;
extern ChipMessageLayer MessageLayer;
extern ChipExchangeManager ExchangeMgr;

} // namespace chip

#endif /* CHIP_GLOBALS_H_ */
