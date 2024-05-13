/*
 * Copyright (c) 2020-2022, Arm Limited. All rights reserved.
 * Copyright (c) 2022 Cypress Semiconductor Corporation (an Infineon
 * company) or an affiliate of Cypress Semiconductor Corporation. All rights
 * reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#include "tfm_hal_isolation.h"
#include "Driver_Common.h"
#include "array.h"
#include "cmsis.h"
#include "load/asset_defs.h"
#include "load/partition_defs.h"
#include "load/spm_load_api.h"
#include "mmio_defs.h"
#include "mpu_armv8m_drv.h"
#include "region.h"
#include "target_cfg.h"
#include "tfm_hal_defs.h"
#include "tfm_peripherals_def.h"
#include <arm_cmse.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

/* It can be retrieved from the MPU_TYPE register. */
#define MPU_REGION_NUM 16
#define PROT_BOUNDARY_VAL ((1U << HANDLE_ATTR_PRIV_POS) & HANDLE_ATTR_PRIV_MASK)

#ifdef CONFIG_TFM_ENABLE_MEMORY_PROTECT
static uint32_t n_configured_regions = 0;
struct mpu_armv8m_dev_t dev_mpu_s    = { MPU_BASE };

REGION_DECLARE(Image$$, ER_VENEER, $$Base);
REGION_DECLARE(Image$$, VENEER_ALIGN, $$Limit);
REGION_DECLARE(Image$$, TFM_UNPRIV_CODE, $$RO$$Base);
REGION_DECLARE(Image$$, TFM_UNPRIV_CODE, $$RO$$Limit);
REGION_DECLARE(Image$$, TFM_APP_CODE_START, $$Base);
REGION_DECLARE(Image$$, TFM_APP_CODE_END, $$Base);
REGION_DECLARE(Image$$, TFM_APP_RW_STACK_START, $$Base);
REGION_DECLARE(Image$$, TFM_APP_RW_STACK_END, $$Base);
#ifdef CONFIG_TFM_PARTITION_META
REGION_DECLARE(Image$$, TFM_SP_META_PTR, $$ZI$$Base);
REGION_DECLARE(Image$$, TFM_SP_META_PTR, $$ZI$$Limit);
#endif /* CONFIG_TFM_PARTITION_META */

const struct mpu_armv8m_region_cfg_t region_cfg[] = {
    /* Veneer region */
    { 0, /* will be updated before using */
      (uint32_t) &REGION_NAME(Image$$, ER_VENEER, $$Base), (uint32_t) &REGION_NAME(Image$$, VENEER_ALIGN, $$Limit),
      MPU_ARMV8M_MAIR_ATTR_CODE_IDX, MPU_ARMV8M_XN_EXEC_OK, MPU_ARMV8M_AP_RO_PRIV_UNPRIV, MPU_ARMV8M_SH_NONE,
#ifdef TFM_PXN_ENABLE
      MPU_ARMV8M_PRIV_EXEC_OK
#endif
    },
    /* TFM Core unprivileged code region */
    { 0, /* will be updated before using */
      (uint32_t) &REGION_NAME(Image$$, TFM_UNPRIV_CODE, $$RO$$Base), (uint32_t) &REGION_NAME(Image$$, TFM_UNPRIV_CODE, $$RO$$Limit),
      MPU_ARMV8M_MAIR_ATTR_CODE_IDX, MPU_ARMV8M_XN_EXEC_OK, MPU_ARMV8M_AP_RO_PRIV_UNPRIV, MPU_ARMV8M_SH_NONE,
#ifdef TFM_PXN_ENABLE
      MPU_ARMV8M_PRIV_EXEC_OK
#endif
    },
    /* RO region */
    { 0, /* will be updated before using */
      (uint32_t) &REGION_NAME(Image$$, TFM_APP_CODE_START, $$Base), (uint32_t) &REGION_NAME(Image$$, TFM_APP_CODE_END, $$Base),
      MPU_ARMV8M_MAIR_ATTR_CODE_IDX, MPU_ARMV8M_XN_EXEC_OK, MPU_ARMV8M_AP_RO_PRIV_UNPRIV, MPU_ARMV8M_SH_NONE,
#ifdef TFM_PXN_ENABLE
#if TFM_LVL == 1
      MPU_ARMV8M_PRIV_EXEC_OK
#else
        MPU_ARMV8M_PRIV_EXEC_NEVER
#endif
#endif
    },
    /* RW, ZI and stack as one region */
    { 0, /* will be updated before using */
      (uint32_t) &REGION_NAME(Image$$, TFM_APP_RW_STACK_START, $$Base),
      (uint32_t) &REGION_NAME(Image$$, TFM_APP_RW_STACK_END, $$Base), MPU_ARMV8M_MAIR_ATTR_DATA_IDX, MPU_ARMV8M_XN_EXEC_NEVER,
      MPU_ARMV8M_AP_RW_PRIV_UNPRIV, MPU_ARMV8M_SH_NONE,
#ifdef TFM_PXN_ENABLE
      MPU_ARMV8M_PRIV_EXEC_NEVER
#endif
    },
#ifdef CONFIG_TFM_PARTITION_META
    /* TFM partition metadata pointer region */
    { 0, /* will be updated before using */
      (uint32_t) &REGION_NAME(Image$$, TFM_SP_META_PTR, $$ZI$$Base), (uint32_t) &REGION_NAME(Image$$, TFM_SP_META_PTR, $$ZI$$Limit),
      MPU_ARMV8M_MAIR_ATTR_DATA_IDX, MPU_ARMV8M_XN_EXEC_NEVER, MPU_ARMV8M_AP_RW_PRIV_UNPRIV, MPU_ARMV8M_SH_NONE,
#ifdef TFM_PXN_ENABLE
      MPU_ARMV8M_PRIV_EXEC_NEVER
#endif
    }
#endif
};
#endif /* CONFIG_TFM_ENABLE_MEMORY_PROTECT */

