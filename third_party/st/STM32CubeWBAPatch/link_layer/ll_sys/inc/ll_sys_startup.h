/**
  ******************************************************************************
  * @file    ll_sys_startup.h
  * @author  MCD Application Team
  * @brief   Header for Link Layer startup interfaces
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2022 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */

#ifndef LL_SYS_STARTUP_H
#define LL_SYS_STARTUP_H

/* Link Layer system interface startup module functions  ************************************************/
#if BLE
void ll_sys_ble_cntrl_init(hst_cbk hostCallback);
#endif
void ll_sys_mac_cntrl_init(void);
void ll_sys_thread_init(void);

#endif /* LL_SYS_STARTUP_H */