/*
 * Copyright (c) 2019 Arm Limited
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
 * \file syscounter_armv8-m_cntrl_drv.h
 *
 * \brief Driver for Armv8-M System Counter Control, covering CNTControlBase
 *        Frame
 *
 * This System Counter is a 64-bit up-counter, generating the physical
 * count for System Timer.
 *
 * Main features:
 *   - Enable/disable and Set/Get the 64-bit upcounter
 *   - 2 scaling registers for the 2 clock sources
 *       - These registers are used to pre-program the scaling values so
 *          that when hardware based clock switching is implemented there is no
 *          need to program the scaling increment value each time when clock is
 *          switched.
 *        - When counter scaling is enabled, ScaleVal is the value added to the
 *          Counter Count Value for every period of the counter as determined
 *          by 1/Frequency from the current operating frequency of the system
 *          counter (the �counter tick�).
 *        - ScaleVal is expressed as an unsigned fixed-point number with
 *          a 8 bit integer value and a 24-bit fractional value
 *   - Interrupt for error detection
 *       There are 2 possible reasons for error notification generation from
 *       the Counter:
 *         1. Security attribute mismatch between register access and security
 *            attribute of the CONTROL frame
 *         2. Address decode error within a given frame
 *
 */

#ifndef __SYSCOUNTER_ARMV8_M_CNTRL_DRV_H__
#define __SYSCOUNTER_ARMV8_M_CNTRL_DRV_H__

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define SYSCOUNTER_ARMV8_M_CNTRL_REGISTER_BIT_WIDTH 32u
/*!< Armv8-M System Counter Control registers bit width */

#define SYSCOUNTER_ARMV8_M_DEFAULT_INIT_CNT_VAL 0u
/*!< Armv8-M System Counter Control default counter init value */

/**
 * \brief Armv8-M System Counter Control scaling value
 */
struct syscounter_armv8_m_cntrl_scale_val_t
{
    uint32_t integer;              /* 8 bit */
    uint32_t fixed_point_fraction; /* 24 bit */
};

/**
 * \brief Armv8-M System Counter Control scaling value macros *
 *        8 bit integer and 24 bit fixed point fractional value
 */
#define SYSCOUNTER_ARMV8_M_SCALE_VAL_INT_MAX UINT8_MAX
#define SYSCOUNTER_ARMV8_M_SCALE_VAL_INT_OFF 24u
#define SYSCOUNTER_ARMV8_M_SCALE_VAL_FRACT_MAX ((1u << SYSCOUNTER_ARMV8_M_SCALE_VAL_INT_OFF) - 1u)

/**
 *  \brief Armv8-M System Counter Control device configuration structure
 */
struct syscounter_armv8_m_cntrl_dev_cfg_t
{
    const uint32_t base;
    /*!< Armv8-M System Counter Control device base address */
    struct syscounter_armv8_m_cntrl_scale_val_t scale0;
    /*!< Default clock scaling value for Clock source 0 */
    struct syscounter_armv8_m_cntrl_scale_val_t scale1;
    /*!< Default clock scaling value for Clock source 1 */
};

/**
 * \brief Armv8-M System Counter Control device data structure
 */
struct syscounter_armv8_m_cntrl_dev_data_t
{
    bool is_initialized;
};

/**
 * \brief Armv8-M System Counter Control device structure
 */
struct syscounter_armv8_m_cntrl_dev_t
{
    const struct syscounter_armv8_m_cntrl_dev_cfg_t * const cfg;
    /*!< Armv8-M System Counter Control configuration structure */
    struct syscounter_armv8_m_cntrl_dev_data_t * const data;
    /*!< Armv8-M System Counter Control data structure */
};

/**
 * \brief Armv8-M System Counter Control error enumeration types
 */
enum syscounter_armv8_m_cntrl_error_t
{
    SYSCOUNTER_ARMV8_M_ERR_NONE        = 0u,
    SYSCOUNTER_ARMV8_M_ERR_INVALID     = 1u,
    SYSCOUNTER_ARMV8_M_ERR_INVALID_ARG = 2u
};

/**
 * \brief Armv8-M System Counter Control scaling number for each clock sources
 */
enum syscounter_armv8_m_cntrl_scale_nr_t
{
    SYSCOUNTER_ARMV8_M_SCALE_NR_0 = 0u,
    /*!< Scaling for \ref SYSCOUNTER_ARMV8_M_SELCLK_CLK0 */
    SYSCOUNTER_ARMV8_M_SCALE_NR_1 = 1u
    /*!< Scaling for \ref SYSCOUNTER_ARMV8_M_SELCLK_CLK1 */
};

/**
 * \brief Clock select values
 */
