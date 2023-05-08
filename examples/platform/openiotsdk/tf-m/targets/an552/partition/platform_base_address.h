/*
 * Copyright (c) 2019-2022 Arm Limited
 *
 * Licensed under the Apache License Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing software
 * distributed under the License is distributed on an "AS IS" BASIS
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/**
 * \file platform_base_address.h
 * \brief This file defines all the peripheral base addresses for AN552 MPS3 SSE-300 +
 *        Ethos-U55 platform.
 */

#ifndef __PLATFORM_BASE_ADDRESS_H__
#define __PLATFORM_BASE_ADDRESS_H__

/* ======= Defines peripherals memory map addresses ======= */
/* Non-secure memory map addresses */
#define ITCM_BASE_NS 0x00000000      /* Instruction TCM Non-Secure base address */
#define SRAM_BASE_NS 0x01000000      /* CODE SRAM Non-Secure base address */
#define DTCM0_BASE_NS 0x20000000     /* Data TCM block 0 Non-Secure base address */
#define DTCM1_BASE_NS 0x20020000     /* Data TCM block 1 Non-Secure base address */
#define DTCM2_BASE_NS 0x20040000     /* Data TCM block 2 Non-Secure base address */
#define DTCM3_BASE_NS 0x20060000     /* Data TCM block 3 Non-Secure base address */
#define ISRAM0_BASE_NS 0x21000000    /* Internal SRAM Area Non-Secure base address */
#define ISRAM1_BASE_NS 0x21100000    /* Internal SRAM Area Non-Secure base address */
#define QSPI_SRAM_BASE_NS 0x28000000 /* QSPI SRAM Non-Secure base address */
/* Non-Secure Subsystem peripheral region */
#define CPU0_PWRCTRL_BASE_NS 0x40012000  /* CPU 0 Power Control Block Non-Secure base address */
#define CPU0_IDENTITY_BASE_NS 0x4001F000 /* CPU 0 Identity Block Non-Secure base address */
#define SSE300_NSACFG_BASE_NS 0x40080000 /* SSE-300 Non-Secure Access Configuration Register Block Non-Secure base address */
/* Non-Secure MSTEXPPILL Peripheral region */
#define GPIO0_CMSDK_BASE_NS 0x41100000        /* GPIO 0 Non-Secure base address */
#define GPIO1_CMSDK_BASE_NS 0x41101000        /* GPIO 1 Non-Secure base address */
#define GPIO2_CMSDK_BASE_NS 0x41102000        /* GPIO 2 Non-Secure base address */
#define GPIO3_CMSDK_BASE_NS 0x41103000        /* GPIO 3 Non-Secure base address */
#define FMC_CMSDK_GPIO_0_BASE_NS 0x41104000   /* FMC CMSDK GPIO 0 Non-Secure base address */
#define FMC_CMSDK_GPIO_1_BASE_NS 0x41105000   /* FMC CMSDK GPIO 1 Non-Secure base address */
#define FMC_CMSDK_GPIO_2_BASE_NS 0x41106000   /* FMC CMSDK GPIO 2 Non-Secure base address */
#define FMC_CMSDK_GPIO_3_BASE_NS 0x41107000   /* FMC CMSDK GPIO 3 Non-Secure base address */
#define EXTERNAL_MANAGER_0_BASE_NS 0x41200000 /* External manager 0 (Unused) Non-Secure base address */
#define EXTERNAL_MANAGER_1_BASE_NS 0x41201000 /* External manager 1 (Unused) Non-Secure base address */
#define EXTERNAL_MANAGER_2_BASE_NS 0x41202000 /* External manager 2 (Unused) Non-Secure base address */
#define EXTERNAL_MANAGER_3_BASE_NS 0x41203000 /* External manager 3 (Unused) Non-Secure base address */
#define ETHERNET_BASE_NS 0x41400000           /* Ethernet Non-Secure base address */
#define USB_BASE_NS 0x41500000                /* USB Non-Secure base address */
#define USER_APB0_BASE_NS 0x41700000          /* User APB 0 Non-Secure base address */
#define USER_APB1_BASE_NS 0x41701000          /* User APB 1 Non-Secure base address */
#define USER_APB2_BASE_NS 0x41702000          /* User APB 2 Non-Secure base address */
#define USER_APB3_BASE_NS 0x41703000          /* User APB 3 Non-Secure base address */
#define QSPI_CONFIG_BASE_NS 0x41800000        /* QSPI Config Non-Secure base address */
#define QSPI_WRITE_BASE_NS 0x41801000         /* QSPI Write Non-Secure base address */
/* Non-Secure Subsystem peripheral region */
#define SYSTIMER0_ARMV8_M_BASE_NS 0x48000000       /* System Timer 0 Non-Secure base address */
#define SYSTIMER1_ARMV8_M_BASE_NS 0x48001000       /* System Timer 1 Non-Secure base address */
#define SYSTIMER2_ARMV8_M_BASE_NS 0x48002000       /* System Timer 2 Non-Secure base address */
#define SYSTIMER3_ARMV8_M_BASE_NS 0x48003000       /* System Timer 3 Non-Secure base address */
#define SSE300_SYSINFO_BASE_NS 0x48020000          /* SSE-300 System info Block Non-Secure base address */
#define SLOWCLK_TIMER_CMSDK_BASE_NS 0x4802F000     /* CMSDK based SLOWCLK Timer Non-Secure base address */
#define SYSWDOG_ARMV8_M_CNTRL_BASE_NS 0x48040000   /* Non-Secure Watchdog Timer control frame Non-Secure base address */
#define SYSWDOG_ARMV8_M_REFRESH_BASE_NS 0x48041000 /* Non-Secure Watchdog Timer refresh frame Non-Secure base address */
#define SYSCNTR_READ_BASE_NS 0x48101000            /* System Counter Read Secure base address */
/* Non-Secure MSTEXPPIHL Peripheral region */
#define ETHOS_U55_APB_BASE_NS 0x48102000        /* Ethos-U55 APB Non-Secure base address */
#define U55_TIMING_ADAPTER_0_BASE_NS 0x48103000 /* Ethos-U55 Timing Adapter 0 APB registers Non-Secure base address */
#define U55_TIMING_ADAPTER_1_BASE_NS 0x48103200 /* Ethos-U55 Timing Adapter 1 APB registers Non-Secure base address */
#define FPGA_SBCon_I2C_TOUCH_BASE_NS 0x49200000 /* FPGA - SBCon I2C (Touch) Non-Secure base address */
#define FPGA_SBCon_I2C_AUDIO_BASE_NS 0x49201000 /* FPGA - SBCon I2C (Audio Conf) Non-Secure base address */
#define FPGA_SPI_ADC_BASE_NS 0x49202000         /* FPGA - PL022 (SPI ADC) Non-Secure base address */
#define FPGA_SPI_SHIELD0_BASE_NS 0x49203000     /* FPGA - PL022 (SPI Shield0) Non-Secure base address */
#define FPGA_SPI_SHIELD1_BASE_NS 0x49204000     /* FPGA - PL022 (SPI Shield1) Non-Secure base address */
#define SBCon_I2C_SHIELD0_BASE_NS 0x49205000    /* SBCon (I2C - Shield0) Non-Secure base address */
#define SBCon_I2C_SHIELD1_BASE_NS 0x49206000    /* SBCon (I2C – Shield1) Non-Secure base address */
#define USER_APB_BASE_NS 0x49207000             /* USER APB Non-Secure base address */
#define FPGA_DDR4_EEPROM_BASE_NS 0x49208000     /* FPGA - SBCon I2C (DDR4 EEPROM) Non-Secure base address */
#define FMC_USER_APB0 0x4920C000                /* FMC User APB0 */
#define FMC_USER_APB1 0x4920D000                /* FMC User APB1 */
#define FMC_USER_APB2 0x4920E000                /* FMC User APB2 */
#define FMC_USER_APB3 0x4920F000                /* FMC User APB3 */
#define FPGA_SCC_BASE_NS 0x49300000             /* FPGA - SCC registers Non-Secure base address */
#define FPGA_I2S_BASE_NS 0x49301000             /* FPGA - I2S (Audio) Non-Secure base address */
#define FPGA_IO_BASE_NS 0x49302000              /* FPGA - IO (System Ctrl + I/O) Non-Secure base address */
#define UART0_BASE_NS 0x49303000                /* UART 0 Non-Secure base address */
#define UART1_BASE_NS 0x49304000                /* UART 1 Non-Secure base address */
#define UART2_BASE_NS 0x49305000                /* UART 2 Non-Secure base address */
#define UART3_BASE_NS 0x49306000                /* UART 3 Non-Secure base address */
#define UART4_BASE_NS 0x49307000                /* UART 4 Non-Secure base address */
#define UART5_BASE_NS 0x49308000                /* UART 5 Non-Secure base address */
#define CLCD_Config_Reg_BASE_NS 0x4930A000      /* CLCD Config Reg Non-Secure base address */
#define RTC_BASE_NS 0x4930B000                  /* RTC Non-Secure base address */
#define DDR4_BLK0_BASE_NS 0x60000000            /* DDR4 block 0 Non-Secure base address */
#define DDR4_BLK2_BASE_NS 0x80000000            /* DDR4 block 2 Non-Secure base address */
#define DDR4_BLK4_BASE_NS 0xA0000000            /* DDR4 block 4 Non-Secure base address */
#define DDR4_BLK6_BASE_NS 0xC0000000            /* DDR4 block 6 Non-Secure base address */

