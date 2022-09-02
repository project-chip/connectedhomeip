/***************************************************************************//**
 * @file
 * @brief Configuration header of Bootloader Interface
 *******************************************************************************
 * # License
 * <b>Copyright 2021 Silicon Laboratories Inc. www.silabs.com</b>
 *******************************************************************************
 *
 * The licensor of this software is Silicon Laboratories Inc.  Your use of this
 * software is governed by the terms of Silicon Labs Master Software License
 * Agreement (MSLA) available at
 * www.silabs.com/about-us/legal/master-software-license-agreement.  This
 * software is distributed to you in Source Code format and is governed by the
 * sections of the MSLA applicable to Source Code.
 *
 ******************************************************************************/
#ifndef BTL_INTERFACE_CFG_S2C4_H
#define BTL_INTERFACE_CFG_S2C4_H

// <<< Use Configuration Wizard in Context Menu >>>
// <h> Bootloader Interface Trust Zone Security State Configuration

// <q BOOTLOADER_DISABLE_OLD_BOOTLOADER_MITIGATION> Disable multi tiered fallback logic
// <i> The fault handling logic as well as the USART auto-detection logic will be disabled.
// <i> The re-configuration of SMU will be handled by querying the running bootloader
// <i> for the peripheral list. Querying the peripheral list is supported from the bootloader
// <i> version 2.0.0. Check for the BOOTLOADER_CAPABILITY_PERIPHERAL_LIST capability to
// <i> see if the running bootloader supports querying the peripheral list.
#define BOOTLOADER_DISABLE_OLD_BOOTLOADER_MITIGATION 0

// <q BOOTLOADER_DISABLE_NVM3_FAULT_HANDLING> Disable peripheral access fault handling
// <i> The fault handling triggered by an erroneous access of peripherals will be disabled.
// <i>
// <i> This should be disabled iff all the peripherals that are in use by the bootloader
// <i> have been properly configured by the "Manually override security state of peripherals" option.
#define BOOTLOADER_DISABLE_NVM3_FAULT_HANDLING 0

// <e BOOTLOADER_MANUAL_OVERRIDE_SECURITY_STATE> Manually override the security state of peripherals in use by the bootloader
// <i> Default: 0.
// <i> Manually override the security state of peripherals. Choose the peripherals touched
// <i> by the customized code to have a valid secure access state before interacting with the bootloader
// <i> In practice, this means that the chosen peripherals will be available at the secure address,
// <i> which is accessible by the bootloader. Once the application is entered back the secure access state
// <i> is changed back to the original state.
// <i>
// <i> The USART auto-detection logic that detects which, if any, USART is in use by the bootloader
// <i> will be disabled. Make sure to choose the correct USART used by the bootloader.
#define BOOTLOADER_MANUAL_OVERRIDE_SECURITY_STATE 0
// <q> EMU
// <i> Update secure access state of EMU before calling into bootloader
#define BOOTLOADER_PPUSATD0_EMU 0

// <q> CMU
// <i> Update secure access state of CMU before calling into bootloader
#define BOOTLOADER_PPUSATD0_CMU 0

// <q> HFRCO0
// <i> Update secure access state of HFRCO0 before calling into bootloader
#define BOOTLOADER_PPUSATD0_HFRCO0 0

// <q> FSRCO
// <i> Update secure access state of FSRCO before calling into bootloader
#define BOOTLOADER_PPUSATD0_FSRCO 0

// <q> DPLL0
// <i> Update secure access state of DPLL0 before calling into bootloader
#define BOOTLOADER_PPUSATD0_DPLL0 0

// <q> LFXO
// <i> Update secure access state of LFXO before calling into bootloader
#define BOOTLOADER_PPUSATD0_LFXO 0

// <q> LFRCO
// <i> Update secure access state of LFRCO before calling into bootloader
#define BOOTLOADER_PPUSATD0_LFRCO 0

// <q> ULFRCO
// <i> Update secure access state of ULFRCO before calling into bootloader
#define BOOTLOADER_PPUSATD0_ULFRCO 0

// <q> MSC
// <i> Update secure access state of MSC before calling into bootloader
#define BOOTLOADER_PPUSATD0_MSC 0

// <q> ICACHE0
// <i> Update secure access state of ICACHE0 before calling into bootloader
#define BOOTLOADER_PPUSATD0_ICACHE0 0

// <q> PRS
// <i> Update secure access state of PRS before calling into bootloader
#define BOOTLOADER_PPUSATD0_PRS 0

// <q> GPIO
// <i> Update secure access state of GPIO before calling into bootloader
#define BOOTLOADER_PPUSATD0_GPIO 0

// <q> LDMA
// <i> Update secure access state of LDMA before calling into bootloader
#define BOOTLOADER_PPUSATD0_LDMA 0