enum tfm_hal_status_t tfm_hal_set_up_static_boundaries(uintptr_t * p_spm_boundary)
{
#ifdef CONFIG_TFM_ENABLE_MEMORY_PROTECT
    struct mpu_armv8m_region_cfg_t localcfg;
#endif
    /* Set up isolation boundaries between SPE and NSPE */
    sau_and_idau_cfg();
    if (mpc_init_cfg() != TFM_PLAT_ERR_SUCCESS)
    {
        return TFM_HAL_ERROR_GENERIC;
    }
    ppc_init_cfg();

    /* Set up static isolation boundaries inside SPE */
#ifdef CONFIG_TFM_ENABLE_MEMORY_PROTECT
    int32_t i;

    mpu_armv8m_clean(&dev_mpu_s);

    if (ARRAY_SIZE(region_cfg) > MPU_REGION_NUM)
    {
        return TFM_HAL_ERROR_GENERIC;
    }
    for (i = 0; i < ARRAY_SIZE(region_cfg); i++)
    {
        memcpy(&localcfg, &region_cfg[i], sizeof(localcfg));
        localcfg.region_nr = i;
        if (mpu_armv8m_region_enable(&dev_mpu_s, (struct mpu_armv8m_region_cfg_t *) &localcfg) != MPU_ARMV8M_OK)
        {
            return TFM_HAL_ERROR_GENERIC;
        }
    }
    n_configured_regions = i;

    mpu_armv8m_enable(&dev_mpu_s, PRIVILEGED_DEFAULT_ENABLE, HARDFAULT_NMI_ENABLE);
#endif /* CONFIG_TFM_ENABLE_MEMORY_PROTECT */

    *p_spm_boundary = (uintptr_t) PROT_BOUNDARY_VAL;

    return TFM_HAL_SUCCESS;
}

/*
 * Implementation of tfm_hal_bind_boundary() on AN552:
 *
 * The API encodes some attributes into a handle and returns it to SPM.
 * The attributes include isolation boundaries, privilege, and MMIO information.
 * When scheduler switches running partitions, SPM compares the handle between
 * partitions to know if boundary update is necessary. If update is required,
 * SPM passes the handle to platform to do platform settings and update
 * isolation boundaries.
 */
enum tfm_hal_status_t tfm_hal_bind_boundary(const struct partition_load_info_t * p_ldinf, uintptr_t * p_boundary)
{
    uint32_t i, j;
    bool privileged;
    bool ns_agent;
    uint32_t partition_attrs = 0;
    const struct asset_desc_t * p_asset;
    struct platform_data_t * plat_data_ptr;
#if TFM_LVL == 2
    struct mpu_armv8m_region_cfg_t localcfg;
#endif
    if (!p_ldinf || !p_boundary)
    {
        return TFM_HAL_ERROR_GENERIC;
    }

#if TFM_LVL == 1
    privileged = true;
#else
    privileged = IS_PARTITION_PSA_ROT(p_ldinf);
#endif

    ns_agent = IS_PARTITION_NS_AGENT(p_ldinf);
    p_asset  = LOAD_INFO_ASSET(p_ldinf);

