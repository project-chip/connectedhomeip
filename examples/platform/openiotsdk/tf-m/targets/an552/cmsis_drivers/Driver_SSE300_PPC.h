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

#ifndef __PPC_SSE300_DRIVER_H__
#define __PPC_SSE300_DRIVER_H__

#include "Driver_Common.h"

#ifdef __cplusplus
extern "C" {
#endif

/* API version */
#define ARM_PPC_API_VERSION ARM_DRIVER_VERSION_MAJOR_MINOR(1, 0)

/* Security attribute used to configure the peripheral */
typedef enum _PPC_SSE300_SecAttr
{
    PPC_SSE300_SECURE_CONFIG = 0, /*!< Secure access */
    PPC_SSE300_NONSECURE_CONFIG,  /*!< Non-secure access */
} PPC_SSE300_SecAttr;

/* Privilege attribute used to configure the peripheral */
typedef enum _PPC_SSE300_PrivAttr
{
    PPC_SSE300_PRIV_AND_NONPRIV_CONFIG = 0, /*!< Privilege and non-privilege
                                             * access */
    PPC_SSE300_PRIV_CONFIG,                 /*!< Privilege only access */
} PPC_SSE300_PrivAttr;

/* Function descriptions */
/**
  SACFG  - Secure Privilege Control Block
  NSACFG - Non-Secure Privilege Control Block

  \fn          ARM_DRIVER_VERSION PPC_SSE300_GetVersion(void)
  \brief       Get driver version.
  \return      \ref ARM_DRIVER_VERSION

  \fn          int32_t PPC_SSE300_Initialize(void)
  \brief       Initializes PPC Interface.
  \return      Returns SSE-300 PPC error code.

  \fn          int32_t PPC_SSE300_Uninitialize(void)
  \brief       De-initializes PPC Interface.
  \return      Returns SSE-300 PPC error code.

  \fn          int32_t PPC_SSE300_ConfigPrivilege(uint32_t periph,
                                                  PPC_SSE300_SecAttr sec_attr,
                                                  PPC_SSE300_PrivAttr priv_attr)
  \brief       Configures privilege level with privileged and unprivileged
               access or privileged access only in the given security domain
               for a peripheral controlled by the given PPC.
  \param[in]   periph:     Peripheral mask for SACFG and NSACFG registers.
  \param[in]   sec_attr:   Specifies Secure or Non Secure domain.
  \param[in]   priv_attr:  Privilege attribute value to set.
  \return      Returns SSE-300 PPC error code.

  \fn          bool PPC_SSE300_IsPeriphPrivOnly (uint32_t periph)
  \brief       Checks if the peripheral is configured to be privilege only
                - with non-secure caller in the non-secure domain
                - with secure caller in the configured security domain
  \param[in]   periph:      Peripheral mask for SACFG and NSACFG registers.
  \return      Returns true if the peripheral is configured as privilege access
               only, false for privilege and unprivilege access mode.

  Secure only functions:

  \fn          int32_t PPC_SSE300_ConfigSecurity(uint32_t periph,
                                                 PPC_SSE300_SecAttr sec_attr)
  \brief       Configures security level for a peripheral controlled by the
               given PPC with secure or non-secure access only.
  \param[in]   periph:     Peripheral mask for SACFG and NSACFG registers.
  \param[in]   sec_attr:   Secure attribute value to set.
  \return      Returns SSE-300 PPC error code.

  \fn          bool PPC_SSE300_IsPeriphSecure (uint32_t periph)
  \brief       Checks if the peripheral is configured to be secure.
  \param[in]   periph:      Peripheral mask for SACFG  and NSACFG registers.
  \return      Returns true if the peripheral is configured as secure,
               false for non-secure.

  \fn          int32_t PPC_SSE300_EnableInterrupt (void)
  \brief       Enables PPC interrupt.
  \return      Returns SSE-300 PPC error code.

  \fn          void PPC_SSE300_DisableInterrupt (void)
  \brief       Disables PPC interrupt.

  \fn          void PPC_SSE300_ClearInterrupt (void)
  \brief       Clears PPC interrupt.

  \fn          bool PPC_SSE300_InterruptState (void)
  \brief       Gets PPC interrupt state.
  \return      Returns true if the interrupt is active, false otherwise.
*/

/**
 * \brief Access structure of the PPC Driver.
 */
typedef struct _DRIVER_PPC_SSE300
{
    ARM_DRIVER_VERSION (*GetVersion)(void); ///< Pointer to \ref ARM_PPC_GetVersion   : Get driver version.
    int32_t (*Initialize)(void);            ///< Pointer to \ref ARM_PPC_Initialize   : Initialize the PPC Interface.
    int32_t (*Uninitialize)(void);          ///< Pointer to \ref ARM_PPC_Uninitialize : De-initialize the PPC Interface.
    int32_t (*ConfigPrivilege)(
        uint32_t periph, PPC_SSE300_SecAttr sec_attr,
        PPC_SSE300_PrivAttr priv_attr); ///< Pointer to \ref ARM_PPC_ConfigPeriph : Configure a peripheral controlled by the PPC.
    bool (*IsPeriphPrivOnly)(
        uint32_t periph); ///< Pointer to \ref IsPeriphPrivOnly : Check if the peripheral is configured to be privilege only.
#if (defined(__ARM_FEATURE_CMSE) && (__ARM_FEATURE_CMSE == 3U))
    int32_t (*ConfigSecurity)(
        uint32_t periph,
        PPC_SSE300_SecAttr sec_attr); ///< Pointer to \ref ARM_PPC_ConfigPeriph : Configure a peripheral controlled by the PPC.
    bool (*IsPeriphSecure)(
        uint32_t periph);             ///< Pointer to \ref IsPeriphSecure :   Check if the peripheral is configured to be secure.
    int32_t (*EnableInterrupt)(void); ///< Pointer to \ref ARM_PPC_EnableInterrupt  : Enable PPC interrupt.
    void (*DisableInterrupt)(void);   ///< Pointer to \ref ARM_PPC_DisableInterrupt : Disable PPC interrupt.
    void (*ClearInterrupt)(void);     ///< Pointer to \ref ARM_PPC_ClearInterrupt   : Clear PPC interrupt.
    bool (*InterruptState)(void);     ///< Pointer to \ref ARM_PPC_InterruptState   : PPC interrupt State.
#endif                                /* (defined (__ARM_FEATURE_CMSE) && (__ARM_FEATURE_CMSE == 3U)) */
} const DRIVER_PPC_SSE300;

#ifdef __cplusplus
}
#endif
#endif /* __PPC_SSE300_DRIVER_H__ */