enum syscounter_armv8_m_cntrl_selclk_t
{
    SYSCOUNTER_ARMV8_M_SELCLK_CLK_INVALID0 = 0u,
    /*!< Clock select invalid value */
    SYSCOUNTER_ARMV8_M_SELCLK_CLK0 = 1u,
    /*!< Clock select clock source 0 */
    SYSCOUNTER_ARMV8_M_SELCLK_CLK1 = 2u,
    /*!< Clock select clock source 1 */
    SYSCOUNTER_ARMV8_M_SELCLK_CLK_INVALID1 = 3u
    /*!< Clock select invalid value */
};

/**
 * \brief Override counter enable condition for writing to CNTSCR registers
 *
 */
enum syscounter_armv8_m_cntrl_cntscr_ovr_t
{
    SYSCOUNTER_ARMV8_M_CNTSCR_IF_DISABLED = 0u,
    /*!< Scaling registers can be written only when counter is disabled */
    SYSCOUNTER_ARMV8_M_CNTSCR_ALWAYS = 1u
    /*!< CNTSCR can be written regardless of counter enabled or disabled */
};

/**
 * \brief Initializes counter to a known default state, which is:
 *          - counter is enabled, so starts counting
 *          - interrupt is disabled
 *          - counter reset to default reset value
 *              \ref SYSCOUNTER_ARMV8_M_DEFAULT_INIT_CNT_VAL
 *          - scaling is disabled
 *          - scaling registers are set to the set values:
 *              \ref struct syscounter_armv8_m_cntrl_dev_cfg_t
 *        Init should be called prior to any other process and
 *        it's the caller's responsibility to follow proper call order.
 *        More than one call results fall through.
 *
 * \param[in] dev Counter device struct \ref syscounter_armv8_m_cntrl_dev_t
 *
 * \return Error status \ref enum syscounter_armv8_m_cntrl_error_t
 *
 * \note This function doesn't check if dev is NULL.
 */
enum syscounter_armv8_m_cntrl_error_t syscounter_armv8_m_cntrl_init(struct syscounter_armv8_m_cntrl_dev_t * dev);

/**
 * \brief Uninitializes counter to a known default state, which is:
 *          - counter is disabled, so stops counting
 *          - interrupt is disabled
 *          - counter reset to default reset value
 *              \ref SYSCOUNTER_ARMV8_M_DEFAULT_INIT_CNT_VAL
 *          - scaling is disabled
 *        Init should be called prior to any other process and
 *        it's the caller's responsibility to follow proper call order.
 *
 * \param[in] dev Counter device struct \ref syscounter_armv8_m_cntrl_dev_t
 *
 * \note This function doesn't check if dev is NULL.
 */
void syscounter_armv8_m_cntrl_uninit(struct syscounter_armv8_m_cntrl_dev_t * dev);

/**
 * \brief Enables the counter, so counter starts counting
 *
 * \param[in] dev Counter device struct \ref syscounter_armv8_m_cntrl_dev_t
 *
 * \note This function doesn't check if dev is NULL.
 */
void syscounter_armv8_m_cntrl_enable_counter(struct syscounter_armv8_m_cntrl_dev_t * dev);

/**
 * \brief Disables the counter, so counter stops counting
 *
 * \param[in] dev Counter device struct \ref syscounter_armv8_m_cntrl_dev_t
 *
 * \note This function doesn't check if dev is NULL.
 */
void syscounter_armv8_m_cntrl_disable_counter(struct syscounter_armv8_m_cntrl_dev_t * dev);

/**
 * \brief Polls counter enable status
 *
 * \param[in] dev Counter device struct \ref syscounter_armv8_m_cntrl_dev_t
 *
 * \return true if enabled, false otherwise
 *
 * \note This function doesn't check if dev is NULL.
 */
bool syscounter_armv8_m_cntrl_is_counter_enabled(struct syscounter_armv8_m_cntrl_dev_t * dev);

/**
 * \brief Enables Halt-On-Debug feature
 *
 * \param[in] dev Counter device struct \ref syscounter_armv8_m_cntrl_dev_t
 *
 * \note This function doesn't check if dev is NULL.
 */
void syscounter_armv8_m_cntrl_enable_halt_on_debug(struct syscounter_armv8_m_cntrl_dev_t * dev);

/**
 * \brief Disables Halt-On-Debug feature
 *
 * \param[in] dev Counter device struct \ref syscounter_armv8_m_cntrl_dev_t
 *
 * \note This function doesn't check if dev is NULL.
 */
void syscounter_armv8_m_cntrl_disable_halt_on_debug(struct syscounter_armv8_m_cntrl_dev_t * dev);

/**
 * \brief Polls Halt-On-Debug enablement status
 *
 * \param[in] dev Counter device struct \ref syscounter_armv8_m_cntrl_dev_t

 * \return true if enabled, false otherwise
 *
 * \note This function doesn't check if dev is NULL.
 */