    /*
     * Validate if the named MMIO of partition is allowed by the platform.
     * Otherwise, skip validation.
     *
     * NOTE: Need to add validation of numbered MMIO if platform requires.
     */
    for (i = 0; i < p_ldinf->nassets; i++)
    {
        if (!(p_asset[i].attr & ASSET_ATTR_NAMED_MMIO))
        {
            continue;
        }
        for (j = 0; j < ARRAY_SIZE(partition_named_mmio_list); j++)
        {
            if (p_asset[i].dev.dev_ref == partition_named_mmio_list[j])
            {
                break;
            }
        }

        if (j == ARRAY_SIZE(partition_named_mmio_list))
        {
            /* The MMIO asset is not in the allowed list of platform. */
            return TFM_HAL_ERROR_GENERIC;
        }
        /* Assume PPC & MPC settings are required even under level 1 */
        plat_data_ptr = REFERENCE_TO_PTR(p_asset[i].dev.dev_ref, struct platform_data_t *);

        if (plat_data_ptr->periph_ppc_bank != PPC_SP_DO_NOT_CONFIGURE)
        {
            ppc_configure_to_secure(plat_data_ptr->periph_ppc_bank, plat_data_ptr->periph_ppc_mask);
            if (privileged)
            {
                ppc_clr_secure_unpriv(plat_data_ptr->periph_ppc_bank, plat_data_ptr->periph_ppc_mask);
            }
            else
            {
                ppc_en_secure_unpriv(plat_data_ptr->periph_ppc_bank, plat_data_ptr->periph_ppc_mask);
            }
        }
#if TFM_LVL == 2
        /*
         * Static boundaries are set. Set up MPU region for MMIO.
         * Setup regions for unprivileged assets only.
         */
        if (!privileged)
        {
            localcfg.region_base    = plat_data_ptr->periph_start;
            localcfg.region_limit   = plat_data_ptr->periph_limit;
            localcfg.region_attridx = MPU_ARMV8M_MAIR_ATTR_DEVICE_IDX;
            localcfg.attr_exec      = MPU_ARMV8M_XN_EXEC_NEVER;
            localcfg.attr_access    = MPU_ARMV8M_AP_RW_PRIV_UNPRIV;
            localcfg.attr_sh        = MPU_ARMV8M_SH_NONE;
#ifdef TFM_PXN_ENABLE
            localcfg.attr_pxn = MPU_ARMV8M_PRIV_EXEC_NEVER;
#endif
            localcfg.region_nr = n_configured_regions++;

            if (mpu_armv8m_region_enable(&dev_mpu_s, &localcfg) != MPU_ARMV8M_OK)
            {
                return TFM_HAL_ERROR_GENERIC;
            }
        }
#endif
    }

    partition_attrs = ((uint32_t) privileged << HANDLE_ATTR_PRIV_POS) & HANDLE_ATTR_PRIV_MASK;
    partition_attrs |= ((uint32_t) ns_agent << HANDLE_ATTR_NS_POS) & HANDLE_ATTR_NS_MASK;
    *p_boundary = (uintptr_t) partition_attrs;

    return TFM_HAL_SUCCESS;
}

enum tfm_hal_status_t tfm_hal_activate_boundary(const struct partition_load_info_t * p_ldinf, uintptr_t boundary)
{
    CONTROL_Type ctrl;
    bool privileged = !!((uint32_t) boundary & HANDLE_ATTR_PRIV_MASK);

    /* Privileged level is required to be set always */
    ctrl.w       = __get_CONTROL();
    ctrl.b.nPRIV = privileged ? 0 : 1;
    __set_CONTROL(ctrl.w);

    return TFM_HAL_SUCCESS;
}

enum tfm_hal_status_t tfm_hal_memory_check(uintptr_t boundary, uintptr_t base, size_t size, uint32_t access_type)
{
    int flags = 0;

    /* If size is zero, this indicates an empty buffer and base is ignored */
    if (size == 0)
    {
        return TFM_HAL_SUCCESS;
    }

    if (!base)
    {
        return TFM_HAL_ERROR_INVALID_INPUT;
    }

    if ((access_type & TFM_HAL_ACCESS_READWRITE) == TFM_HAL_ACCESS_READWRITE)
    {
        flags |= CMSE_MPU_READWRITE;
    }
    else if (access_type & TFM_HAL_ACCESS_READABLE)
    {
        flags |= CMSE_MPU_READ;
    }
    else
    {
        return TFM_HAL_ERROR_INVALID_INPUT;
    }

    if (!((uint32_t) boundary & HANDLE_ATTR_PRIV_MASK))
    {
        flags |= CMSE_MPU_UNPRIV;
    }

    if ((uint32_t) boundary & HANDLE_ATTR_NS_MASK)
    {
        CONTROL_Type ctrl;
        ctrl.w = __TZ_get_CONTROL_NS();
        if (ctrl.b.nPRIV == 1)
        {
            flags |= CMSE_MPU_UNPRIV;
        }
        else
        {
            flags &= ~CMSE_MPU_UNPRIV;
        }
        flags |= CMSE_NONSECURE;
    }

    if (cmse_check_address_range((void *) base, size, flags) != NULL)
    {
        return TFM_HAL_SUCCESS;
    }
    else
    {
        return TFM_HAL_ERROR_MEM_FAULT;
    }
}

bool tfm_hal_boundary_need_switch(uintptr_t boundary_from, uintptr_t boundary_to)
{
    if (boundary_from == boundary_to)
    {
        return false;
    }

    if (((uint32_t) boundary_from & HANDLE_ATTR_PRIV_MASK) && ((uint32_t) boundary_to & HANDLE_ATTR_PRIV_MASK))
    {
        return false;
    }
    return true;
}
