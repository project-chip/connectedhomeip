/*
 * Copyright (c) 2019-2022 Arm Limited. All rights reserved.
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

#include "target_cfg.h"
#include "Driver_MPC.h"
#include "Driver_SSE300_PPC.h"
#include "cmsis.h"
#include "device_definition.h"
#include "region.h"
#include "region_defs.h"
#include "syscounter_armv8-m_cntrl_drv.h"
#include "tfm_plat_defs.h"
#include "uart_stdout.h"
#include "utilities.h"

/* Throw out bus error when an access causes security violation */
#define CMSDK_SECRESPCFG_BUS_ERR_MASK (1UL << 0)

/* The section names come from the scatter file */
REGION_DECLARE(Load$$LR$$, LR_NS_PARTITION, $$Base);
REGION_DECLARE(Image$$, ER_VENEER, $$Base);
REGION_DECLARE(Image$$, VENEER_ALIGN, $$Limit);

const struct memory_region_limits memory_regions = {
    .non_secure_code_start = (uint32_t) &REGION_NAME(Load$$LR$$, LR_NS_PARTITION, $$Base) + BL2_HEADER_SIZE,

    .non_secure_partition_base = (uint32_t) &REGION_NAME(Load$$LR$$, LR_NS_PARTITION, $$Base),

    .non_secure_partition_limit = (uint32_t) &REGION_NAME(Load$$LR$$, LR_NS_PARTITION, $$Base) + NS_PARTITION_SIZE - 1,

    .veneer_base  = (uint32_t) &REGION_NAME(Image$$, ER_VENEER, $$Base),
    .veneer_limit = (uint32_t) &REGION_NAME(Image$$, VENEER_ALIGN, $$Limit),
};

/* Configures the RAM region to NS callable in sacfg block's nsccfg register */
#define RAMNSC 0x2
/* Configures the CODE region to NS callable in sacfg block's nsccfg register */
#define CODENSC 0x1

/* Import MPC drivers */
extern ARM_DRIVER_MPC Driver_ISRAM0_MPC;
extern ARM_DRIVER_MPC Driver_ISRAM1_MPC;
extern ARM_DRIVER_MPC Driver_SRAM_MPC;
extern ARM_DRIVER_MPC Driver_QSPI_MPC;

/* Import PPC drivers */
extern DRIVER_PPC_SSE300 Driver_PPC_SSE300_MAIN0;
extern DRIVER_PPC_SSE300 Driver_PPC_SSE300_MAIN_EXP0;
extern DRIVER_PPC_SSE300 Driver_PPC_SSE300_MAIN_EXP1;
extern DRIVER_PPC_SSE300 Driver_PPC_SSE300_MAIN_EXP2;
extern DRIVER_PPC_SSE300 Driver_PPC_SSE300_MAIN_EXP3;
extern DRIVER_PPC_SSE300 Driver_PPC_SSE300_PERIPH0;
extern DRIVER_PPC_SSE300 Driver_PPC_SSE300_PERIPH1;
extern DRIVER_PPC_SSE300 Driver_PPC_SSE300_PERIPH_EXP0;
extern DRIVER_PPC_SSE300 Driver_PPC_SSE300_PERIPH_EXP1;
extern DRIVER_PPC_SSE300 Driver_PPC_SSE300_PERIPH_EXP2;
extern DRIVER_PPC_SSE300 Driver_PPC_SSE300_PERIPH_EXP3;

/* Define Peripherals NS address range for the platform */
#define PERIPHERALS_BASE_NS_START (0x40000000)
#define PERIPHERALS_BASE_NS_END (0x4FFFFFFF)

/* Enable system reset request for CPU 0 */
#define ENABLE_CPU0_SYSTEM_RESET_REQUEST (1U << 8U)

/* To write into AIRCR register, 0x5FA value must be write to the VECTKEY field,
 * otherwise the processor ignores the write.
 */
#define SCB_AIRCR_WRITE_MASK ((0x5FAUL << SCB_AIRCR_VECTKEY_Pos))