// <q> LDMAXBAR
// <i> Update secure access state of LDMAXBAR before calling into bootloader
#define BOOTLOADER_PPUSATD0_LDMAXBAR 0

// <q> TIMER0
// <i> Update secure access state of TIMER0 before calling into bootloader
#define BOOTLOADER_PPUSATD0_TIMER0 0

// <q> TIMER1
// <i> Update secure access state of TIMER1 before calling into bootloader
#define BOOTLOADER_PPUSATD0_TIMER1 0

// <q> TIMER2
// <i> Update secure access state of TIMER2 before calling into bootloader
#define BOOTLOADER_PPUSATD0_TIMER2 0

// <q> TIMER3
// <i> Update secure access state of TIMER3 before calling into bootloader
#define BOOTLOADER_PPUSATD0_TIMER3 0

// <q> TIMER4
// <i> Update secure access state of TIMER4 before calling into bootloader
#define BOOTLOADER_PPUSATD0_TIMER4 0

// <q> USART0
// <i> Update secure access state of USART0 before calling into bootloader
#define BOOTLOADER_PPUSATD0_USART0 0

// <q> BURTC
// <i> Update secure access state of BURTC before calling into bootloader
#define BOOTLOADER_PPUSATD0_BURTC 0

// <q> I2C1
// <i> Update secure access state of I2C1 before calling into bootloader
#define BOOTLOADER_PPUSATD0_I2C1 0

// <q> CHIPTESTCTRL
// <i> Update secure access state of CHIPTESTCTRL before calling into bootloader
#define BOOTLOADER_PPUSATD0_CHIPTESTCTRL 0

// <q> SYSCFGCFGNS
// <i> Update secure access state of SYSCFGCFGNS before calling into bootloader
#define BOOTLOADER_PPUSATD0_SYSCFGCFGNS 0

// <q> SYSCFG
// <i> Update secure access state of SYSCFG before calling into bootloader
#define BOOTLOADER_PPUSATD0_SYSCFG 0

// <q> BURAM
// <i> Update secure access state of BURAM before calling into bootloader
#define BOOTLOADER_PPUSATD0_BURAM 0

// <q> GPCRC
// <i> Update secure access state of GPCRC before calling into bootloader
#define BOOTLOADER_PPUSATD0_GPCRC 0

// <q> DCDC
// <i> Update secure access state of DCDC before calling into bootloader
#define BOOTLOADER_PPUSATD0_DCDC 0

// <q> HOSTMAILBOX
// <i> Update secure access state of HOSTMAILBOX before calling into bootloader
#define BOOTLOADER_PPUSATD0_HOSTMAILBOX 0

// <q> EUSART1
// <i> Update secure access state of EUSART1 before calling into bootloader
#define BOOTLOADER_PPUSATD0_EUSART1 0

// <q> SYSRTC
// <i> Update secure access state of SYSRTC before calling into bootloader
#define BOOTLOADER_PPUSATD0_SYSRTC 0

// <q> KEYSCAN
// <i> Update secure access state of KEYSCAN before calling into bootloader
#define BOOTLOADER_PPUSATD1_KEYSCAN 0

// <q> DMEM
// <i> Update secure access state of DMEM before calling into bootloader
#define BOOTLOADER_PPUSATD1_DMEM 0

// <q> RADIOAES
// <i> Update secure access state of RADIOAES before calling into bootloader
#define BOOTLOADER_PPUSATD1_RADIOAES 0

// <q> SMU
// <i> Update secure access state of SMU before calling into bootloader
#define BOOTLOADER_PPUSATD1_SMU 0

// <q> SMUCFGNS
// <i> Update secure access state of SMUCFGNS before calling into bootloader
#define BOOTLOADER_PPUSATD1_SMUCFGNS 0

// <q> LETIMER0
// <i> Update secure access state of LETIMER0 before calling into bootloader
#define BOOTLOADER_PPUSATD1_LETIMER0 0

// <q> IADC0
// <i> Update secure access state of IADC0 before calling into bootloader
#define BOOTLOADER_PPUSATD1_IADC0 0

// <q> ACMP0
// <i> Update secure access state of ACMP0 before calling into bootloader
#define BOOTLOADER_PPUSATD1_ACMP0 0

// <q> ACMP1
// <i> Update secure access state of ACMP1 before calling into bootloader
#define BOOTLOADER_PPUSATD1_ACMP1 0

// <q> AMUXCP0
// <i> Update secure access state of AMUXCP0 before calling into bootloader
#define BOOTLOADER_PPUSATD1_AMUXCP0 0

// <q> VDAC0
// <i> Update secure access state of VDAC0 before calling into bootloader
#define BOOTLOADER_PPUSATD1_VDAC0 0

