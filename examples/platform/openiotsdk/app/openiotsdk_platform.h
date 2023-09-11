/*
 *
 * SPDX-FileCopyrightText: 2022 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 *    @file
 *      This file provides the common Open IoT SDK platform functions.
 *      It can be used in Matter examples implementation.
 */

#ifndef OPENIOTSDK_PLATFORM_H
#define OPENIOTSDK_PLATFORM_H

#include <stdbool.h>
#include <stdlib.h>

// Default stack size for network thread (8kB)
#define NETWORK_THREAD_STACK_SIZE_DEFAULT (8 * 1024)

/**
 * @brief Initialise the Open IoT SDK platform
 * Mbedtls platform setup
 * OS kernel initialization and check
 *
 * @return EXIT_SUCCESS or EXIT_FAILURE
 */
int openiotsdk_platform_init(void);

/**
 * @brief Initialise the CHIP sources
 * Platform memory and CHIP stack initialization
 * Start CHIP event loop task
 *
 * @return EXIT_SUCCESS or EXIT_FAILURE
 */
int openiotsdk_chip_init(void);

/**
 * @brief Run the Open IoT SDK platform
 * Start the OS kernel
 *
 * @return EXIT_SUCCESS or EXIT_FAILURE
 */
int openiotsdk_platform_run(void);

/**
 * @brief Initialise the Open IoT SDK network
 * Run the network task and wait for newtork up
 *
 * @param wait Wait for network up
 * @return EXIT_SUCCESS or EXIT_FAILURE
 */
int openiotsdk_network_init(bool wait);

/**
 * @brief Run the CHIP sources/components
 * Initialize ZCL Data Model and start server
 *
 * @return EXIT_SUCCESS or EXIT_FAILURE
 */
int openiotsdk_chip_run(void);

/**
 * @brief Shutdown the CHIP sources/components
 * Stop chip server
 */
void openiotsdk_chip_shutdown(void);

#endif /* ! OPENIOTSDK_PLATFORM_H */