/* Debug configuration flags */
#define SPNIDEN_SEL_STATUS (0x01u << 7)
#define SPNIDEN_STATUS (0x01u << 6)
#define SPIDEN_SEL_STATUS (0x01u << 5)
#define SPIDEN_STATUS (0x01u << 4)
#define NIDEN_SEL_STATUS (0x01u << 3)
#define NIDEN_STATUS (0x01u << 2)
#define DBGEN_SEL_STATUS (0x01u << 1)
#define DBGEN_STATUS (0x01u << 0)

#define All_SEL_STATUS (SPNIDEN_SEL_STATUS | SPIDEN_SEL_STATUS | NIDEN_SEL_STATUS | DBGEN_SEL_STATUS)

static DRIVER_PPC_SSE300 * const ppc_bank_drivers[] = {
    &Driver_PPC_SSE300_MAIN0,       &Driver_PPC_SSE300_MAIN_EXP0,   &Driver_PPC_SSE300_MAIN_EXP1,   &Driver_PPC_SSE300_MAIN_EXP2,
    &Driver_PPC_SSE300_MAIN_EXP3,   &Driver_PPC_SSE300_PERIPH0,     &Driver_PPC_SSE300_PERIPH1,     &Driver_PPC_SSE300_PERIPH_EXP0,
    &Driver_PPC_SSE300_PERIPH_EXP1, &Driver_PPC_SSE300_PERIPH_EXP2, &Driver_PPC_SSE300_PERIPH_EXP3,
};

#define PPC_BANK_COUNT (sizeof(ppc_bank_drivers) / sizeof(ppc_bank_drivers[0]))

enum tfm_plat_err_t enable_fault_handlers(void)
{
    /* Explicitly set secure fault priority to the highest */
    NVIC_SetPriority(SecureFault_IRQn, 0);

    /* Enables BUS, MEM, USG and Secure faults */
    SCB->SHCSR |= SCB_SHCSR_USGFAULTENA_Msk | SCB_SHCSR_BUSFAULTENA_Msk | SCB_SHCSR_MEMFAULTENA_Msk | SCB_SHCSR_SECUREFAULTENA_Msk;
    return TFM_PLAT_ERR_SUCCESS;
}

enum tfm_plat_err_t system_reset_cfg(void)
{
    struct sse300_sysctrl_t * sysctrl = (struct sse300_sysctrl_t *) SSE300_SYSCTRL_BASE_S;
    uint32_t reg_value                = SCB->AIRCR;

    /* Enable system reset request for CPU 0, to be triggered via
     * NVIC_SystemReset function.
     */
    sysctrl->reset_mask |= ENABLE_CPU0_SYSTEM_RESET_REQUEST;

    /* Clear SCB_AIRCR_VECTKEY value */
    reg_value &= ~(uint32_t) (SCB_AIRCR_VECTKEY_Msk);

    /* Enable system reset request only to the secure world */
    reg_value |= (uint32_t) (SCB_AIRCR_WRITE_MASK | SCB_AIRCR_SYSRESETREQS_Msk);

    SCB->AIRCR = reg_value;

    return TFM_PLAT_ERR_SUCCESS;
}

/*--------------------- NVIC interrupt NS/S configuration --------------------*/
enum tfm_plat_err_t nvic_interrupt_target_state_cfg(void)
{
    uint8_t i;

    /* Target every interrupt to NS; unimplemented interrupts will be WI */
    for (i = 0; i < (sizeof(NVIC->ITNS) / sizeof(NVIC->ITNS[0])); i++)
    {
        NVIC->ITNS[i] = 0xFFFFFFFF;
    }

    /* Make sure that MPC and PPC are targeted to S state */
    NVIC_ClearTargetState(MPC_IRQn);
    NVIC_ClearTargetState(PPC_IRQn);

#ifdef SECURE_UART1
    /* UART1 is a secure peripheral, so its IRQs have to target S state */
    NVIC_ClearTargetState(UARTRX1_IRQn);
    NVIC_ClearTargetState(UARTTX1_IRQn);
#endif

    return TFM_PLAT_ERR_SUCCESS;
}

/*----------------- NVIC interrupt enabling for S peripherals ----------------*/
enum tfm_plat_err_t nvic_interrupt_enable(void)
{
    int32_t ret = ARM_DRIVER_OK;
    int32_t i   = 0;

