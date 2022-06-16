/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
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
 * @file
 *   This file implements the handler for data model messages.
 */

#include <app/util/DataModelHandler.h>

#if __has_include(<zap-generated/endpoint_config.h>)
#define USE_ZAP_CONFIG 1
#include <app/util/attribute-storage.h>
#include <app/util/util.h>
#endif
#include <lib/support/logging/CHIPLogging.h>

using namespace ::chip;

void InitDataModelHandler(chip::Messaging::ExchangeManager * exchangeManager)
{
#ifdef USE_ZAP_CONFIG
    ChipLogProgress(Zcl, "Using ZAP configuration...");
    emberAfEndpointConfigure();
    emberAfInit(exchangeManager);
#endif
}
