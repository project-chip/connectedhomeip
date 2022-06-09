/* Copyright 2019 Cypress Semiconductor Corporation
 *
 * Copyright (c) 2018 Open Source Foundries Limited
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#ifndef MCUBOOT_CONFIG_H
#define MCUBOOT_CONFIG_H

/*
 * Template configuration file for MCUboot.
 *
 * When porting MCUboot to a new target, copy it somewhere that your
 * include path can find it as mcuboot_config/mcuboot_config.h, and
 * make adjustments to suit your platform.
 *
 * For examples, see:
 *
 * boot/zephyr/include/mcuboot_config/mcuboot_config.h
 * boot/mynewt/mcuboot_config/include/mcuboot_config/mcuboot_config.h
 */
/* Default maximum number of flash sectors per image slot; change
 * as desirable. */
#ifndef MCUBOOT_MAX_IMG_SECTORS
#define MCUBOOT_MAX_IMG_SECTORS 128
#endif

/*
 * Signature types
 *
 * You must choose exactly one signature type.
 */

/* Uncomment for RSA signature support */
//#define MCUBOOT_SIGN_RSA

/* Uncomment for ECDSA signatures using curve P-256. */
//#define MCUBOOT_SIGN_EC256
//#define NUM_ECC_BYTES (256 / 8) // P-256 curve size in bytes, rnok: to make compilable

// #define MCUBOOT_SIGN_EC

/*
 * Upgrade mode
 *
 * The default is to support A/B image swapping with rollback.  A
 * simpler code path, which only supports overwriting the
 * existing image with the update image, is also available.
 */
#ifdef MCUBOOT_OVERWRITE_ONLY
/* Uncomment to only erase and overwrite those slot 0 sectors needed
 * to install the new image, rather than the entire image slot. */
/* #define MCUBOOT_OVERWRITE_ONLY_FAST */
#else
/* Using SWAP w Scratch by default.
 * Comment/Uncomment which is needed. */
/* #define MCUBOOT_SWAP_USING_SCRATCH 1 */
/* #define MCUBOOT_SWAP_USING_MOVE 1 */
#define MCUBOOT_SWAP_USING_STATUS 1
#endif

/*
 * Cryptographic settings
 *
 * You must choose between mbedTLS and Tinycrypt as source of
 * cryptographic primitives. Other cryptographic settings are also
 * available.
 */

/* Uncomment to use ARM's mbedTLS cryptographic primitives */
#define MCUBOOT_USE_MBED_TLS
/* Uncomment to use Tinycrypt's. */
/* #define MCUBOOT_USE_TINYCRYPT */

/*
 * Always check the signature of the image in slot 0 before booting,
 * even if no upgrade was performed. This is recommended if the boot
 * time penalty is acceptable.
 */
//#define MCUBOOT_VALIDATE_PRIMARY_SLOT

/*
 * Flash abstraction
 */

/* Uncomment if your flash map API supports flash_area_get_sectors().
 * See the flash APIs for more details. */
// TODO: FWSECURITY-755
#define MCUBOOT_USE_FLASH_AREA_GET_SECTORS

/* Default number of separately updateable images; change in case of
 * multiple images. */
#ifndef MCUBOOT_IMAGE_NUMBER
#define MCUBOOT_IMAGE_NUMBER 1
#endif

/*
 * Logging
 */

/*
 * If logging is enabled the following functions must be defined by the
 * platform:
 *
 *    MCUBOOT_LOG_ERR(...)
 *    MCUBOOT_LOG_WRN(...)
 *    MCUBOOT_LOG_INF(...)
 *    MCUBOOT_LOG_DBG(...)
 *
 * The following global logging level configuration macros must also be
 * defined, each with a unique value. Those will be used to define a global
 * configuration and will allow any source files to override the global
 * configuration:
 *
 *    MCUBOOT_LOG_LEVEL_OFF
 *    MCUBOOT_LOG_LEVEL_ERROR
 *    MCUBOOT_LOG_LEVEL_WARNING
 *    MCUBOOT_LOG_LEVEL_INFO
 *    MCUBOOT_LOG_LEVEL_DEBUG
 *
 * The global logging level must be defined, with one of the previously defined
 * logging levels:
 *
 *    #define MCUBOOT_LOG_LEVEL MCUBOOT_LOG_LEVEL_(OFF|ERROR|WARNING|INFO|DEBUG)
 *
 * MCUBOOT_LOG_LEVEL sets the minimum level that will be logged. The function
 * priority is:
 *
 *    MCUBOOT_LOG_ERR > MCUBOOT_LOG_WRN > MCUBOOT_LOG_INF > MCUBOOT_LOG_DBG
 *
 * NOTE: Each source file is still able to request its own logging level by
 * defining BOOT_LOG_LEVEL before #including `bootutil_log.h`
 */
#define MCUBOOT_HAVE_LOGGING 1

/*
 * Assertions
 */

/* Uncomment if your platform has its own mcuboot_config/mcuboot_assert.h.
 * If so, it must provide an ASSERT macro for use by bootutil. Otherwise,
 * "assert" is used. */
//#define MCUBOOT_HAVE_ASSERT_H

#define MCUBOOT_WATCHDOG_FEED()                                                                                                    \
    do                                                                                                                             \
    {                                                                                                                              \
        /* TODO: to be implemented */                                                                                              \
    } while (0)

#endif /* MCUBOOT_CONFIG_H */