    /* MPC interrupt enabling */
    mpc_clear_irq();
    ret = Driver_ISRAM0_MPC.EnableInterrupt();
    if (ret != ARM_DRIVER_OK)
    {
        ERROR_MSG("Failed to Enable MPC interrupt for ISRAM0!");
        return TFM_PLAT_ERR_SYSTEM_ERR;
    }

    ret = Driver_ISRAM1_MPC.EnableInterrupt();
    if (ret != ARM_DRIVER_OK)
    {
        ERROR_MSG("Failed to Enable MPC interrupt for ISRAM1!");
        return TFM_PLAT_ERR_SYSTEM_ERR;
    }

    ret = Driver_SRAM_MPC.EnableInterrupt();
    if (ret != ARM_DRIVER_OK)
    {
        ERROR_MSG("Failed to Enable MPC interrupt for SRAM!");
        return TFM_PLAT_ERR_SYSTEM_ERR;
    }

    ret = Driver_QSPI_MPC.EnableInterrupt();
    if (ret != ARM_DRIVER_OK)
    {
        ERROR_MSG("Failed to Enable MPC interrupt for QSPI!");
        return TFM_PLAT_ERR_SYSTEM_ERR;
    }

    NVIC_ClearPendingIRQ(MPC_IRQn);
    NVIC_EnableIRQ(MPC_IRQn);

    /* PPC interrupt enabling */
    ppc_clear_irq();

    for (i = 0; i < PPC_BANK_COUNT; i++)
    {
        ret = ppc_bank_drivers[i]->EnableInterrupt();
        if (ret != ARM_DRIVER_OK)
        {
            ERROR_MSG("Failed to Enable interrupt on PPC");
            return TFM_PLAT_ERR_SYSTEM_ERR;
        }
    }

    NVIC_ClearPendingIRQ(PPC_IRQn);
    NVIC_EnableIRQ(PPC_IRQn);

#ifdef PSA_FF_TEST_SECURE_UART2
    NVIC_EnableIRQ(FF_TEST_UART_IRQ);
#endif
    return TFM_PLAT_ERR_SUCCESS;
}

enum tfm_plat_err_t init_debug(void)
{
    struct sse300_sysctrl_t * sysctrl = (struct sse300_sysctrl_t *) SSE300_SYSCTRL_BASE_S;

#if defined(DAUTH_NONE)
    /* Set all the debug enable selector bits to 1 */
    sysctrl->secdbgset = All_SEL_STATUS;
    /* Set all the debug enable bits to 0 */
    sysctrl->secdbgclr = DBGEN_STATUS | NIDEN_STATUS | SPIDEN_STATUS | SPNIDEN_STATUS;
#elif defined(DAUTH_NS_ONLY)
    /* Set all the debug enable selector bits to 1 */
    sysctrl->secdbgset = All_SEL_STATUS;
    /* Set the debug enable bits to 1 for NS, and 0 for S mode */
    sysctrl->secdbgset = DBGEN_STATUS | NIDEN_STATUS;
    sysctrl->secdbgclr = SPIDEN_STATUS | SPNIDEN_STATUS;
#elif defined(DAUTH_FULL)
    /* Set all the debug enable selector bits to 1 */
    sysctrl->secdbgset = All_SEL_STATUS;
    /* Set all the debug enable bits to 1 */
    sysctrl->secdbgset = DBGEN_STATUS | NIDEN_STATUS | SPIDEN_STATUS | SPNIDEN_STATUS;
#else

#if !defined(DAUTH_CHIP_DEFAULT)
#error "No debug authentication setting is provided."
#endif

    /* Set all the debug enable selector bits to 0 */
    sysctrl->secdbgclr = All_SEL_STATUS;

    /* No need to set any enable bits because the value depends on
     * input signals.
     */
#endif
    return TFM_PLAT_ERR_SUCCESS;
}

