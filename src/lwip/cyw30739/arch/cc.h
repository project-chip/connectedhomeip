/*
 *
 *    Copyright (c) 2020 Nest Labs, Inc.
 *    All rights reserved.
 *
 *    This document is the property of Nest. It is considered
 *    confidential and proprietary information.
 *
 *    This document may not be reproduced or transmitted in any form,
 *    in whole or in part, without the express written permission of
 *    Nest.
 *
 *    Description:
 *      This file defines processor-architecture- and toolchain-
 *      specific constants and types required for building
 *      LwIP against FreeRTOS.
 *
 */
#pragma once

#include <assert.h>

#define LWIP_PLATFORM_ASSERT(MSG) assert(MSG)
