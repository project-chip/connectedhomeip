/**
  ******************************************************************************
  * @file    ll_sys_startup.c
  * @author  MCD Application Team
  * @brief   Link Layer IP system interface startup module
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

#include "ll_fw_config.h"
#include "ll_sys.h"
#include "ll_intf.h"
#include "ll_sys_startup.h"
#include "common_types.h"
#if defined(MAC) && (SUPPORT_OPENTHREAD_1_2 == 0)
/* Projects with MAC Layer (i.e. 15.4 except Thread) */
#include "st_mac_802_15_4_sap.h"
#endif /* MAC */

/**
  * @brief  Missed HCI event flag
  */
uint8_t missed_hci_event_flag = 0;

static void ll_sys_dependencies_init(void);
static void ll_sys_event_missed_cb( ble_buff_hdr_t* ptr_evnt_hdr )
{
  missed_hci_event_flag = 1;
}

/**
  * @brief  Initialize the Link Layer IP BLE controller
  * @param  None
  * @retval None
  */
void ll_sys_ble_cntrl_init(hst_cbk hostCallback)
{
  const struct hci_dispatch_tbl* p_hci_dis_tbl = NULL;

  hci_get_dis_tbl( &p_hci_dis_tbl );

  ll_intf_init(p_hci_dis_tbl);

  ll_intf_rgstr_hst_cbk(hostCallback);

  ll_intf_rgstr_hst_cbk_ll_queue_full( ll_sys_event_missed_cb );

  ll_sys_dependencies_init();
}
#if defined(MAC) && (SUPPORT_OPENTHREAD_1_2 == 0)
/**
  * @brief  Initialize the Link Layer IP 802.15.4 MAC controller
  * @param  None
  * @retval None
  */
void ll_sys_mac_cntrl_init(void)
{
  ST_MAC_preInit();
  ll_sys_dependencies_init();
}
#endif /* MAC */

/**
  * @brief  Start the Link Layer IP in OpenThread configuration
  * @param  None
  * @retval None
  */
void ll_sys_thread_init(void)
{
  ll_sys_dependencies_init();
}

/**
  * @brief  Initialize the Link Layer resources for startup.
  *         This includes: - Deep Sleep feature resources
  *                        - Link Layer background task
  * @param  None
  * @retval None
  */
static void ll_sys_dependencies_init(void)
{
  static uint8_t is_ll_initialized = 0;
  ll_sys_status_t dp_slp_status;

  /* Ensure Link Layer resources are created only once */
  if (is_ll_initialized == 1) {
    return;
  }
  is_ll_initialized = 1;

  /* Deep sleep feature initialization */
  dp_slp_status = ll_sys_dp_slp_init();
  ll_sys_assert(dp_slp_status == LL_SYS_OK);

  /* Background task initialization */
  ll_sys_bg_process_init();

  /* Link Layer user parameters application */
  ll_sys_config_params();
}