/*------------------- SAU/IDAU configuration functions -----------------------*/
void sau_and_idau_cfg(void)
{
    struct sse300_sacfg_t * sacfg = (struct sse300_sacfg_t *) SSE300_SACFG_BASE_S;

    /* Enables SAU */
    TZ_SAU_Enable();

    /* Configures SAU regions to be non-secure */
    SAU->RNR  = 0;
    SAU->RBAR = (memory_regions.non_secure_partition_base & SAU_RBAR_BADDR_Msk);
    SAU->RLAR = (memory_regions.non_secure_partition_limit & SAU_RLAR_LADDR_Msk) | SAU_RLAR_ENABLE_Msk;

    SAU->RNR  = 1;
    SAU->RBAR = (NS_DATA_START & SAU_RBAR_BADDR_Msk);
    SAU->RLAR = (NS_DATA_LIMIT & SAU_RLAR_LADDR_Msk) | SAU_RLAR_ENABLE_Msk;

    /* Configures veneers region to be non-secure callable */
    SAU->RNR  = 2;
    SAU->RBAR = (memory_regions.veneer_base & SAU_RBAR_BADDR_Msk);
    SAU->RLAR = (memory_regions.veneer_limit & SAU_RLAR_LADDR_Msk) | SAU_RLAR_ENABLE_Msk | SAU_RLAR_NSC_Msk;

    /* Configure the peripherals space */
    SAU->RNR  = 3;
    SAU->RBAR = (PERIPHERALS_BASE_NS_START & SAU_RBAR_BADDR_Msk);
    SAU->RLAR = (PERIPHERALS_BASE_NS_END & SAU_RLAR_LADDR_Msk) | SAU_RLAR_ENABLE_Msk;

    /* Configure the peripherals space */
    SAU->RNR  = 4;
    SAU->RBAR = (MPC_SRAM_RANGE_BASE_NS & SAU_RBAR_BADDR_Msk);
    SAU->RLAR = (MPC_SRAM_RANGE_LIMIT_NS & SAU_RLAR_LADDR_Msk) | SAU_RLAR_ENABLE_Msk;

    /* Allows SAU to define the CODE region as a NSC */
    sacfg->nsccfg |= RAMNSC;
}

/*------------------- Memory configuration functions -------------------------*/
enum tfm_plat_err_t mpc_init_cfg(void)
{
    int32_t ret = ARM_DRIVER_OK;

    /* ISRAM0 is allocated for NS data, so whole range is set to non-secure
     * accesible. */
    ret = Driver_ISRAM0_MPC.Initialize();
    if (ret != ARM_DRIVER_OK)
    {
        ERROR_MSG("Failed to Initialize MPC for ISRAM0!");
        return TFM_PLAT_ERR_SYSTEM_ERR;
    }
    ret = Driver_ISRAM0_MPC.ConfigRegion(MPC_ISRAM0_RANGE_BASE_NS, MPC_ISRAM0_RANGE_LIMIT_NS, ARM_MPC_ATTR_NONSECURE);
    if (ret != ARM_DRIVER_OK)
    {
        ERROR_MSG("Failed to Configure MPC for ISRAM0!");
        return TFM_PLAT_ERR_SYSTEM_ERR;
    }

    /* ISRAM1 is allocated for NS data, so whole range is set to non-secure
     * accesible. */
    ret = Driver_ISRAM1_MPC.Initialize();
    if (ret != ARM_DRIVER_OK)
    {
        ERROR_MSG("Failed to Initialize MPC for ISRAM1!");
        return TFM_PLAT_ERR_SYSTEM_ERR;
    }
    ret = Driver_ISRAM1_MPC.ConfigRegion(MPC_ISRAM1_RANGE_BASE_NS, MPC_ISRAM1_RANGE_LIMIT_NS, ARM_MPC_ATTR_NONSECURE);
    if (ret != ARM_DRIVER_OK)
    {
        ERROR_MSG("Failed to Configure MPC for ISRAM1!");
        return TFM_PLAT_ERR_SYSTEM_ERR;
    }

