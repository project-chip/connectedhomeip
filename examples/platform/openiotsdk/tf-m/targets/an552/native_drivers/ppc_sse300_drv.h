/*
 * Copyright (c) 2019-2020 Arm Limited
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
 * \file ppc_sse300_drv.h
 * \brief Generic driver for SSE-300 Peripheral Protection Controllers (PPC).
 */

#ifndef __PPC_SSE_300_DRV_H__
#define __PPC_SSE_300_DRV_H__

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* SSE-300 PPC names */
enum ppc_sse300_name_t
{
    PPC_SSE300_MAIN0 = 0,   /*!< MAIN PPC 0 */
    PPC_SSE300_MAIN_EXP0,   /*!< Expansion 0 MAIN PPC */
    PPC_SSE300_MAIN_EXP1,   /*!< Expansion 1 MAIN PPC */
    PPC_SSE300_MAIN_EXP2,   /*!< Expansion 2 MAIN PPC */
    PPC_SSE300_MAIN_EXP3,   /*!< Expansion 3 MAIN PPC */
    PPC_SSE300_PERIPH0,     /*!< PERIPH PPC0 */
    PPC_SSE300_PERIPH1,     /*!< PERIPH PPC1 */
    PPC_SSE300_PERIPH_EXP0, /*!< Expansion 0 PERIPH PPC */
    PPC_SSE300_PERIPH_EXP1, /*!< Expansion 1 PERIPH PPC */
    PPC_SSE300_PERIPH_EXP2, /*!< Expansion 2 PERIPH PPC */
    PPC_SSE300_PERIPH_EXP3, /*!< Expansion 3 PERIPH PPC */
    SSE300_PPC_MAX_NUM
};

/* SSE-300 PPC device configuration structure */
struct ppc_sse300_dev_cfg_t
{
    uint32_t const sacfg_base;  /*!< Secure Privilege Control Block base */
    uint32_t const nsacfg_base; /*!< Non-Secure Privilege Control Block base */
    enum ppc_sse300_name_t ppc_name;
};

/* SSE-300 PPC device data structure */
struct ppc_sse300_dev_data_t
{
    volatile uint32_t * sacfg_ns_ppc;   /*!< Pointer to non-secure register */
    volatile uint32_t * sacfg_sp_ppc;   /*!< Pointer to secure unprivileged
                                              register */
    volatile uint32_t * nsacfg_nsp_ppc; /*!< Pointer to non-secure unprivileged
                                              register */
    uint32_t int_bit_mask;              /*!< Interrupt bit mask */
    bool is_initialized;                /*!< Indicates if the PPC driver
                                             is initialized */
};

/* SSE-300 PPC device structure */
struct ppc_sse300_dev_t
{
    const struct ppc_sse300_dev_cfg_t * const cfg; /*!< PPC configuration */
    struct ppc_sse300_dev_data_t * const data;     /*!< PPC data */
};

/* Security attribute used to configure the peripherals */
enum ppc_sse300_sec_attr_t
{
    PPC_SSE300_SECURE_ACCESS = 0, /*! Secure access */
    PPC_SSE300_NONSECURE_ACCESS,  /*! Non-secure access */
};

/* Privilege attribute used to configure the peripherals */
enum ppc_sse300_priv_attr_t
{
    PPC_SSE300_PRIV_AND_NONPRIV_ACCESS = 0, /*! Privilege and NonPrivilege
                                                access */
    PPC_SSE300_PRIV_ONLY_ACCESS,            /*! Privilege only access */
};

/* ARM PPC error codes */
enum ppc_sse300_error_t
{
    PPC_SSE300_ERR_NONE = 0,      /*!< No error */
    PPC_SSE300_ERR_INVALID_PARAM, /*!< PPC invalid parameter error */
    PPC_SSE300_ERR_NOT_INIT,      /*!< PPC not initialized */
    PPC_SSE300_ERR_NOT_PERMITTED  /*!< PPC Operation not permitted */
};

/**
 * \brief Initialize the PPC device.
 *
 * \param[in] dev       PPC device \ref ppc_sse300_dev_t
 *
 * \return Returns error code as specified in \ref ppc_sse300_error_t
 *
 * \note This function doesn't check if dev is NULL.
 */
enum ppc_sse300_error_t ppc_sse300_init(struct ppc_sse300_dev_t * dev);