// <q> VDAC1
// <i> Update secure access state of VDAC1 before calling into bootloader
#define BOOTLOADER_PPUSATD1_VDAC1 0

// <q> PCNT
// <i> Update secure access state of PCNT before calling into bootloader
#define BOOTLOADER_PPUSATD1_PCNT 0

// <q> HFRCO1
// <i> Update secure access state of HFRCO1 before calling into bootloader
#define BOOTLOADER_PPUSATD1_HFRCO1 0

// <q> HFXO0
// <i> Update secure access state of HFXO0 before calling into bootloader
#define BOOTLOADER_PPUSATD1_HFXO0 0

// <q> I2C0
// <i> Update secure access state of I2C0 before calling into bootloader
#define BOOTLOADER_PPUSATD1_I2C0 0

// <q> WDOG0
// <i> Update secure access state of WDOG0 before calling into bootloader
#define BOOTLOADER_PPUSATD1_WDOG0 0

// <q> WDOG1
// <i> Update secure access state of WDOG1 before calling into bootloader
#define BOOTLOADER_PPUSATD1_WDOG1 0

// <q> EUSART0
// <i> Update secure access state of EUSART0 before calling into bootloader
#define BOOTLOADER_PPUSATD1_EUSART0 0

// <q> SEMAILBOX
// <i> Update secure access state of SEMAILBOX before calling into bootloader
#define BOOTLOADER_PPUSATD1_SEMAILBOX 0

// <q> MVP
// <i> Update secure access state of MVP before calling into bootloader
#define BOOTLOADER_PPUSATD1_MVP 0

// <q> AHBRADIO
// <i> Update secure access state of AHBRADIO before calling into bootloader
#define BOOTLOADER_PPUSATD1_AHBRADIO 0
// </e>

// </h>
// <<< end of configuration section >>>

#if BOOTLOADER_MANUAL_OVERRIDE_SECURITY_STATE == 1
#define BOOTLOADER_PPUSATD0_MASK ((BOOTLOADER_PPUSATD0_EMU << _SMU_PPUSATD0_EMU_SHIFT) \
                                   | (BOOTLOADER_PPUSATD0_CMU << _SMU_PPUSATD0_CMU_SHIFT) \
                                   | (BOOTLOADER_PPUSATD0_HFRCO0 << _SMU_PPUSATD0_HFRCO0_SHIFT) \
                                   | (BOOTLOADER_PPUSATD0_FSRCO << _SMU_PPUSATD0_FSRCO_SHIFT) \
                                   | (BOOTLOADER_PPUSATD0_DPLL0 << _SMU_PPUSATD0_DPLL0_SHIFT) \
                                   | (BOOTLOADER_PPUSATD0_LFXO << _SMU_PPUSATD0_LFXO_SHIFT) \
                                   | (BOOTLOADER_PPUSATD0_LFRCO << _SMU_PPUSATD0_LFRCO_SHIFT) \
                                   | (BOOTLOADER_PPUSATD0_ULFRCO << _SMU_PPUSATD0_ULFRCO_SHIFT) \
                                   | (BOOTLOADER_PPUSATD0_MSC << _SMU_PPUSATD0_MSC_SHIFT) \
                                   | (BOOTLOADER_PPUSATD0_ICACHE0 << _SMU_PPUSATD0_ICACHE0_SHIFT) \
                                   | (BOOTLOADER_PPUSATD0_PRS << _SMU_PPUSATD0_PRS_SHIFT) \
                                   | (BOOTLOADER_PPUSATD0_GPIO << _SMU_PPUSATD0_GPIO_SHIFT) \
                                   | (BOOTLOADER_PPUSATD0_LDMA << _SMU_PPUSATD0_LDMA_SHIFT) \
                                   | (BOOTLOADER_PPUSATD0_LDMAXBAR << _SMU_PPUSATD0_LDMAXBAR_SHIFT) \
                                   | (BOOTLOADER_PPUSATD0_TIMER0 << _SMU_PPUSATD0_TIMER0_SHIFT) \
                                   | (BOOTLOADER_PPUSATD0_TIMER1 << _SMU_PPUSATD0_TIMER1_SHIFT) \
                                   | (BOOTLOADER_PPUSATD0_TIMER2 << _SMU_PPUSATD0_TIMER2_SHIFT) \
                                   | (BOOTLOADER_PPUSATD0_TIMER3 << _SMU_PPUSATD0_TIMER3_SHIFT) \
                                   | (BOOTLOADER_PPUSATD0_TIMER4 << _SMU_PPUSATD0_TIMER4_SHIFT) \
                                   | (BOOTLOADER_PPUSATD0_USART0 << _SMU_PPUSATD0_USART0_SHIFT) \
                                   | (BOOTLOADER_PPUSATD0_BURTC << _SMU_PPUSATD0_BURTC_SHIFT) \
                                   | (BOOTLOADER_PPUSATD0_I2C1 << _SMU_PPUSATD0_I2C1_SHIFT) \
                                   | (BOOTLOADER_PPUSATD0_CHIPTESTCTRL << _SMU_PPUSATD0_CHIPTESTCTRL_SHIFT) \
                                   | (BOOTLOADER_PPUSATD0_SYSCFGCFGNS << _SMU_PPUSATD0_SYSCFGCFGNS_SHIFT) \
                                   | (BOOTLOADER_PPUSATD0_SYSCFG << _SMU_PPUSATD0_SYSCFG_SHIFT) \
                                   | (BOOTLOADER_PPUSATD0_BURAM << _SMU_PPUSATD0_BURAM_SHIFT) \
                                   | (BOOTLOADER_PPUSATD0_GPCRC << _SMU_PPUSATD0_GPCRC_SHIFT) \
                                   | (BOOTLOADER_PPUSATD0_DCDC << _SMU_PPUSATD0_DCDC_SHIFT) \
                                   | (BOOTLOADER_PPUSATD0_HOSTMAILBOX << _SMU_PPUSATD0_HOSTMAILBOX_SHIFT) \
                                   | (BOOTLOADER_PPUSATD0_EUSART1 << _SMU_PPUSATD0_EUSART1_SHIFT) \
                                   | (BOOTLOADER_PPUSATD0_SYSRTC << _SMU_PPUSATD0_SYSRTC_SHIFT))