    /* Configuring additional flash partition. */
    ret = Driver_SRAM_MPC.Initialize();
    if (ret != ARM_DRIVER_OK)
    {
        ERROR_MSG("Failed to Initialize MPC for SRAM!");
        return TFM_PLAT_ERR_SYSTEM_ERR;
    }
    ret = Driver_SRAM_MPC.ConfigRegion(MPC_SRAM_RANGE_BASE_NS, MPC_SRAM_RANGE_LIMIT_NS, ARM_MPC_ATTR_NONSECURE);
    if (ret != ARM_DRIVER_OK)
    {
        ERROR_MSG("Failed to Configure MPC for SRAM!");
        return TFM_PLAT_ERR_SYSTEM_ERR;
    }

    /* Configuring primary non-secure partition. */
    ret = Driver_QSPI_MPC.Initialize();
    if (ret != ARM_DRIVER_OK)
    {
        ERROR_MSG("Failed to Initialize MPC for QSPI!");
        return TFM_PLAT_ERR_SYSTEM_ERR;
    }
    ret = Driver_QSPI_MPC.ConfigRegion(memory_regions.non_secure_partition_base, memory_regions.non_secure_partition_limit,
                                       ARM_MPC_ATTR_NONSECURE);
    if (ret != ARM_DRIVER_OK)
    {
        ERROR_MSG("Failed to Configure MPC for QSPI!");
        return TFM_PLAT_ERR_SYSTEM_ERR;
    }

    /* SRAM, ISRAM0 and ISRAM1 MPCs left unlocked as they are not reset if NVIC system
     * reset asserted.
     */

    /* Add barriers to assure the MPC configuration is done before continue
     * the execution.
     */
    __DSB();
    __ISB();

    return TFM_PLAT_ERR_SUCCESS;
}

void mpc_revert_non_secure_to_secure_cfg(void)
{
    Driver_ISRAM0_MPC.ConfigRegion(MPC_ISRAM0_RANGE_BASE_S, MPC_ISRAM0_RANGE_LIMIT_S, ARM_MPC_ATTR_SECURE);

    Driver_ISRAM1_MPC.ConfigRegion(MPC_ISRAM1_RANGE_BASE_S, MPC_ISRAM1_RANGE_LIMIT_S, ARM_MPC_ATTR_SECURE);

    Driver_SRAM_MPC.ConfigRegion(MPC_SRAM_RANGE_BASE_S, MPC_SRAM_RANGE_LIMIT_S, ARM_MPC_ATTR_SECURE);

    Driver_QSPI_MPC.ConfigRegion(MPC_QSPI_RANGE_BASE_S, MPC_QSPI_RANGE_LIMIT_S, ARM_MPC_ATTR_SECURE);

    /* Add barriers to assure the MPC configuration is done before continue
     * the execution.
     */
    __DSB();
    __ISB();
}

void mpc_clear_irq(void)
{
    Driver_ISRAM0_MPC.ClearInterrupt();
    Driver_ISRAM1_MPC.ClearInterrupt();
    Driver_SRAM_MPC.ClearInterrupt();
    Driver_QSPI_MPC.ClearInterrupt();
}

/*------------------- PPC configuration functions -------------------------*/
enum tfm_plat_err_t ppc_init_cfg(void)
{
    struct sse300_sacfg_t * sacfg = (struct sse300_sacfg_t *) SSE300_SACFG_BASE_S;
    int32_t err                   = ARM_DRIVER_OK;

    /* Grant non-secure access to peripherals on MAIN EXP0 */
    err |= Driver_PPC_SSE300_MAIN_EXP0.Initialize();
    err |= Driver_PPC_SSE300_MAIN_EXP0.ConfigSecurity(GPIO0_MAIN_PPCEXP0_POS_MASK, PPC_SSE300_NONSECURE_CONFIG);
    err |= Driver_PPC_SSE300_MAIN_EXP0.ConfigSecurity(GPIO1_MAIN_PPCEXP0_POS_MASK, PPC_SSE300_NONSECURE_CONFIG);
    err |= Driver_PPC_SSE300_MAIN_EXP0.ConfigSecurity(GPIO2_MAIN_PPCEXP0_POS_MASK, PPC_SSE300_NONSECURE_CONFIG);
    err |= Driver_PPC_SSE300_MAIN_EXP0.ConfigSecurity(GPIO3_MAIN_PPCEXP0_POS_MASK, PPC_SSE300_NONSECURE_CONFIG);
    err |= Driver_PPC_SSE300_MAIN_EXP0.ConfigSecurity(USB_AND_ETHERNET_MAIN_PPCEXP0_POS_MASK, PPC_SSE300_NONSECURE_CONFIG);

