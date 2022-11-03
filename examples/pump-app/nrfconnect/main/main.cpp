/*
 * SPDX-FileCopyrightText: (c) 2020 Project CHIP Authors
 * SPDX-FileCopyrightText: (c) 2019 Google LLC.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "AppTask.h"

#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(app, CONFIG_MATTER_LOG_LEVEL);

using namespace ::chip;

int main()
{
    CHIP_ERROR err = GetAppTask().StartApp();

    LOG_ERR("Exited with code %" CHIP_ERROR_FORMAT, err.Format());
    return err == CHIP_NO_ERROR ? EXIT_SUCCESS : EXIT_FAILURE;
}
