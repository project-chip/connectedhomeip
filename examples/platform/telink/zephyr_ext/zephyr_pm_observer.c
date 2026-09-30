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

#include "zephyr_pm_observer.h"
#if CONFIG_PM
#include <zephyr/pm/pm.h>
#endif /* CONFIG_PM */

static volatile bool pm_observer_suspend_detected;
static volatile bool pm_observer_deep_sleep_detected;

#if CONFIG_PM
static void pm_observer_state_entry(enum pm_state state)
{
    switch (state)
    {
    case PM_STATE_SUSPEND_TO_IDLE:
        pm_observer_suspend_detected = true;
        break;
    case PM_STATE_STANDBY:
        pm_observer_deep_sleep_detected = true;
        break;
    default:
        break;
    }
}
#endif /* CONFIG_PM */

void pm_observer_init(void)
{
#if CONFIG_PM
    static struct pm_notifier pm_observer_notifier = { .state_entry = pm_observer_state_entry, .state_exit = NULL };

    pm_notifier_register(&pm_observer_notifier);
#endif /* CONFIG_PM */
}

bool pm_observer_has_suspended(void)
{
    return pm_observer_suspend_detected;
}

bool pm_observer_deep_sleep_occurred(void)
{
    return pm_observer_deep_sleep_detected;
}
