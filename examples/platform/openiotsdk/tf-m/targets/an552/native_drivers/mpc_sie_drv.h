/*
 * Copyright (c) 2016-2019 Arm Limited. All rights reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/**
 * \file mpc_sie_drv.h
 * \brief Generic driver for ARM SIE Memory Protection
 *        Controllers (MPC).
 */

#ifndef __MPC_SIE_DRV_H__
#define __MPC_SIE_DRV_H__

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Error code returned by the driver functions */
enum mpc_sie_error_t
{
    MPC_SIE_ERR_NONE,                          /*!< No error */
    MPC_SIE_INVALID_ARG,                       /*!< MPC invalid input arguments */
    MPC_SIE_NOT_INIT,                          /*!< MPC not initialized */
    MPC_SIE_ERR_NOT_IN_RANGE,                  /*!< Address does not belong to a range
                                                *   controlled by the MPC */
    MPC_SIE_ERR_NOT_ALIGNED,                   /*!< Address is not aligned on the block size
                                                *   of this MPC
                                                */
    MPC_SIE_ERR_INVALID_RANGE,                 /*!< The given address range to configure
                                                *   is invalid. This could be because:
                                                *   - The base and limit swapped
                                                *   - The base and limit addresses
                                                *     are in different ranges
                                                */
    MPC_SIE_ERR_RANGE_SEC_ATTR_NON_COMPATIBLE, /*!< The given range cannot be
                                                *   accessed with the wanted
                                                *   security attributes
                                                */
    MPC_SIE_UNSUPPORTED_HARDWARE_VERSION,      /*!< MPC hardware version read from
                                                *   PIDR0 is not supported
                                                */
    MPC_SIE_ERR_GATING_NOT_PRESENT             /*!< MPC gating not present in HW */
};

/* Security attribute used in various place of the API */
enum mpc_sie_sec_attr_t
{
    MPC_SIE_SEC_ATTR_SECURE,    /*!< Secure attribute */
    MPC_SIE_SEC_ATTR_NONSECURE, /*!< Non-secure attribute */
    /*!< Used when getting the configuration of a memory range and some blocks
     *   are secure whereas some other are non secure
     */
    MPC_SIE_SEC_ATTR_MIXED,
};

/* What can happen when trying to do an illegal memory access */
enum mpc_sie_sec_resp_t
{
    MPC_SIE_RESP_RAZ_WI,              /*!< Read As Zero, Write Ignored */
    MPC_SIE_RESP_BUS_ERROR,           /*!< Bus error */
    MPC_SIE_RESP_WAIT_GATING_DISABLED /*!< Wait until gating is disabled */
};

/* Description of a memory range controlled by the MPC */
struct mpc_sie_memory_range_t
{
    const uint32_t base;                /*!< Base address (included in the range) */
    const uint32_t limit;               /*!< Limit address (included in the range) */
    const uint32_t range_offset;        /*!< Offset of current range area to the 0
                                         *   point of the whole area (the sum of the
                                         *   sizes of the previous memory ranges
                                         *   covered by the same MPC)
                                         */
    const enum mpc_sie_sec_attr_t attr; /*!< Optional security attribute
                                         *   needed to be matched when
                                         *   accessing this range.
                                         *   For example, the non-secure
                                         *   alias of a memory region can not
                                         *   be accessed using secure access,
                                         *   and configuring the MPC to
                                         *   secure using that range will not
                                         *   be permitted by the driver.
                                         */
};

/* ARM MPC SIE device configuration structure */
struct mpc_sie_dev_cfg_t
{
    const uint32_t base; /*!< MPC base address */
};

/* ARM MPC SIE device data structure */
struct mpc_sie_dev_data_t
{
    /*!< Array of pointers to memory ranges controlled by the MPC */
    const struct mpc_sie_memory_range_t ** range_list;
    uint8_t nbr_of_ranges; /*!< Number of memory ranges in the list */
    bool is_initialized;   /*!< Indicates if the MPC driver
                            *   is initialized and enabled
                            */
    uint32_t sie_version;  /*!< SIE version */
};

/* ARM MPC SIE device structure */
struct mpc_sie_dev_t
{
    const struct mpc_sie_dev_cfg_t * const cfg; /*!< MPC configuration */
    struct mpc_sie_dev_data_t * const data;     /*!< MPC data */
};

