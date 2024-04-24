/*
 *
 *    Copyright (c) 2023 Project CHIP Authors
 *    Copyright 2023-2024 NXP
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

#ifndef _APP_FACTORY_DATA_H_
#define _APP_FACTORY_DATA_H_

#include <lib/core/CHIPError.h>

#ifdef __cplusplus
extern "C" {
#endif

namespace chip {
namespace NXP {
namespace App {
/**
 * Allows to register Matter factory data before initializing the Matter stack
 */
CHIP_ERROR AppFactoryData_PreMatterStackInit(void);

/**
 * Allows to register Matter factory data  after initializing the Matter stack
 */
CHIP_ERROR AppFactoryData_PostMatterStackInit(void);
} // namespace App
} // namespace NXP
} // namespace chip

#ifdef __cplusplus
}
#endif

#endif /* _APP_FACTORY_DATA_H_ */
