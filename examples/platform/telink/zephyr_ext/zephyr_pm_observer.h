/*
 *
 *    Copyright (c) 2026 Project CHIP Authors
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

#ifndef ZEPHYR_PM_OBSERVER_H
#define ZEPHYR_PM_OBSERVER_H

#ifdef __cplusplus
extern "C" {
#else
#include <stdbool.h>
#endif

void pm_observer_init(void);
bool pm_observer_has_suspended(void);
bool pm_observer_deep_sleep_occurred(void);

#ifdef __cplusplus
}
#endif

#endif /* ZEPHYR_PM_OBSERVER_H */