bool syscounter_armv8_m_cntrl_is_halt_on_debug_enabled(struct syscounter_armv8_m_cntrl_dev_t * dev);

/**
 * \brief Enables scaling
 *        The used scaling register is depending on the used HW clock source.
 *
 * \param[in] dev Counter device struct \ref syscounter_armv8_m_cntrl_dev_t
 *
 * \note This function doesn't check if dev is NULL.
 */
void syscounter_armv8_m_cntrl_enable_scale(struct syscounter_armv8_m_cntrl_dev_t * dev);

/**
 * \brief Disables scaling
 *        Counter count will be incremented by default 1 for each ticks.
 *
 * \param[in] dev Counter device struct \ref syscounter_armv8_m_cntrl_dev_t
 *
 * \note This function doesn't check if dev is NULL.
 */
void syscounter_armv8_m_cntrl_disable_scale(struct syscounter_armv8_m_cntrl_dev_t * dev);

/**
 * \brief Polls scaling enablement status
 *
 * \param[in] dev Counter device struct \ref syscounter_armv8_m_cntrl_dev_t
 *
 * \return true if enabled, false otherwise
 *
 * \note This function doesn't check if dev is NULL.
 */
bool syscounter_armv8_m_cntrl_is_scale_enabled(struct syscounter_armv8_m_cntrl_dev_t * dev);

/**
 * \brief Enables interrupt
 *
 *  There are 2 possible reasons for error notification generation from
 *  the Counter:
 *   1. Security attribute mismatch between register access and security
 *       attribute of the CONTROL frame
 *   2. Address decode error within a given frame
 *
 * \param[in] dev Counter device struct \ref syscounter_armv8_m_cntrl_dev_t
 *
 * \note This function doesn't check if dev is NULL.
 */
void syscounter_armv8_m_cntrl_enable_interrupt(struct syscounter_armv8_m_cntrl_dev_t * dev);

/**
 * \brief Disables interrupt
 *
 * \param[in] dev Counter device struct \ref syscounter_armv8_m_cntrl_dev_t
 *
 * \note This function doesn't check if dev is NULL.
 */
void syscounter_armv8_m_cntrl_disable_interrupt(struct syscounter_armv8_m_cntrl_dev_t * dev);

/**
 * \brief Polls interrupt enablement status
 *
 * \param[in] dev Counter device struct \ref syscounter_armv8_m_cntrl_dev_t
 *
 * \return true if enabled, false otherwise
 *
 * \note This function doesn't check if dev is NULL.
 */
bool syscounter_armv8_m_cntrl_is_interrupt_enabled(struct syscounter_armv8_m_cntrl_dev_t * dev);

/**
 * \brief Enables PSLVERR output
 *
 *        PSLVERR output signal on APB bus dynamically generated for the
 *        following error:
 *          For security attribute mismatch between register access and security
 *          attribute of the CONTROL frame
 *
 * \param[in] dev Counter device struct \ref syscounter_armv8_m_cntrl_dev_t
 *
 * \note This function doesn't check if dev is NULL.
 */
void syscounter_armv8_m_cntrl_enable_pslverr(struct syscounter_armv8_m_cntrl_dev_t * dev);

/**
 * \brief Disables PSLVERR output
 *
 * \param[in] dev Counter device struct \ref syscounter_armv8_m_cntrl_dev_t
 *
 * \note This function doesn't check if dev is NULL.
 */
void syscounter_armv8_m_cntrl_disable_pslverr(struct syscounter_armv8_m_cntrl_dev_t * dev);

/**
 * \brief Polls PSLVERR output enablement status
 *
 * \param[in] dev Counter device struct \ref syscounter_armv8_m_cntrl_dev_t
 *
 * \return true if enabled, false otherwise
 *
 * \note This function doesn't check if dev is NULL.
 */
bool syscounter_armv8_m_cntrl_is_pslverr_enabled(struct syscounter_armv8_m_cntrl_dev_t * dev);

/**
 * \brief Clears interrupt pending flag
 *
 * \param[in] dev Counter device struct \ref syscounter_armv8_m_cntrl_dev_t
 *
 * \note This function doesn't check if dev is NULL.
 */
void syscounter_armv8_m_cntrl_clear_interrupt(struct syscounter_armv8_m_cntrl_dev_t * dev);

/**
 * \brief Polls Halt-On-Debug status
 *
 * \param[in] dev Counter device struct \ref syscounter_armv8_m_cntrl_dev_t
 *
 * \return true if counter is halted, false otherwise
 *
 * \note This function doesn't check if dev is NULL.
 */