/* Secure memory map addresses */
#define ITCM_BASE_S 0x10000000      /* Instruction TCM Secure base address */
#define SRAM_BASE_S 0x11000000      /* CODE SRAM Secure base address */
#define DTCM0_BASE_S 0x30000000     /* Data TCM block 0 Secure base address */
#define DTCM1_BASE_S 0x30020000     /* Data TCM block 1 Secure base address */
#define DTCM2_BASE_S 0x30040000     /* Data TCM block 2 Secure base address */
#define DTCM3_BASE_S 0x30060000     /* Data TCM block 3 Secure base address */
#define ISRAM0_BASE_S 0x31000000    /* Internal SRAM Area Secure base address */
#define ISRAM1_BASE_S 0x31100000    /* Internal SRAM Area Secure base address */
#define QSPI_SRAM_BASE_S 0x38000000 /* QSPI SRAM Secure base address */
/* Secure Subsystem peripheral region */
#define CPU0_SECCTRL_BASE_S 0x50011000  /* CPU 0 Local Security Control Block Secure base address */
#define CPU0_PWRCTRL_BASE_S 0x50012000  /* CPU 0 Power Control Block Secure base address */
#define CPU0_IDENTITY_BASE_S 0x5001F000 /* CPU 0 Identity Block Secure base address */
#define SSE300_SACFG_BASE_S 0x50080000  /* SSE-300 Secure Access Configuration Register Secure base address */
#define MPC_ISRAM0_BASE_S 0x50083000    /* Internal SRAM0 Memory Protection Controller Secure base address */
#define MPC_ISRAM1_BASE_S 0x50084000    /* Internal SRAM1 Memory Protection Controller Secure base address */
/* Secure MSTEXPPILL Peripheral region */
#define GPIO0_CMSDK_BASE_S 0x51100000       /* GPIO 0 Secure base address */
#define GPIO1_CMSDK_BASE_S 0x51101000       /* GPIO 1 Secure base address */
#define GPIO2_CMSDK_BASE_S 0x51102000       /* GPIO 2 Secure base address */
#define GPIO3_CMSDK_BASE_S 0x51103000       /* GPIO 3 Secure base address */
#define FMC_CMSDK_GPIO_0_BASE_S 0x51104000  /* FMC CMSDK GPIO 0 Secure base address */
#define FMC_CMSDK_GPIO_1_BASE_S 0x51105000  /* FMC CMSDK GPIO 1 Secure base address */
#define FMC_CMSDK_GPIO_2_BASE_S 0x51106000  /* FMC CMSDK GPIO 2 Secure base address */
#define FMC_CMSDK_GPIO_3_BASE_S 0x51107000  /* FMC CMSDK GPIO 3 Secure base address */
#define EXTERNAL_MANAGER0_BASE_S 0x51200000 /* External Manager 0 (Unused) Secure base address */
#define EXTERNAL_MANAGER1_BASE_S 0x51201000 /* External Manager 1 (Unused) Secure base address */
#define EXTERNAL_MANAGER2_BASE_S 0x51202000 /* External Manager 2 (Unused) Secure base address */
#define EXTERNAL_MANAGER3_BASE_S 0x51203000 /* External Manager 3 (Unused) Secure base address */
#define ETHERNET_BASE_S 0x51400000          /* Ethernet Secure base address */
#define USB_BASE_S 0x51500000               /* USB Secure base address */
#define USER_APB0_BASE_S 0x51700000         /* User APB 0 Secure base address */
#define USER_APB1_BASE_S 0x51701000         /* User APB 1 Secure base address */
#define USER_APB2_BASE_S 0x51702000         /* User APB 2 Secure base address */
#define USER_APB3_BASE_S 0x51703000         /* User APB 3 Secure base address */
#define QSPI_CONFIG_BASE_S 0x51800000       /* QSPI Config Secure base address */
#define QSPI_WRITE_BASE_S 0x51801000        /* QSPI Write Secure base address */
#define MPC_SRAM_BASE_S 0x57000000          /* SRAM Memory Protection Controller Secure base address */
#define MPC_QSPI_BASE_S 0x57001000          /* QSPI Memory Protection Controller Secure base address */
#define MPC_DDR4_BASE_S 0x57002000          /* DDR4 Memory Protection Controller Secure base address */
/* Secure Subsystem peripheral region */
#define SYSTIMER0_ARMV8_M_BASE_S 0x58000000       /* System Timer 0 Secure base address */
#define SYSTIMER1_ARMV8_M_BASE_S 0x58001000       /* System Timer 1 Secure base address */
#define SYSTIMER2_ARMV8_M_BASE_S 0x58002000       /* System Timer 0 Secure base address */
#define SYSTIMER3_ARMV8_M_BASE_S 0x58003000       /* System Timer 1 Secure base address */
#define SSE300_SYSINFO_BASE_S 0x58020000          /* SSE-300 System info Block Secure base address */
#define SSE300_SYSCTRL_BASE_S 0x58021000          /* SSE-300 System control Block Secure base address */
#define SSE300_SYSPPU_BASE_S 0x58022000           /* SSE-300 System Power Policy Unit Secure base address */
#define SSE300_CPU0PPU_BASE_S 0x58023000          /* SSE-300 CPU 0 Power Policy Unit Secure base address */
#define SSE300_MGMTPPU_BASE_S 0x58028000          /* SSE-300 Management Power Policy Unit Secure base address */
#define SSE300_DBGPPU_BASE_S 0x58029000           /* SSE-300 Debug Power Policy Unit Secure base address */
#define SLOWCLK_WDOG_CMSDK_BASE_S 0x5802E000      /* CMSDK based SLOWCLK Watchdog Secure base address */
#define SLOWCLK_TIMER_CMSDK_BASE_S 0x5802F000     /* CMSDK based SLOWCLK Timer Secure base address */
#define SYSWDOG_ARMV8_M_CNTRL_BASE_S 0x58040000   /* Secure Watchdog Timer control frame Secure base address */
#define SYSWDOG_ARMV8_M_REFRESH_BASE_S 0x58041000 /* Secure Watchdog Timer refresh frame Secure base address */
#define SYSCNTR_CNTRL_BASE_S 0x58100000           /* System Counter Control Secure base address */
#define SYSCNTR_READ_BASE_S 0x58101000            /* System Counter Read Secure base address */
/* Secure MSTEXPPIHL Peripheral region */
#define ETHOS_U55_APB_BASE_S 0x58102000        /* Ethos-U55 APB Secure base address */
#define U55_TIMING_ADAPTER_0_BASE_S 0x58103000 /* Ethos-U55 Timing Adapter 0 APB registers Secure base address */
#define U55_TIMING_ADAPTER_1_BASE_S 0x58103200 /* Ethos-U55 Timing Adapter 1 APB registers Secure base address */
#define FPGA_SBCon_I2C_TOUCH_BASE_S 0x59200000 /* FPGA - SBCon I2C (Touch) Secure base address */
#define FPGA_SBCon_I2C_AUDIO_BASE_S 0x59201000 /* FPGA - SBCon I2C (Audio Conf) Secure base address */
#define FPGA_SPI_ADC_BASE_S 0x59202000         /* FPGA - PL022 (SPI ADC) Secure base address */
#define FPGA_SPI_SHIELD0_BASE_S 0x59203000     /* FPGA - PL022 (SPI Shield0) Secure base address */
#define FPGA_SPI_SHIELD1_BASE_S 0x59204000     /* FPGA - PL022 (SPI Shield1) Secure base address */
#define SBCon_I2C_SHIELD0_BASE_S 0x59205000    /* SBCon (I2C - Shield0) Secure base address */
#define SBCon_I2C_SHIELD1_BASE_S 0x59206000    /* SBCon (I2C – Shield1) Secure base address */
#define USER_APB_BASE_S 0x59207000             /* USER APB Secure base address */
#define FPGA_DDR4_EEPROM_BASE_S 0x59208000     /* FPGA - SBCon I2C (DDR4 EEPROM) Secure base address */
#define FMC_USER_APB_0_BASE_S 0x5920C000       /* FMC User APB0 registers Secure base address */
#define FMC_USER_APB_1_BASE_S 0x5920D000       /* FMC User APB1 registers Secure base address */
#define FMC_USER_APB_2_BASE_S 0x5920E000       /* FMC User APB2 registers Secure base address */
#define FMC_USER_APB_3_BASE_S 0x5920F000       /* FMC User APB3 registers Secure base address */
#define FPGA_SCC_BASE_S 0x59300000             /* FPGA - SCC registers Secure base address */
#define FPGA_I2S_BASE_S 0x59301000             /* FPGA - I2S (Audio) Secure base address */
#define FPGA_IO_BASE_S 0x59302000              /* FPGA - IO (System Ctrl + I/O) Secure base address */
#define UART0_BASE_S 0x59303000                /* UART 0 Secure base address */
#define UART1_BASE_S 0x59304000                /* UART 1 Secure base address */
#define UART2_BASE_S 0x59305000                /* UART 2 Secure base address */
#define UART3_BASE_S 0x59306000                /* UART 3 Secure base address */
#define UART4_BASE_S 0x59307000                /* UART 4 Secure base address */
#define UART5_BASE_S 0x59308000                /* UART 5 Secure base address */
#define CLCD_Config_Reg_BASE_S 0x5930A000      /* CLCD Config Reg Secure base address */
#define RTC_BASE_S 0x5930B000                  /* RTC Secure base address */
#define DDR4_BLK1_BASE_S 0x70000000            /* DDR4 block 1 Secure base address */
#define DDR4_BLK3_BASE_S 0x90000000            /* DDR4 block 3 Secure base address */
#define DDR4_BLK5_BASE_S 0xB0000000            /* DDR4 block 5 Secure base address */
#define DDR4_BLK7_BASE_S 0xD0000000            /* DDR4 block 7 Secure base address */