    /* Grant non-secure access to peripherals on MAIN EXP1 */
    err |= Driver_PPC_SSE300_MAIN_EXP1.Initialize();
    err |= Driver_PPC_SSE300_MAIN_EXP1.ConfigSecurity(AHB_USER1_MAIN_PPCEXP1_POS_MASK, PPC_SSE300_NONSECURE_CONFIG);
    err |= Driver_PPC_SSE300_MAIN_EXP1.ConfigSecurity(AHB_USER2_MAIN_PPCEXP1_POS_MASK, PPC_SSE300_NONSECURE_CONFIG);
    err |= Driver_PPC_SSE300_MAIN_EXP1.ConfigSecurity(AHB_USER3_MAIN_PPCEXP1_POS_MASK, PPC_SSE300_NONSECURE_CONFIG);

    /* Grant non-secure access to peripherals on PERIPH0 */
    err |= Driver_PPC_SSE300_PERIPH0.Initialize();
    err |= Driver_PPC_SSE300_PERIPH0.ConfigSecurity(SYSTEM_TIMER0_PERIPH_PPC0_POS_MASK, PPC_SSE300_NONSECURE_CONFIG);
    err |= Driver_PPC_SSE300_PERIPH0.ConfigSecurity(SYSTEM_TIMER1_PERIPH_PPC0_POS_MASK, PPC_SSE300_NONSECURE_CONFIG);
    err |= Driver_PPC_SSE300_PERIPH0.ConfigSecurity(SYSTEM_TIMER2_PERIPH_PPC0_POS_MASK, PPC_SSE300_NONSECURE_CONFIG);
    err |= Driver_PPC_SSE300_PERIPH0.ConfigSecurity(SYSTEM_TIMER3_PERIPH_PPC0_POS_MASK, PPC_SSE300_NONSECURE_CONFIG);
    err |= Driver_PPC_SSE300_PERIPH0.ConfigSecurity(WATCHDOG_PERIPH_PPC0_POS_MASK, PPC_SSE300_NONSECURE_CONFIG);

    /* Grant non-secure access to peripherals on PERIPH1 */
    err |= Driver_PPC_SSE300_PERIPH1.Initialize();
    err |= Driver_PPC_SSE300_PERIPH1.ConfigSecurity(SLOWCLK_TIMER_PERIPH_PPC1_POS_MASK, PPC_SSE300_NONSECURE_CONFIG);

    /* Grant non-secure access to peripherals on PERIPH EXP2 */
    err |= Driver_PPC_SSE300_PERIPH_EXP2.Initialize();

    err |= Driver_PPC_SSE300_PERIPH_EXP2.ConfigSecurity(FPGA_I2S_PERIPH_PPCEXP2_POS_MASK, PPC_SSE300_NONSECURE_CONFIG);
    err |= Driver_PPC_SSE300_PERIPH_EXP2.ConfigSecurity(FPGA_IO_PERIPH_PPCEXP2_POS_MASK, PPC_SSE300_NONSECURE_CONFIG);
    err |= Driver_PPC_SSE300_PERIPH_EXP2.ConfigSecurity(UART0_PERIPH_PPCEXP2_POS_MASK, PPC_SSE300_NONSECURE_CONFIG);
    err |= Driver_PPC_SSE300_PERIPH_EXP2.ConfigSecurity(UART1_PERIPH_PPCEXP2_POS_MASK, PPC_SSE300_NONSECURE_CONFIG);
    err |= Driver_PPC_SSE300_PERIPH_EXP2.ConfigSecurity(UART2_PERIPH_PPCEXP2_POS_MASK, PPC_SSE300_NONSECURE_CONFIG);
    err |= Driver_PPC_SSE300_PERIPH_EXP2.ConfigSecurity(UART3_PERIPH_PPCEXP2_POS_MASK, PPC_SSE300_NONSECURE_CONFIG);
    err |= Driver_PPC_SSE300_PERIPH_EXP2.ConfigSecurity(UART4_PERIPH_PPCEXP2_POS_MASK, PPC_SSE300_NONSECURE_CONFIG);
    err |= Driver_PPC_SSE300_PERIPH_EXP2.ConfigSecurity(UART5_PERIPH_PPCEXP2_POS_MASK, PPC_SSE300_NONSECURE_CONFIG);
    err |= Driver_PPC_SSE300_PERIPH_EXP2.ConfigSecurity(CLCD_PERIPH_PPCEXP2_POS_MASK, PPC_SSE300_NONSECURE_CONFIG);