/**
 * \brief Initializes a MPC device.
 *
 * \param[in] dev            MPC device \ref mpc_sie_dev_t
 * \param[in] range_list     List of memory ranges controller by the MPC
 *                           (\ref mpc_sie_memory_range_t). This list can not
 *                           freed after the initializations.
 * \param[in] nbr_of_ranges  Number of memory ranges
 *
 * \return Returns error code as specified in \ref mpc_sie_error_t
 *
 * \note This function doesn't check if dev is NULL.
 */
enum mpc_sie_error_t mpc_sie_init(struct mpc_sie_dev_t * dev, const struct mpc_sie_memory_range_t ** range_list,
                                  uint8_t nbr_of_ranges);

/**
 * \brief Gets MPC block size. All regions must be aligned on this block
 *        size (base address and limit+1 address).
 *
 * \param[in]  dev       MPC device \ref mpc_sie_dev_t
 * \param[out] blk_size  MPC block size
 *
 * \return Returns error code as specified in \ref mpc_sie_error_t
 *
 * \note This function doesn't check if dev is NULL.
 */
enum mpc_sie_error_t mpc_sie_get_block_size(struct mpc_sie_dev_t * dev, uint32_t * blk_size);

/**
 * \brief Configures a memory region (base and limit included).
 *
 * \param[in] dev    MPC device \ref mpc_sie_dev_t
 * \param[in] base   Base address of the region to poll. This bound is
 *                   included. It does not need to be aligned in any way.
 *
 * \param[in] limit  Limit address of the region to poll. This bound is
 *                   included. (limit+1) does not need to be aligned
 *                   in any way.
 * \param[in] attr   Security attribute of the region. If the region has mixed
 *                   secure/non-secure, a special value is returned
 *                   (\ref mpc_sie_sec_attr_t).
 *
 *            In case base and limit+1 addresses are not aligned on
 *            the block size, the enclosing region with base and
 *            limit+1 aligned on block size will be queried.
 *            In case of early termination of the function (error), the
 *            security attribute will be set to MPC_SIE_ATTR_MIXED.
 *
 * \return Returns error code as specified in \ref mpc_sie_error_t
 *
 * \note This function doesn't check if dev is NULL.
 */
enum mpc_sie_error_t mpc_sie_config_region(struct mpc_sie_dev_t * dev, const uint32_t base, const uint32_t limit,
                                           enum mpc_sie_sec_attr_t attr);

/**
 * \brief Gets a memory region configuration(base and limit included).
 *
 * \param[in]  dev    MPC device \ref mpc_sie_dev_t
 * \param[in]  base   Base address of the region to get the configuration.
 * \param[in]  limit  Limit address of the region to get the configuration.
 * \param[out] attr   Security attribute of the region
 *                    \ref mpc_sie_sec_attr_t
 *
 * \return Returns error code as specified in \ref mpc_sie_error_t
 *
 * \note This function doesn't check if dev is NULL.
 */
enum mpc_sie_error_t mpc_sie_get_region_config(struct mpc_sie_dev_t * dev, uint32_t base, uint32_t limit,
                                               enum mpc_sie_sec_attr_t * attr);

/**
 * \brief Gets the MPC control value.
 *
 * \param[in]  dev       MPC device \ref mpc_sie_dev_t
 * \param[out] ctrl_val  Current MPC control value.
 *
 * \return Returns error code as specified in \ref mpc_sie_error_t
 *
 * \note This function doesn't check if dev is NULL.
 */
enum mpc_sie_error_t mpc_sie_get_ctrl(struct mpc_sie_dev_t * dev, uint32_t * ctrl_val);

/**
 * \brief Sets the MPC control value.
 *
 * \param[in] dev       MPC device \ref mpc_sie_dev_t
 * \param[in] mpc_ctrl  New MPC control value
 *
 * \return Returns error code as specified in \ref mpc_sie_error_t
 *
 * \note This function doesn't check if dev is NULL.
 */
enum mpc_sie_error_t mpc_sie_set_ctrl(struct mpc_sie_dev_t * dev, uint32_t mpc_ctrl);

/**
 * \brief Gets the configured secure response.
 *
 * \param[in]  dev      MPC device \ref mpc_sie_dev_t
 * \param[out] sec_rep  Configured secure response (\ref mpc_sie_sec_resp_t).
 *
 * \return Returns error code as specified in \ref mpc_sie_error_t
 *
 * \note This function doesn't check if dev is NULL.
 */