/* Memory map addresses exempt from memory attribution by both the SAU and IDAU */
#define SSE300_EWIC_BASE                                                                                                           \
    0xE0047000 /* External Wakeup Interrupt Controller                                                                             \
                * Access from Non-secure software is only allowed                                                                  \
                * if AIRCR.BFHFNMINS is set to 1 */

/* Memory size definitions */
#define ITCM_SIZE (0x00080000)      /* 512 kB */
#define DTCM_BLK_SIZE (0x00020000)  /* 128 kB */
#define DTCM_BLK_NUM (0x4)          /* Number of DTCM blocks */
#define SRAM_SIZE (0x00100000)      /* 1 MB */
#define ISRAM0_SIZE (0x00100000)    /* 1 MB */
#define ISRAM1_SIZE (0x00100000)    /* 1 MB */
#define QSPI_SRAM_SIZE (0x00800000) /* 8 MB */
#define DDR4_BLK_SIZE (0x10000000)  /* 256 MB */
#define DDR4_BLK_NUM (0x8)          /* Number of DDR4 blocks */

/* Defines for Driver MPC's */
/* SRAM -- 1 MB */
#define MPC_SRAM_RANGE_BASE_NS (SRAM_BASE_NS)
#define MPC_SRAM_RANGE_LIMIT_NS (SRAM_BASE_NS + SRAM_SIZE - 1)
#define MPC_SRAM_RANGE_OFFSET_NS (0x0)
#define MPC_SRAM_RANGE_BASE_S (SRAM_BASE_S)
#define MPC_SRAM_RANGE_LIMIT_S (SRAM_BASE_S + SRAM_SIZE - 1)
#define MPC_SRAM_RANGE_OFFSET_S (0x0)

