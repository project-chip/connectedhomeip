/**
 * SPDX-FileCopyrightText: 2023 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

/**
 * Defines used by endpoint_config.h that may be useful when manually creating
 * endpoint configurations (e.g. for dynamic endpoints).
 */
#define ZAP_TYPE(type) ZCL_##type##_ATTRIBUTE_TYPE
#define ZAP_EMPTY_DEFAULT()                                                                                                        \
    {                                                                                                                              \
        (uint32_t) 0                                                                                                               \
    }
#define ZAP_SIMPLE_DEFAULT(x)                                                                                                      \
    {                                                                                                                              \
        (uint32_t) x                                                                                                               \
    }
#define ZAP_ATTRIBUTE_MASK(mask) ATTRIBUTE_MASK_##mask
#define ZAP_CLUSTER_MASK(mask) CLUSTER_MASK_##mask