#define BOOTLOADER_PPUSATD1_MASK ((BOOTLOADER_PPUSATD1_KEYSCAN << _SMU_PPUSATD1_KEYSCAN_SHIFT) \
                                   | (BOOTLOADER_PPUSATD1_DMEM << _SMU_PPUSATD1_DMEM_SHIFT) \
                                   | (BOOTLOADER_PPUSATD1_RADIOAES << _SMU_PPUSATD1_RADIOAES_SHIFT) \
                                   | (BOOTLOADER_PPUSATD1_SMU << _SMU_PPUSATD1_SMU_SHIFT) \
                                   | (BOOTLOADER_PPUSATD1_SMUCFGNS << _SMU_PPUSATD1_SMUCFGNS_SHIFT) \
                                   | (BOOTLOADER_PPUSATD1_LETIMER0 << _SMU_PPUSATD1_LETIMER0_SHIFT) \
                                   | (BOOTLOADER_PPUSATD1_IADC0 << _SMU_PPUSATD1_IADC0_SHIFT) \
                                   | (BOOTLOADER_PPUSATD1_ACMP0 << _SMU_PPUSATD1_ACMP0_SHIFT) \
                                   | (BOOTLOADER_PPUSATD1_ACMP1 << _SMU_PPUSATD1_ACMP1_SHIFT) \
                                   | (BOOTLOADER_PPUSATD1_AMUXCP0 << _SMU_PPUSATD1_AMUXCP0_SHIFT) \
                                   | (BOOTLOADER_PPUSATD1_VDAC0 << _SMU_PPUSATD1_VDAC0_SHIFT) \
                                   | (BOOTLOADER_PPUSATD1_VDAC1 << _SMU_PPUSATD1_VDAC1_SHIFT) \
                                   | (BOOTLOADER_PPUSATD1_PCNT << _SMU_PPUSATD1_PCNT_SHIFT) \
                                   | (BOOTLOADER_PPUSATD1_HFRCO1 << _SMU_PPUSATD1_HFRCO1_SHIFT) \
                                   | (BOOTLOADER_PPUSATD1_HFXO0 << _SMU_PPUSATD1_HFXO0_SHIFT) \
                                   | (BOOTLOADER_PPUSATD1_I2C0 << _SMU_PPUSATD1_I2C0_SHIFT) \
                                   | (BOOTLOADER_PPUSATD1_WDOG0 << _SMU_PPUSATD1_WDOG0_SHIFT) \
                                   | (BOOTLOADER_PPUSATD1_WDOG1 << _SMU_PPUSATD1_WDOG1_SHIFT) \
                                   | (BOOTLOADER_PPUSATD1_EUSART0 << _SMU_PPUSATD1_EUSART0_SHIFT) \
                                   | (BOOTLOADER_PPUSATD1_SEMAILBOX << _SMU_PPUSATD1_SEMAILBOX_SHIFT) \
                                   | (BOOTLOADER_PPUSATD1_MVP << _SMU_PPUSATD1_MVP_SHIFT) \
                                   | (BOOTLOADER_PPUSATD1_AHBRADIO << _SMU_PPUSATD1_AHBRADIO_SHIFT))
#endif // BOOTLOADER_MANUAL_OVERRIDE_SECURITY_STATE

#endif // BTL_INTERFACE_CFG_S2C4_H