    /* Grant un-privileged access for UART0 in NS domain */
    err |= Driver_PPC_SSE300_PERIPH_EXP2.ConfigPrivilege(UART0_PERIPH_PPCEXP2_POS_MASK, PPC_SSE300_NONSECURE_CONFIG,
                                                         PPC_SSE300_PRIV_AND_NONPRIV_CONFIG);

    /* Initialize not used PPC drivers */
    err |= Driver_PPC_SSE300_MAIN0.Initialize();
    err |= Driver_PPC_SSE300_MAIN_EXP2.Initialize();
    err |= Driver_PPC_SSE300_MAIN_EXP3.Initialize();
    err |= Driver_PPC_SSE300_PERIPH_EXP0.Initialize();
    err |= Driver_PPC_SSE300_PERIPH_EXP1.Initialize();
    err |= Driver_PPC_SSE300_PERIPH_EXP3.Initialize();

    /*
     * Configure the response to a security violation as a
     * bus error instead of RAZ/WI
     */
    sacfg->secrespcfg |= CMSDK_SECRESPCFG_BUS_ERR_MASK;

    if (err != ARM_DRIVER_OK)
    {
        return TFM_PLAT_ERR_SYSTEM_ERR;
    }

    return TFM_PLAT_ERR_SUCCESS;
}

void ppc_configure_to_secure(enum ppc_bank_e bank, uint32_t pos)
{
    DRIVER_PPC_SSE300 * ppc_driver;

    if (bank >= PPC_BANK_COUNT)
    {
        return;
    }

    ppc_driver = ppc_bank_drivers[bank];
    if (ppc_driver)
    {
        ppc_driver->ConfigSecurity(pos, PPC_SSE300_SECURE_CONFIG);
    }
}

void ppc_configure_to_non_secure(enum ppc_bank_e bank, uint32_t pos)
{
    DRIVER_PPC_SSE300 * ppc_driver;

    if (bank >= PPC_BANK_COUNT)
    {
        return;
    }

    ppc_driver = ppc_bank_drivers[bank];
    if (ppc_driver)
    {
        ppc_driver->ConfigSecurity(pos, PPC_SSE300_NONSECURE_CONFIG);
    }
}

void ppc_en_secure_unpriv(enum ppc_bank_e bank, uint32_t pos)
{
    DRIVER_PPC_SSE300 * ppc_driver;

    if (bank >= PPC_BANK_COUNT)
    {
        return;
    }

    ppc_driver = ppc_bank_drivers[bank];
    if (ppc_driver)
    {
        ppc_driver->ConfigPrivilege(pos, PPC_SSE300_SECURE_CONFIG, PPC_SSE300_PRIV_AND_NONPRIV_CONFIG);
    }
}

void ppc_clr_secure_unpriv(enum ppc_bank_e bank, uint32_t pos)
{
    DRIVER_PPC_SSE300 * ppc_driver;

    if (bank >= PPC_BANK_COUNT)
    {
        return;
    }

    ppc_driver = ppc_bank_drivers[bank];
    if (ppc_driver)
    {
        ppc_driver->ConfigPrivilege(pos, PPC_SSE300_SECURE_CONFIG, PPC_SSE300_PRIV_CONFIG);
    }
}

void ppc_clear_irq(void)
{
    int32_t i = 0;

    for (i = 0; i < PPC_BANK_COUNT; i++)
    {
        ppc_bank_drivers[i]->ClearInterrupt();
    }
}
