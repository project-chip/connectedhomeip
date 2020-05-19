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

#ifndef TEST_UTILS_H_
#define TEST_UTILS_H_

#include <core/CHIPError.h>

#if __cplusplus
extern "C" {
#endif

namespace chip {

typedef int (*UnitTestTriggerFunction)(void);

CHIP_ERROR RegisterUnitTests(UnitTestTriggerFunction tests);

int RunRegisteredUnitTests();

} // namespace chip

#if __cplusplus
}
#endif

#endif /* TEST_UTILS_H_ */