bool syscounter_armv8_m_cntrl_is_counter_halted_on_debug(struct syscounter_armv8_m_cntrl_dev_t * dev);

/**
 * \brief Read counter value
 *
 * \param[in] dev Counter device struct \ref syscounter_armv8_m_cntrl_dev_t
 *
 * \return 64 bit counter value
 *
 * \note This function doesn't check if dev is NULL.
 */
uint64_t syscounter_armv8_m_cntrl_get_counter_value(struct syscounter_armv8_m_cntrl_dev_t * dev);

/**
 * \brief Writes counter value
 *
 * \param[in] dev Counter device struct \ref syscounter_armv8_m_cntrl_dev_t
 * \param[in] value 64 bit counter value
 *
 * \note This function doesn't check if dev is NULL.
 */
void syscounter_armv8_m_cntrl_set_counter_value(struct syscounter_armv8_m_cntrl_dev_t * dev, uint64_t value);

/**
 * \brief Polls whether scaling is implemented
 *
 * \param[in] dev Counter device struct \ref syscounter_armv8_m_cntrl_dev_t
 *
 * \return true if implemented, false otherwise
 *
 * \note This function doesn't check if dev is NULL.
 */
bool syscounter_armv8_m_cntrl_is_counter_scaling_implemented(struct syscounter_armv8_m_cntrl_dev_t * dev);

/**
 * \brief Polls whether HW based clock switching is implemented
 *
 * \param[in] dev Counter device struct \ref syscounter_armv8_m_cntrl_dev_t
 *
 * \return true if implemented, false otherwise
 *
 * \note This function doesn't check if dev is NULL.
 */
bool syscounter_armv8_m_cntrl_is_clock_switching_implemented(struct syscounter_armv8_m_cntrl_dev_t * dev);

/**
 * \brief Reads which clock source is being used
 *
 * \param[in] dev Counter device struct \ref syscounter_armv8_m_cntrl_dev_t
 *
 * \return Clock source \ref enum syscounter_armv8_m_cntrl_selclk_t
 *
 * \note This function doesn't check if dev is NULL.
 */
enum syscounter_armv8_m_cntrl_selclk_t syscounter_armv8_m_cntrl_get_clock_source(struct syscounter_armv8_m_cntrl_dev_t * dev);

/**
 * \brief Reads scaling register can be overridden anytime
 *
 * \param[in] dev Counter device struct \ref syscounter_armv8_m_cntrl_dev_t
 *
 * \return Override condition \ref enum syscounter_armv8_m_cntrl_cntscr_ovr_t
 *
 * \note This function doesn't check if dev is NULL.
 */
enum syscounter_armv8_m_cntrl_cntscr_ovr_t
syscounter_armv8_m_cntrl_get_override_cntscr(struct syscounter_armv8_m_cntrl_dev_t * dev);

/**
 * \brief Reads scaling register
 *
 * \param[in] dev Counter device struct \ref syscounter_armv8_m_cntrl_dev_t
 * \param[in] nr Index of scaling register to read
 *              \ref enum syscounter_armv8_m_cntrl_scale_nr_t
 * \param[out] nr Pointer to structure to read the scale value
 *                \ref struct syscounter_armv8_m_cntrl_scale_val_t
 *
 * \return Override condition \ref enum syscounter_armv8_m_cntrl_cntscr_ovr_t
 *
 * \note This function doesn't check if dev is NULL.
 */
enum syscounter_armv8_m_cntrl_error_t
syscounter_armv8_m_cntrl_get_counter_scale_value(struct syscounter_armv8_m_cntrl_dev_t * dev,
                                                 enum syscounter_armv8_m_cntrl_scale_nr_t nr,
                                                 struct syscounter_armv8_m_cntrl_scale_val_t * val);

/**
 * \brief Writes scaling register
 *
 * \param[in] dev Counter device struct \ref syscounter_armv8_m_cntrl_dev_t
 * \param[in] nr Index of scaling register to write
 *              \ref enum syscounter_armv8_m_cntrl_scale_nr_t
 * \param[in] Scale value structure
 *              \ref struct syscounter_armv8_m_cntrl_scale_val_t
 *
 * \return Error status \ref enum syscounter_armv8_m_cntrl_error_t
 *
 * \note This function doesn't check if dev is NULL.
 */
enum syscounter_armv8_m_cntrl_error_t
syscounter_armv8_m_cntrl_set_counter_scale_value(struct syscounter_armv8_m_cntrl_dev_t * dev,
                                                 enum syscounter_armv8_m_cntrl_scale_nr_t nr,
                                                 struct syscounter_armv8_m_cntrl_scale_val_t val);

#ifdef __cplusplus
}
#endif
#endif /* __SYSCOUNTER_ARMV8_M_CNTRL_DRV_H__ */