/* QSPI -- 8 MB*/
#define MPC_QSPI_RANGE_BASE_NS (QSPI_SRAM_BASE_NS)
#define MPC_QSPI_RANGE_LIMIT_NS (QSPI_SRAM_BASE_NS + QSPI_SRAM_SIZE - 1)
#define MPC_QSPI_RANGE_OFFSET_NS (0x0)
#define MPC_QSPI_RANGE_BASE_S (QSPI_SRAM_BASE_S)
#define MPC_QSPI_RANGE_LIMIT_S (QSPI_SRAM_BASE_S + QSPI_SRAM_SIZE - 1)
#define MPC_QSPI_RANGE_OFFSET_S (0x0)

/* ISRAM0 -- 1 MB*/
#define MPC_ISRAM0_RANGE_BASE_NS (ISRAM0_BASE_NS)
#define MPC_ISRAM0_RANGE_LIMIT_NS (ISRAM0_BASE_NS + ISRAM0_SIZE - 1)
#define MPC_ISRAM0_RANGE_OFFSET_NS (0x0)
#define MPC_ISRAM0_RANGE_BASE_S (ISRAM0_BASE_S)
#define MPC_ISRAM0_RANGE_LIMIT_S (ISRAM0_BASE_S + ISRAM0_SIZE - 1)
#define MPC_ISRAM0_RANGE_OFFSET_S (0x0)

