/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
 *    All rights reserved.
 *
 *    Licensed under the Apache License, Version 2.0 (the "License");
 *    you may not use this file except in compliance with the License.
 *    You may obtain a copy of the License at
 *
 *        http://www.apache.org/licenses/LICENSE-2.0
 */

#pragma once

#if defined(BL616CL)
#define CHIP_RESET_PIN 38
#elif defined(BL616)
#define CHIP_RESET_PIN 2
#endif
#define CHIP_CONTACT_PIN 20
