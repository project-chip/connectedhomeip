/***************************************************************************//**
 * @file
 * @brief NVM3 definition of the default data structures.
 *******************************************************************************
 * # License
 * <b>Copyright 2018 Silicon Laboratories Inc. www.silabs.com</b>
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

#include "nvm3.h"
#include "nvm3_hal_flash.h"
#include "nvm3_default_config.h"

static nvm3_Handle_t  nvm3_defaultHandleData;

#if (NVM3_DEFAULT_CACHE_SIZE != 0)
static nvm3_CacheEntry_t defaultCache[NVM3_DEFAULT_CACHE_SIZE];
#endif

static nvm3_Init_t nvm3_defaultInitData =
{
  (nvm3_HalPtr_t)0,
  0,
#if (NVM3_DEFAULT_CACHE_SIZE != 0)
  defaultCache,
#else
  NULL,
#endif
  NVM3_DEFAULT_CACHE_SIZE,
  NVM3_DEFAULT_MAX_OBJECT_SIZE,
  NVM3_DEFAULT_REPACK_HEADROOM,
  &nvm3_halFlashHandle,
};

//------------------------------------------------------------------------------
// Public
//------------------------------------------------------------------------------

nvm3_Handle_t *nvm3_defaultHandle = &nvm3_defaultHandleData;

nvm3_Init_t *nvm3_defaultInit = &nvm3_defaultInitData;

Ecode_t nvm3_initDefault(void)
{
  return nvm3_open(nvm3_defaultHandle, nvm3_defaultInit);
}

Ecode_t nvm3_deinitDefault(void)
{
  return nvm3_close(nvm3_defaultHandle);
}

void setNvm3End(uint32_t end_addr)
{
  uint32_t size = NVM3_DEFAULT_NVM_SIZE;
  uint32_t nvm3_start_addr = (end_addr - size);
  nvm3_defaultInit->nvmAdr = (nvm3_HalPtr_t)nvm3_start_addr;
  nvm3_defaultInit->nvmSize = size;
  nvm3_initDefault();
}