/* ISRAM1 -- 1 MB*/
#define MPC_ISRAM1_RANGE_BASE_NS (ISRAM1_BASE_NS)
#define MPC_ISRAM1_RANGE_LIMIT_NS (ISRAM1_BASE_NS + ISRAM1_SIZE - 1)
#define MPC_ISRAM1_RANGE_OFFSET_NS (0x0)
#define MPC_ISRAM1_RANGE_BASE_S (ISRAM1_BASE_S)
#define MPC_ISRAM1_RANGE_LIMIT_S (ISRAM1_BASE_S + ISRAM1_SIZE - 1)
#define MPC_ISRAM1_RANGE_OFFSET_S (0x0)

/* DDR4 -- 2GB (8 * 256 MB) */
#define MPC_DDR4_BLK0_RANGE_BASE_NS (DDR4_BLK0_BASE_NS)
#define MPC_DDR4_BLK0_RANGE_LIMIT_NS (DDR4_BLK0_BASE_NS + ((DDR4_BLK_SIZE) -1))
#define MPC_DDR4_BLK0_RANGE_OFFSET_NS (0x0)
#define MPC_DDR4_BLK1_RANGE_BASE_S (DDR4_BLK1_BASE_S)
#define MPC_DDR4_BLK1_RANGE_LIMIT_S (DDR4_BLK1_BASE_S + ((DDR4_BLK_SIZE) -1))
#define MPC_DDR4_BLK1_RANGE_OFFSET_S (DDR4_BLK1_BASE_S - DDR4_BLK0_BASE_NS)
#define MPC_DDR4_BLK2_RANGE_BASE_NS (DDR4_BLK2_BASE_NS)
#define MPC_DDR4_BLK2_RANGE_LIMIT_NS (DDR4_BLK2_BASE_NS + ((DDR4_BLK_SIZE) -1))
#define MPC_DDR4_BLK2_RANGE_OFFSET_NS (DDR4_BLK2_BASE_NS - DDR4_BLK0_BASE_NS)
#define MPC_DDR4_BLK3_RANGE_BASE_S (DDR4_BLK3_BASE_S)
#define MPC_DDR4_BLK3_RANGE_LIMIT_S (DDR4_BLK3_BASE_S + ((DDR4_BLK_SIZE) -1))
#define MPC_DDR4_BLK3_RANGE_OFFSET_S (DDR4_BLK3_BASE_S - DDR4_BLK0_BASE_NS)
#define MPC_DDR4_BLK4_RANGE_BASE_NS (DDR4_BLK4_BASE_NS)
#define MPC_DDR4_BLK4_RANGE_LIMIT_NS (DDR4_BLK4_BASE_NS + ((DDR4_BLK_SIZE) -1))
#define MPC_DDR4_BLK4_RANGE_OFFSET_NS (DDR4_BLK4_BASE_NS - DDR4_BLK0_BASE_NS)
#define MPC_DDR4_BLK5_RANGE_BASE_S (DDR4_BLK5_BASE_S)
#define MPC_DDR4_BLK5_RANGE_LIMIT_S (DDR4_BLK5_BASE_S + ((DDR4_BLK_SIZE) -1))
#define MPC_DDR4_BLK5_RANGE_OFFSET_S (DDR4_BLK5_BASE_S - DDR4_BLK0_BASE_NS)
#define MPC_DDR4_BLK6_RANGE_BASE_NS (DDR4_BLK6_BASE_NS)
#define MPC_DDR4_BLK6_RANGE_LIMIT_NS (DDR4_BLK6_BASE_NS + ((DDR4_BLK_SIZE) -1))
#define MPC_DDR4_BLK6_RANGE_OFFSET_NS (DDR4_BLK6_BASE_NS - DDR4_BLK0_BASE_NS)
#define MPC_DDR4_BLK7_RANGE_BASE_S (DDR4_BLK7_BASE_S)
#define MPC_DDR4_BLK7_RANGE_LIMIT_S (DDR4_BLK7_BASE_S + ((DDR4_BLK_SIZE) -1))
#define MPC_DDR4_BLK7_RANGE_OFFSET_S (DDR4_BLK7_BASE_S - DDR4_BLK0_BASE_NS)

#endif /* __PLATFORM_BASE_ADDRESS_H__ */