enum mpc_sie_error_t mpc_sie_get_sec_resp(struct mpc_sie_dev_t * dev, enum mpc_sie_sec_resp_t * sec_rep);

/**
 * \brief Sets the response type when SW asks to gate the incoming transfers.
 *
 * \param[in] dev     MPC device \ref mpc_sie_dev_t
 * \param[in] sec_rep Secure response to configure (\ref mpc_sie_sec_resp_t).
 *
 * \note This function doesn't check if dev is NULL.
 */
enum mpc_sie_error_t mpc_sie_set_sec_resp(struct mpc_sie_dev_t * dev, enum mpc_sie_sec_resp_t sec_rep);

/**
 * \brief Enables MPC interrupt.
 *
 * \param[in] dev  MPC device \ref mpc_sie_dev_t
 *
 * \return Returns error code as specified in \ref mpc_sie_error_t
 *
 * \note This function doesn't check if dev is NULL.
 */
enum mpc_sie_error_t mpc_sie_irq_enable(struct mpc_sie_dev_t * dev);

/**
 * \brief Disables MPC interrupt
 *
 * \param[in] dev  MPC device \ref mpc_sie_dev_t
 *
 * \note This function doesn't check if dev is NULL.
 */
void mpc_sie_irq_disable(struct mpc_sie_dev_t * dev);

/**
 * \brief Clears MPC interrupt.
 *
 * \param[in] dev  MPC device \ref mpc_sie_dev_t
 *
 * \note This function doesn't check if dev is NULL.
 */
void mpc_sie_clear_irq(struct mpc_sie_dev_t * dev);

/**
 * \brief Returns the MPC interrupt state.
 *
 * \param[in] dev  MPC device \ref mpc_sie_dev_t
 *
 * \return Returns 1 if the interrupt is active, 0 otherwise.
 *
 * \note This function doesn't check if dev is NULL.
 */
uint32_t mpc_sie_irq_state(struct mpc_sie_dev_t * dev);

/**
 * \brief Locks down the MPC configuration.
 *
 * \param[in] dev  MPC device \ref mpc_sie_dev_t
 *
 * \return Returns error code as specified in \ref mpc_sie_error_t
 *
 * \note This function doesn't check if dev is NULL.
 */
enum mpc_sie_error_t mpc_sie_lock_down(struct mpc_sie_dev_t * dev);

/**
 * \brief Returns if gating is present in hardware.
 *
 * \param[in]  dev             MPC device \ref mpc_sie_dev_t
 * \param[out] gating_present  Returns if gating is present in hardware.
 *
 * \return Returns error code as specified in \ref mpc_sie_error_t
 *
 * \note This function doesn't check if dev is NULL.
 */
enum mpc_sie_error_t mpc_sie_is_gating_present(struct mpc_sie_dev_t * dev, bool * gating_present);

/**
 * \brief Returns the value of Peripheral ID 0 register.
 *
 * \param[in] dev  MPC device \ref mpc_sie_dev_t
 *
 * \return Returns the value of Peripheral ID 0 register.
 *
 * \note This function doesn't check if dev is NULL.
 */
uint32_t get_sie_version(struct mpc_sie_dev_t * dev);

/**
 * \brief Reads bit indicating acknowledge for gating incoming transfers.
 *
 * \param[in] dev  MPC device \ref mpc_sie_dev_t
 *
 * \return True if acknowledge is set.
 *
 * \note This function doesn't check if dev is NULL.
 */
bool mpc_sie_get_gate_ack(struct mpc_sie_dev_t * dev);

/**
 * \brief Sets bit to request for gating incoming transfers.
 *
 * \param[in] dev  MPC device \ref mpc_sie_dev_t
 *
 * \note This function doesn't check if dev is NULL.
 */
void mpc_sie_request_gating(struct mpc_sie_dev_t * dev);

/**
 * \brief Clears bit to request for gating incoming transfers.
 *
 * \param[in] dev  MPC device \ref mpc_sie_dev_t
 *
 * \note This function doesn't check if dev is NULL.
 */
void mpc_sie_release_gating(struct mpc_sie_dev_t * dev);

#ifdef __cplusplus
}
#endif
#endif /* __MPC_SIE_DRV_H__ */