/**
 * \brief Configures privilege attribute through the PPC device.
 *
 * \param[in] dev        PPC device \ref ppc_sse300_dev_t
 * \param[in] mask       Peripheral mask for the PPC.
 * \param[in] sec_attr   Secure attribute value.
 * \param[in] priv_attr  Privilege attribute value.
 *
 * \return Returns error code as specified in \ref ppc_sse300_error_t
 *
 * \note This function doesn't check if dev is NULL.
 */
enum ppc_sse300_error_t ppc_sse300_config_privilege(struct ppc_sse300_dev_t * dev, uint32_t mask,
                                                    enum ppc_sse300_sec_attr_t sec_attr, enum ppc_sse300_priv_attr_t priv_attr);

/**
 * \brief Checks if the peripheral is configured as Privilege only or
 *        Privilege and non-Privilege access mode.
 *
 * \param[in] dev     PPC device \ref ppc_sse300_dev_t
 * \param[in] mask    Peripheral mask for the PPC.
 *
 * \return Returns true for Privilege only configuration and false otherwise
 *           - with non-secure caller in the non-secure domain
 *           - with secure caller in the configured security domain
 *         If the driver is not initialized the return value of this function is
 *         true (Privilege only) as it is the default system configuration.
 *
 * \note This function doesn't check if dev is NULL.
 */
bool ppc_sse300_is_periph_priv_only(struct ppc_sse300_dev_t * dev, uint32_t mask);

/* Secure only functions */
#if (defined(__ARM_FEATURE_CMSE) && (__ARM_FEATURE_CMSE == 3U))

/**
 * \brief Configures security attribute through the PPC device.
 *
 * \param[in] dev        PPC device \ref ppc_sse300_dev_t
 * \param[in] mask       Peripheral mask for the PPC.
 * \param[in] sec_attr   Secure attribute value.
 *
 * \return Returns error code as specified in \ref ppc_sse300_error_t
 *
 * \note This function doesn't check if dev is NULL.
 */
enum ppc_sse300_error_t ppc_sse300_config_security(struct ppc_sse300_dev_t * dev, uint32_t mask,
                                                   enum ppc_sse300_sec_attr_t sec_attr);

/**
 * \brief Checks if the peripheral is configured as secure or non-secure.
 *
 * \param[in] dev     PPC device \ref ppc_sse300_dev_t
 * \param[in] mask    Peripheral mask for the PPC.
 *
 * \return Returns true for secure and false for non-secure.
 *         If the driver is not initialized the return value is true (secure) as
 *         it is the default system configuration.
 *
 * \note This function doesn't check if dev is NULL.
 */
bool ppc_sse300_is_periph_secure(struct ppc_sse300_dev_t * dev, uint32_t mask);

/**
 * \brief Enables PPC interrupt.
 *
 * \param[in] dev  PPC device \ref ppc_sse300_dev_t
 *
 * \return Returns error code as specified in \ref ppc_sse300_error_t
 *
 * \note This function doesn't check if dev is NULL.
 */
enum ppc_sse300_error_t ppc_sse300_irq_enable(struct ppc_sse300_dev_t * dev);

/**
 * \brief Disables PPC interrupt.
 *
 * \param[in] dev  PPC device \ref ppc_sse300_dev_t
 *
 * \note This function doesn't check if dev is NULL.
 */
void ppc_sse300_irq_disable(struct ppc_sse300_dev_t * dev);

/**
 * \brief Clears PPC interrupt.
 *
 * \param[in] dev  PPC device \ref ppc_sse300_dev_t
 *
 * \note This function doesn't check if dev is NULL.
 */
void ppc_sse300_clear_irq(struct ppc_sse300_dev_t * dev);

/**
 * \brief Returns the PPC interrupt state.
 *
 * \param[in] dev  PPC device \ref ppc_sse300_dev_t
 *
 * \return Returns true if the interrupt is active and otherwise false.
 *         If the driver is not initialized the return value of this function is
 *         false (not active) as it is the default system configuration.
 *
 * \note This function doesn't check if dev is NULL.
 */
bool ppc_sse300_irq_state(struct ppc_sse300_dev_t * dev);

#endif /* (defined (__ARM_FEATURE_CMSE) && (__ARM_FEATURE_CMSE == 3U)) */

#ifdef __cplusplus
}
#endif
#endif /* __PPC_SSE_300_DRV_H__ */
