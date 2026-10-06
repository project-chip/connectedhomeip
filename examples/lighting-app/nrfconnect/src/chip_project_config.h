/*
 * Copyright (c) 2021 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

/**
 *    @file
 *          Example project configuration file for CHIP.
 *
 *          This is a place to put application or project-specific overrides
 *          to the default configuration values for general CHIP features.
 *
 */

#pragma once

/* Reduce some flash space when the CONFIG_CHIP_MEMORY_PROFILING is selected */
#ifdef CONFIG_CHIP_MEMORY_PROFILING
#define CHIP_CONFIG_LOG_MODULE_SecureChannel_PROGRESS 0
#define CHIP_CONFIG_LOG_MODULE_FabricProvisioning_PROGRESS 0
#define CHIP_CONFIG_LOG_MODULE_InteractionModel_PROGRESS 0
#define CHIP_CONFIG_LOG_MODULE_InteractionModel_DETAIL 0
#define CHIP_CONFIG_LOG_MODULE_DataManagement_PROGRESS 0
#endif
