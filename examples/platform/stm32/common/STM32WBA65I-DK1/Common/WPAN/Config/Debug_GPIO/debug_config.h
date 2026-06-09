/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    debug_config.h
  * @author  MCD Application Team
  * @brief   Real Time Debug module general configuration file
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2025 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */

/* USER CODE END Header */
#ifndef DEBUG_CONFIG_H
#define DEBUG_CONFIG_H

#ifdef __cplusplus
extern "C" {
#endif

#include "app_conf.h"

#if(CFG_RT_DEBUG_GPIO_MODULE == 1)

/***********************************/
/** Debug configuration selection **/
/***********************************/
/* Debug configuration for System purpose */
#define USE_RT_DEBUG_CONFIGURATION_SYSTEM                     (0)

/* Debug configuration for BLE purpose */
#define USE_RT_DEBUG_CONFIGURATION_BLE                        (0)

/* Debug configuration for MAC purpose */
#define USE_RT_DEBUG_CONFIGURATION_MAC                        (0)

/* Debug configuration for COEX purpose */
#define USE_RT_DEBUG_CONFIGURATION_COEX                       (0)

/*********************************/
/** GPIO debug signal selection **/
/*********************************/

/* System clock manager - System clock config */
#define USE_RT_DEBUG_SCM_SYSTEM_CLOCK_CONFIG                  (0)
#define GPIO_DEBUG_SCM_SYSTEM_CLOCK_CONFIG                    {GPIOA, LL_PWR_GPIO_PIN_12}

/* System clock manager - Setup */
#define USE_RT_DEBUG_SCM_SETUP                                (0)
#define GPIO_DEBUG_SCM_SETUP                                  {GPIOA, LL_PWR_GPIO_PIN_5}

/* System clock manager - HSE RDY interrupt handling */
#define USE_RT_DEBUG_SCM_HSERDY_ISR                           (0)
#define GPIO_DEBUG_SCM_HSERDY_ISR                             {GPIOA, LL_PWR_GPIO_PIN_15}

#define USE_RT_DEBUG_ADC_ACTIVATION                           (0)
#define GPIO_DEBUG_ADC_ACTIVATION                             {GPIOB, LL_PWR_GPIO_PIN_4}

#define USE_RT_DEBUG_ADC_DEACTIVATION                         (0)
#define GPIO_DEBUG_ADC_DEACTIVATION                           {GPIOA, LL_PWR_GPIO_PIN_0}

#define USE_RT_DEBUG_ADC_TEMPERATURE_ACQUISITION              (0)
#define GPIO_DEBUG_ADC_TEMPERATURE_ACQUISITION                {GPIOB, LL_PWR_GPIO_PIN_8}

#define USE_RT_DEBUG_RNG_ENABLE                               (0)
#define GPIO_DEBUG_RNG_ENABLE                                 {GPIOA, LL_PWR_GPIO_PIN_0}

#define USE_RT_DEBUG_RNG_DISABLE                              (0)
#define GPIO_DEBUG_RNG_DISABLE                                {GPIOA, LL_PWR_GPIO_PIN_0}

#define USE_RT_DEBUG_RNG_GEN_RAND_NUM                         (0)
#define GPIO_DEBUG_RNG_GEN_RAND_NUM                           {GPIOB, LL_PWR_GPIO_PIN_12}

#define USE_RT_DEBUG_LOW_POWER_STOP_MODE_ENTER                (0)
#define GPIO_DEBUG_LOW_POWER_STOP_MODE_ENTER                  {GPIOA, LL_PWR_GPIO_PIN_0}

#define USE_RT_DEBUG_LOW_POWER_STOP_MODE_EXIT                 (0)
#define GPIO_DEBUG_LOW_POWER_STOP_MODE_EXIT                   {GPIOA, LL_PWR_GPIO_PIN_0}

#define USE_RT_DEBUG_LOW_POWER_STOP_MODE_ACTIVE               (0)
#define GPIO_DEBUG_LOW_POWER_STOP_MODE_ACTIVE                 {GPIOB, LL_PWR_GPIO_PIN_3}

#define USE_RT_DEBUG_LOW_POWER_STOP2_MODE_ENTER               (0)
#define GPIO_DEBUG_LOW_POWER_STOP2_MODE_ENTER                 {GPIOA, LL_PWR_GPIO_PIN_0}

#define USE_RT_DEBUG_LOW_POWER_STOP2_MODE_EXIT                (0)
#define GPIO_DEBUG_LOW_POWER_STOP2_MODE_EXIT                  {GPIOA, LL_PWR_GPIO_PIN_0}

#define USE_RT_DEBUG_LOW_POWER_STOP2_MODE_ACTIVE              (0)
#define GPIO_DEBUG_LOW_POWER_STOP2_MODE_ACTIVE                {GPIOA, LL_PWR_GPIO_PIN_0}

#define USE_RT_DEBUG_LOW_POWER_STANDBY_MODE_ENTER             (0)
#define GPIO_DEBUG_LOW_POWER_STANDBY_MODE_ENTER               {GPIOA, LL_PWR_GPIO_PIN_0}

#define USE_RT_DEBUG_LOW_POWER_STANDBY_MODE_EXIT              (0)
#define GPIO_DEBUG_LOW_POWER_STANDBY_MODE_EXIT                {GPIOA, LL_PWR_GPIO_PIN_0}

#define USE_RT_DEBUG_LOW_POWER_STANDBY_MODE_ACTIVE            (0)
#define GPIO_DEBUG_LOW_POWER_STANDBY_MODE_ACTIVE              {GPIOB, LL_PWR_GPIO_PIN_15}

#define USE_RT_DEBUG_HCI_READ_DONE                            (0)
#define GPIO_DEBUG_HCI_READ_DONE                              {GPIOA, LL_PWR_GPIO_PIN_0}

#define USE_RT_DEBUG_HCI_RCVD_CMD                             (0)
#define GPIO_DEBUG_HCI_RCVD_CMD                               {GPIOA, LL_PWR_GPIO_PIN_0}

#define USE_RT_DEBUG_HCI_WRITE_DONE                           (0)
#define GPIO_DEBUG_HCI_WRITE_DONE                             {GPIOA, LL_PWR_GPIO_PIN_0}

#define USE_RT_DEBUG_SCHDLR_EVNT_UPDATE                       (0)
#define GPIO_DEBUG_SCHDLR_EVNT_UPDATE                         {GPIOA, LL_PWR_GPIO_PIN_0}

#define USE_RT_DEBUG_SCHDLR_TIMER_SET                         (0)
#define GPIO_DEBUG_SCHDLR_TIMER_SET                           {GPIOA, LL_PWR_GPIO_PIN_0}

#define USE_RT_DEBUG_SCHDLR_PHY_CLBR_TIMER                    (0)
#define GPIO_DEBUG_SCHDLR_PHY_CLBR_TIMER                      {GPIOA, LL_PWR_GPIO_PIN_0}

#define USE_RT_DEBUG_SCHDLR_EVNT_SKIPPED                      (0)
#define GPIO_DEBUG_SCHDLR_EVNT_SKIPPED                        {GPIOA, LL_PWR_GPIO_PIN_0}

#define USE_RT_DEBUG_SCHDLR_HNDL_NXT_TRACE                    (0)
#define GPIO_DEBUG_SCHDLR_HNDL_NXT_TRACE                      {GPIOA, LL_PWR_GPIO_PIN_12}

#define USE_RT_DEBUG_ACTIVE_SCHDLR_NEAR_DETEDTED              (0)
#define GPIO_DEBUG_ACTIVE_SCHDLR_NEAR_DETEDTED                {GPIOA, LL_PWR_GPIO_PIN_0}

#define USE_RT_DEBUG_ACTIVE_SCHDLR_NEAR_GAP_CHECK             (0)
#define GPIO_DEBUG_ACTIVE_SCHDLR_NEAR_GAP_CHECK               {GPIOA, LL_PWR_GPIO_PIN_0}

#define USE_RT_DEBUG_ACTIVE_SCHDLR_NEAR_TIME_CHECK            (0)
#define GPIO_DEBUG_ACTIVE_SCHDLR_NEAR_TIME_CHECK              {GPIOA, LL_PWR_GPIO_PIN_0}

#define USE_RT_DEBUG_ACTIVE_SCHDLR_NEAR_TRACE                 (0)
#define GPIO_DEBUG_ACTIVE_SCHDLR_NEAR_TRACE                   {GPIOA, LL_PWR_GPIO_PIN_0}

#define USE_RT_DEBUG_SCHDLR_EVNT_RGSTR                        (0)
#define GPIO_DEBUG_SCHDLR_EVNT_RGSTR                          {GPIOB, LL_PWR_GPIO_PIN_8}

#define USE_RT_DEBUG_SCHDLR_ADD_CONFLICT_Q                    (0)
#define GPIO_DEBUG_SCHDLR_ADD_CONFLICT_Q                      {GPIOA, LL_PWR_GPIO_PIN_0}

#define USE_RT_DEBUG_SCHDLR_HNDL_MISSED_EVNT                  (0)
#define GPIO_DEBUG_SCHDLR_HNDL_MISSED_EVNT                    {GPIOA, LL_PWR_GPIO_PIN_5}

#define USE_RT_DEBUG_SCHDLR_UNRGSTR_EVNT                      (0)
#define GPIO_DEBUG_SCHDLR_UNRGSTR_EVNT                        {GPIOA, LL_PWR_GPIO_PIN_0}

#define USE_RT_DEBUG_SCHDLR_EXEC_EVNT_TRACE                   (0)
#define GPIO_DEBUG_SCHDLR_EXEC_EVNT_TRACE                     {GPIOA, LL_PWR_GPIO_PIN_15}

#define USE_RT_DEBUG_SCHDLR_EXEC_EVNT_PROFILE                 (0)
#define GPIO_DEBUG_SCHDLR_EXEC_EVNT_PROFILE                   {GPIOA, LL_PWR_GPIO_PIN_0}

#define USE_RT_DEBUG_SCHDLR_EXEC_EVNT_ERROR                   (0)
#define GPIO_DEBUG_SCHDLR_EXEC_EVNT_ERROR                     {GPIOA, LL_PWR_GPIO_PIN_0}

#define USE_RT_DEBUG_SCHDLR_EXEC_EVNT_WINDOW_WIDENING         (0)
#define GPIO_DEBUG_SCHDLR_EXEC_EVNT_WINDOW_WIDENING           {GPIOA, LL_PWR_GPIO_PIN_0}

#define USE_RT_DEBUG_LLHWC_CMN_CLR_ISR                        (0)
#define GPIO_DEBUG_LLHWC_CMN_CLR_ISR                          {GPIOA, LL_PWR_GPIO_PIN_0}

#define USE_RT_DEBUG_LLWCC_CMN_HG_ISR                         (0)
#define GPIO_DEBUG_LLWCC_CMN_HG_ISR                           {GPIOA, LL_PWR_GPIO_PIN_15}

#define USE_RT_DEBUG_LLHWC_CMN_LW_ISR                         (0)
#define GPIO_DEBUG_LLHWC_CMN_LW_ISR                           {GPIOA, LL_PWR_GPIO_PIN_12}

#define USE_RT_DEBUG_LLHWC_CMN_CLR_TIMER_ERROR                (0)
#define GPIO_DEBUG_LLHWC_CMN_CLR_TIMER_ERROR                  {GPIOA, LL_PWR_GPIO_PIN_0}

#define USE_RT_DEBUG_LLHWC_LL_ISR                             (0)
#define GPIO_DEBUG_LLHWC_LL_ISR                               {GPIOA, LL_PWR_GPIO_PIN_0}

#define USE_RT_DEBUG_LLHWC_SPLTMR_SET                         (0)
#define GPIO_DEBUG_LLHWC_SPLTMR_SET                           {GPIOA, LL_PWR_GPIO_PIN_0}

#define USE_RT_DEBUG_LLHWC_SPLTMR_GET                         (0)
#define GPIO_DEBUG_LLHWC_SPLTMR_GET                           {GPIOA, LL_PWR_GPIO_PIN_0}

#define USE_RT_DEBUG_LLHWC_LOW_ISR                            (0)
#define GPIO_DEBUG_LLHWC_LOW_ISR                              {GPIOA, LL_PWR_GPIO_PIN_0}

#define USE_RT_DEBUG_LLHWC_STOP_SCN                           (0)
#define GPIO_DEBUG_LLHWC_STOP_SCN                             {GPIOA, LL_PWR_GPIO_PIN_0}

#define USE_RT_DEBUG_LLHWC_WAIT_ENVT_ON_AIR                   (0)
#define GPIO_DEBUG_LLHWC_WAIT_ENVT_ON_AIR                     {GPIOA, LL_PWR_GPIO_PIN_0}

#define USE_RT_DEBUG_LLHWC_SET_CONN_EVNT_PARAM                (0)
#define GPIO_DEBUG_LLHWC_SET_CONN_EVNT_PARAM                  {GPIOA, LL_PWR_GPIO_PIN_0}

#define USE_RT_DEBUG_POST_EVNT                                (0)
#define GPIO_DEBUG_POST_EVNT                                  {GPIOA, LL_PWR_GPIO_PIN_0}

#define USE_RT_DEBUG_HNDL_ALL_EVNTS                           (0)
#define GPIO_DEBUG_HNDL_ALL_EVNTS                             {GPIOA, LL_PWR_GPIO_PIN_0}

#define USE_RT_DEBUG_PROCESS_EVNT                             (0)
#define GPIO_DEBUG_PROCESS_EVNT                               {GPIOA, LL_PWR_GPIO_PIN_0}

#define USE_RT_DEBUG_PROCESS_ISO_DATA                         (0)
#define GPIO_DEBUG_PROCESS_ISO_DATA                           {GPIOA, LL_PWR_GPIO_PIN_0}

#define USE_RT_DEBUG_ALLOC_TX_ISO_EMPTY_PKT                   (0)
#define GPIO_DEBUG_ALLOC_TX_ISO_EMPTY_PKT                     {GPIOA, LL_PWR_GPIO_PIN_0}

#define USE_RT_DEBUG_BIG_FREE_EMPTY_PKTS                      (0)
#define GPIO_DEBUG_BIG_FREE_EMPTY_PKTS                        {GPIOA, LL_PWR_GPIO_PIN_0}

#define USE_RT_DEBUG_RECOMBINE_UNFRMD_DATA_OK                 (0)
#define GPIO_DEBUG_RECOMBINE_UNFRMD_DATA_OK                   {GPIOA, LL_PWR_GPIO_PIN_0}

#define USE_RT_DEBUG_RECOMBINE_UNFRMD_DATA_CRC                (0)
#define GPIO_DEBUG_RECOMBINE_UNFRMD_DATA_CRC                  {GPIOA, LL_PWR_GPIO_PIN_0}

#define USE_RT_DEBUG_RECOMBINE_UNFRMD_DATA_NoRX               (0)
#define GPIO_DEBUG_RECOMBINE_UNFRMD_DATA_NoRX                 {GPIOA, LL_PWR_GPIO_PIN_0}

#define USE_RT_DEBUG_RECOMBINE_UNFRMD_DATA_TRACE              (0)
#define GPIO_DEBUG_RECOMBINE_UNFRMD_DATA_TRACE                {GPIOA, LL_PWR_GPIO_PIN_0}

#define USE_RT_DEBUG_ISO_HNDL_SDU                             (0)
#define GPIO_DEBUG_ISO_HNDL_SDU                               {GPIOA, LL_PWR_GPIO_PIN_0}

#define USE_RT_DEBUG_LL_INTF_INIT                             (0)
#define GPIO_DEBUG_LL_INTF_INIT                               {GPIOA, LL_PWR_GPIO_PIN_0}

#define USE_RT_DEBUG_DATA_TO_CNTRLR                           (0)
#define GPIO_DEBUG_DATA_TO_CNTRLR                             {GPIOA, LL_PWR_GPIO_PIN_0}

#define USE_RT_DEBUG_FREE_LL_PKT_HNDLR                        (0)
#define GPIO_DEBUG_FREE_LL_PKT_HNDLR                          {GPIOA, LL_PWR_GPIO_PIN_0}

#define USE_RT_DEBUG_PHY_INIT_CLBR_TRACE                      (0)
#define GPIO_DEBUG_PHY_INIT_CLBR_TRACE                        {GPIOA, LL_PWR_GPIO_PIN_0}

#define USE_RT_DEBUG_PHY_RUNTIME_CLBR_TRACE                   (0)
#define GPIO_DEBUG_PHY_RUNTIME_CLBR_TRACE                     {GPIOA, LL_PWR_GPIO_PIN_0}

#define USE_RT_DEBUG_PHY_CLBR_ISR                             (0)
#define GPIO_DEBUG_PHY_CLBR_ISR                               {GPIOB, LL_PWR_GPIO_PIN_3}

#define USE_RT_DEBUG_PHY_INIT_CLBR_SINGLE_CH                  (0)
#define GPIO_DEBUG_PHY_INIT_CLBR_SINGLE_CH                    {GPIOA, LL_PWR_GPIO_PIN_0}

#define USE_RT_DEBUG_PHY_CLBR_STRTD                           (0)
#define GPIO_DEBUG_PHY_CLBR_STRTD                             {GPIOA, LL_PWR_GPIO_PIN_0}

#define USE_RT_DEBUG_PHY_CLBR_EXEC                            (0)
#define GPIO_DEBUG_PHY_CLBR_EXEC                              {GPIOB, LL_PWR_GPIO_PIN_4}

#define USE_RT_DEBUG_RCO_STRT_STOP_RUNTIME_CLBR_ACTV          (0)
#define GPIO_DEBUG_RCO_STRT_STOP_RUNTIME_CLBR_ACTV            {GPIOA, LL_PWR_GPIO_PIN_0}

#define USE_RT_DEBUG_RCO_STRT_STOP_RUNTIME_RCO_CLBR           (0)
#define GPIO_DEBUG_RCO_STRT_STOP_RUNTIME_RCO_CLBR             {GPIOA, LL_PWR_GPIO_PIN_0}

#define USE_RT_DEBUG_STRT_STOP_RUNTIME_RCO_CLBR_SWT           (0)
#define GPIO_DEBUG_STRT_STOP_RUNTIME_RCO_CLBR_SWT             {GPIOA, LL_PWR_GPIO_PIN_0}

#define USE_RT_DEBUG_STRT_STOP_RUNTIME_RCO_CLBR_TRACE         (0)
#define GPIO_DEBUG_STRT_STOP_RUNTIME_RCO_CLBR_TRACE           {GPIOA, LL_PWR_GPIO_PIN_0}

#define USE_RT_DEBUG_RCO_ISR_TRACE                            (0)
#define GPIO_DEBUG_RCO_ISR_TRACE                              {GPIOA, LL_PWR_GPIO_PIN_0}

#define USE_RT_DEBUG_RCO_ISR_COMPENDATE                       (0)
#define GPIO_DEBUG_RCO_ISR_COMPENDATE                         {GPIOA, LL_PWR_GPIO_PIN_0}

#define USE_RT_DEBUG_RAL_STRT_TX                              (0)
#define GPIO_DEBUG_RAL_STRT_TX                                {GPIOA, LL_PWR_GPIO_PIN_5}

#define USE_RT_DEBUG_RAL_ISR_TIMER_ERROR                      (0)
#define GPIO_DEBUG_RAL_ISR_TIMER_ERROR                        {GPIOA, LL_PWR_GPIO_PIN_0}

#define USE_RT_DEBUG_RAL_ISR_TRACE                            (0)
#define GPIO_DEBUG_RAL_ISR_TRACE                              {GPIOB, LL_PWR_GPIO_PIN_3}

#define USE_RT_DEBUG_RAL_STOP_OPRTN                           (0)
#define GPIO_DEBUG_RAL_STOP_OPRTN                             {GPIOB, LL_PWR_GPIO_PIN_8}

#define USE_RT_DEBUG_RAL_STRT_RX                              (0)
#define GPIO_DEBUG_RAL_STRT_RX                                {GPIOB, LL_PWR_GPIO_PIN_12}

#define USE_RT_DEBUG_RAL_DONE_CLBK_TX                         (0)
#define GPIO_DEBUG_RAL_DONE_CLBK_TX                           {GPIOA, LL_PWR_GPIO_PIN_0}

#define USE_RT_DEBUG_RAL_DONE_CLBK_RX                         (0)
#define GPIO_DEBUG_RAL_DONE_CLBK_RX                           {GPIOA, LL_PWR_GPIO_PIN_0}

#define USE_RT_DEBUG_RAL_DONE_CLBK_ED                         (0)
#define GPIO_DEBUG_RAL_DONE_CLBK_ED                           {GPIOA, LL_PWR_GPIO_PIN_0}

#define USE_RT_DEBUG_RAL_ED_SCAN                              (0)
#define GPIO_DEBUG_RAL_ED_SCAN                                {GPIOA, LL_PWR_GPIO_PIN_0}

#define USE_RT_DEBUG_ERROR_MEM_CAP_EXCED                      (0)
#define GPIO_DEBUG_ERROR_MEM_CAP_EXCED                        {GPIOA, LL_PWR_GPIO_PIN_0}

#define USE_RT_DEBUG_ERROR_COMMAND_DISALLOWED                 (0)
#define GPIO_DEBUG_ERROR_COMMAND_DISALLOWED                   {GPIOA, LL_PWR_GPIO_PIN_0}

#define USE_RT_DEBUG_PTA_INIT                                 (0)
#define GPIO_DEBUG_PTA_INIT                                   {GPIOA, LL_PWR_GPIO_PIN_0}

#define USE_RT_DEBUG_PTA_EN                                   (0)
#define GPIO_DEBUG_PTA_EN                                     {GPIOA, LL_PWR_GPIO_PIN_0}

#define USE_RT_DEBUG_LLHWC_PTA_SET_EN                         (0)
#define GPIO_DEBUG_LLHWC_PTA_SET_EN                           {GPIOA, LL_PWR_GPIO_PIN_0}

#define USE_RT_DEBUG_LLHWC_PTA_SET_PARAMS                     (0)
#define GPIO_DEBUG_LLHWC_PTA_SET_PARAMS                       {GPIOA, LL_PWR_GPIO_PIN_0}

#define USE_RT_DEBUG_COEX_STRT_ON_IDLE                        (0)
#define GPIO_DEBUG_COEX_STRT_ON_IDLE                          {GPIOB, LL_PWR_GPIO_PIN_15}

#define USE_RT_DEBUG_COEX_ASK_FOR_AIR                         (0)
#define GPIO_DEBUG_COEX_ASK_FOR_AIR                           {GPIOB, LL_PWR_GPIO_PIN_3}

#define USE_RT_DEBUG_COEX_TIMER_EVNT_CLBK                     (0)
#define GPIO_DEBUG_COEX_TIMER_EVNT_CLBK                       {GPIOA, LL_PWR_GPIO_PIN_0}

#define USE_RT_DEBUG_COEX_STRT_ONE_SHOT                       (0)
#define GPIO_DEBUG_COEX_STRT_ONE_SHOT                         {GPIOA, LL_PWR_GPIO_PIN_5}

#define USE_RT_DEBUG_COEX_FORCE_STOP_RX                       (0)
#define GPIO_DEBUG_COEX_FORCE_STOP_RX                         {GPIOB, LL_PWR_GPIO_PIN_12}

#define USE_RT_DEBUG_LLHWC_ADV_DONE                           (0)
#define GPIO_DEBUG_LLHWC_ADV_DONE                             {GPIOA, LL_PWR_GPIO_PIN_0}

#define USE_RT_DEBUG_LLHWC_SCN_DONE                           (0)
#define GPIO_DEBUG_LLHWC_SCN_DONE                             {GPIOA, LL_PWR_GPIO_PIN_0}

#define USE_RT_DEBUG_LLHWC_INIT_DONE                          (0)
#define GPIO_DEBUG_LLHWC_INIT_DONE                            {GPIOA, LL_PWR_GPIO_PIN_0}

#define USE_RT_DEBUG_LLHWC_CONN_DONE                          (0)
#define GPIO_DEBUG_LLHWC_CONN_DONE                            {GPIOA, LL_PWR_GPIO_PIN_0}

#define USE_RT_DEBUG_LLHWC_CIG_DONE                           (0)
#define GPIO_DEBUG_LLHWC_CIG_DONE                             {GPIOA, LL_PWR_GPIO_PIN_0}

#define USE_RT_DEBUG_LLHWC_BIG_DONE                           (0)
#define GPIO_DEBUG_LLHWC_BIG_DONE                             {GPIOA, LL_PWR_GPIO_PIN_0}

#define USE_RT_DEBUG_OS_TMR_CREATE                            (0)
#define GPIO_DEBUG_OS_TMR_CREATE                              {GPIOA, LL_PWR_GPIO_PIN_0}

#define USE_RT_DEBUG_ADV_EXT_TIMEOUT_CBK                      (0)
#define GPIO_DEBUG_ADV_EXT_TIMEOUT_CBK                        {GPIOA, LL_PWR_GPIO_PIN_0}

#define USE_RT_DEBUG_ADV_EXT_SCN_DUR_CBK                      (0)
#define GPIO_DEBUG_ADV_EXT_SCN_DUR_CBK                        {GPIOA, LL_PWR_GPIO_PIN_0}

#define USE_RT_DEBUG_ADV_EXT_SCN_PERIOD_CBK                   (0)
#define GPIO_DEBUG_ADV_EXT_SCN_PERIOD_CBK                     {GPIOA, LL_PWR_GPIO_PIN_0}

#define USE_RT_DEBUG_ADV_EXT_PRDC_SCN_TIMEOUT_CBK             (0)
#define GPIO_DEBUG_ADV_EXT_PRDC_SCN_TIMEOUT_CBK               {GPIOA, LL_PWR_GPIO_PIN_0}

#define USE_RT_DEBUG_BIS_SYNC_TIMEOUT_TMR_CBK                 (0)
#define GPIO_DEBUG_BIS_SYNC_TIMEOUT_TMR_CBK                   {GPIOA, LL_PWR_GPIO_PIN_0}

#define USE_RT_DEBUG_BIS_TERM_TMR_CBK                         (0)
#define GPIO_DEBUG_BIS_TERM_TMR_CBK                           {GPIOA, LL_PWR_GPIO_PIN_0}

#define USE_RT_DEBUG_BIS_TST_MODE_CBK                         (0)
#define GPIO_DEBUG_BIS_TST_MODE_CBK                           {GPIOA, LL_PWR_GPIO_PIN_0}

#define USE_RT_DEBUG_BIS_TST_MODE_TMR_CBK                     (0)
#define GPIO_DEBUG_BIS_TST_MODE_TMR_CBK                       {GPIOA, LL_PWR_GPIO_PIN_0}

#define USE_RT_DEBUG_ISO_POST_TMR_CBK                         (0)
#define GPIO_DEBUG_ISO_POST_TMR_CBK                           {GPIOA, LL_PWR_GPIO_PIN_0}

#define USE_RT_DEBUG_ISO_TST_MODE_TMR_CBK                     (0)
#define GPIO_DEBUG_ISO_TST_MODE_TMR_CBK                       {GPIOA, LL_PWR_GPIO_PIN_0}

#define USE_RT_DEBUG_CONN_POST_TMR_CBK                        (0)
#define GPIO_DEBUG_CONN_POST_TMR_CBK                          {GPIOA, LL_PWR_GPIO_PIN_0}

#define USE_RT_DEBUG_EVNT_SCHDLR_TMR_CBK                      (0)
#define GPIO_DEBUG_EVNT_SCHDLR_TMR_CBK                        {GPIOA, LL_PWR_GPIO_PIN_0}

#define USE_RT_DEBUG_HCI_POST_TMR_CBK                         (0)
#define GPIO_DEBUG_HCI_POST_TMR_CBK                           {GPIOA, LL_PWR_GPIO_PIN_0}

#define USE_RT_DEBUG_LLCP_POST_TMR_CBK                        (0)
#define GPIO_DEBUG_LLCP_POST_TMR_CBK                          {GPIOA, LL_PWR_GPIO_PIN_0}

#define USE_RT_DEBUG_LLHWC_ENRGY_DETECT_CBK                   (0)
#define GPIO_DEBUG_LLHWC_ENRGY_DETECT_CBK                     {GPIOA, LL_PWR_GPIO_PIN_0}

#define USE_RT_DEBUG_PRVCY_POST_TMR_CBK                       (0)
#define GPIO_DEBUG_PRVCY_POST_TMR_CBK                         {GPIOA, LL_PWR_GPIO_PIN_0}

#define USE_RT_DEBUG_ANT_PRPR_TMR_CBK                         (0)
#define GPIO_DEBUG_ANT_PRPR_TMR_CBK                           {GPIOA, LL_PWR_GPIO_PIN_0}

#define USE_RT_DEBUG_COEX_TMR_FRC_STOP_AIR_GRANT_CBK          (0)
#define GPIO_DEBUG_COEX_TMR_FRC_STOP_AIR_GRANT_CBK            {GPIOA, LL_PWR_GPIO_PIN_0}

#define USE_RT_DEBUG_MLME_RX_EN_TMR_CBK                       (0)
#define GPIO_DEBUG_MLME_RX_EN_TMR_CBK                         {GPIOA, LL_PWR_GPIO_PIN_0}

#define USE_RT_DEBUG_MLME_GNRC_TMR_CBK                        (0)
#define GPIO_DEBUG_MLME_GNRC_TMR_CBK                          {GPIOA, LL_PWR_GPIO_PIN_0}

#define USE_RT_DEBUG_MIB_JOIN_LST_TMR_CBK                     (0)
#define GPIO_DEBUG_MIB_JOIN_LST_TMR_CBK                       {GPIOA, LL_PWR_GPIO_PIN_0}

#define USE_RT_DEBUG_MLME_PWR_PRES_TMR_CBK                    (0)
#define GPIO_DEBUG_MLME_PWR_PRES_TMR_CBK                      {GPIOA, LL_PWR_GPIO_PIN_0}

#define USE_RT_DEBUG_PRESISTENCE_TMR_CBK                      (0)
#define GPIO_DEBUG_PRESISTENCE_TMR_CBK                        {GPIOA, LL_PWR_GPIO_PIN_0}

#define USE_RT_DEBUG_RADIO_PHY_PRDC_CLBK_TMR_CBK              (0)
#define GPIO_DEBUG_RADIO_PHY_PRDC_CLBK_TMR_CBK                {GPIOA, LL_PWR_GPIO_PIN_0}

#define USE_RT_DEBUG_RADIO_CSMA_TMR_CBK                       (0)
#define GPIO_DEBUG_RADIO_CSMA_TMR_CBK                         {GPIOA, LL_PWR_GPIO_PIN_0}

#define USE_RT_DEBUG_RADIO_CSL_RCV_TMR_CBK                    (0)
#define GPIO_DEBUG_RADIO_CSL_RCV_TMR_CBK                      {GPIOA, LL_PWR_GPIO_PIN_0}

#define USE_RT_DEBUG_ED_TMR_CBK                               (0)
#define GPIO_DEBUG_ED_TMR_CBK                                 {GPIOA, LL_PWR_GPIO_PIN_0}

#define USE_RT_DEBUG_DIO_EXT_TMR_CBK                          (0)
#define GPIO_DEBUG_DIO_EXT_TMR_CBK                            {GPIOA, LL_PWR_GPIO_PIN_0}

#define USE_RT_DEBUG_RCO_CLBR_TMR_CBK                         (0)
#define GPIO_DEBUG_RCO_CLBR_TMR_CBK                           {GPIOA, LL_PWR_GPIO_PIN_0}

#define USE_RT_DEBUG_ADV_EXT_MNGR_ADV_CBK                     (0)
#define GPIO_DEBUG_ADV_EXT_MNGR_ADV_CBK                       {GPIOA, LL_PWR_GPIO_PIN_0}

#define USE_RT_DEBUG_ADV_EXT_MNGR_SCN_CBK                     (0)
#define GPIO_DEBUG_ADV_EXT_MNGR_SCN_CBK                       {GPIOA, LL_PWR_GPIO_PIN_0}

#define USE_RT_DEBUG_ADV_EXT_MNGR_SCN_ERR_CBK                 (0)
#define GPIO_DEBUG_ADV_EXT_MNGR_SCN_ERR_CBK                   {GPIOA, LL_PWR_GPIO_PIN_0}

#define USE_RT_DEBUG_ADV_EXT_MNGR_PRDC_SCN_CBK                (0)
#define GPIO_DEBUG_ADV_EXT_MNGR_PRDC_SCN_CBK                  {GPIOA, LL_PWR_GPIO_PIN_0}

#define USE_RT_DEBUG_ADV_EXT_MNGR_PRDC_SCN_ERR_CBK            (0)
#define GPIO_DEBUG_ADV_EXT_MNGR_PRDC_SCN_ERR_CBK              {GPIOA, LL_PWR_GPIO_PIN_0}

#define USE_RT_DEBUG_BIG_ADV_CBK                              (0)
#define GPIO_DEBUG_BIG_ADV_CBK                                {GPIOA, LL_PWR_GPIO_PIN_0}

#define USE_RT_DEBUG_BIG_ADV_ERR_CBK                          (0)
#define GPIO_DEBUG_BIG_ADV_ERR_CBK                            {GPIOA, LL_PWR_GPIO_PIN_0}

#define USE_RT_DEBUG_BIG_SYNC_CBK                             (0)
#define GPIO_DEBUG_BIG_SYNC_CBK                               {GPIOA, LL_PWR_GPIO_PIN_0}

#define USE_RT_DEBUG_BIG_SYNC_ERR_CBK                         (0)
#define GPIO_DEBUG_BIG_SYNC_ERR_CBK                           {GPIOA, LL_PWR_GPIO_PIN_0}

#define USE_RT_DEBUG_ISO_CIS_PKT_TRNSM_RECEIVED_CBK           (0)
#define GPIO_DEBUG_ISO_CIS_PKT_TRNSM_RECEIVED_CBK             {GPIOA, LL_PWR_GPIO_PIN_0}

#define USE_RT_DEBUG_ISO_CIG_ERR_CBK                          (0)
#define GPIO_DEBUG_ISO_CIG_ERR_CBK                            {GPIOA, LL_PWR_GPIO_PIN_0}

#define USE_RT_DEBUG_CONN_PKT_TRNSM_RECEIVED_CBK              (0)
#define GPIO_DEBUG_CONN_PKT_TRNSM_RECEIVED_CBK                {GPIOA, LL_PWR_GPIO_PIN_0}

#define USE_RT_DEBUG_PRDC_CLBR_EXTRL_CBK                      (0)
#define GPIO_DEBUG_PRDC_CLBR_EXTRL_CBK                        {GPIOA, LL_PWR_GPIO_PIN_0}

#define USE_RT_DEBUG_PTR_PRDC_ADV_SYNC_CBK                    (0)
#define GPIO_DEBUG_PTR_PRDC_ADV_SYNC_CBK                      {GPIOA, LL_PWR_GPIO_PIN_0}

#define USE_RT_DEBUG_NCONN_SCN_CBK                            (0)
#define GPIO_DEBUG_NCONN_SCN_CBK                              {GPIOA, LL_PWR_GPIO_PIN_0}

#define USE_RT_DEBUG_NCONN_ADV_CBK                            (0)
#define GPIO_DEBUG_NCONN_ADV_CBK                              {GPIOA, LL_PWR_GPIO_PIN_0}

#define USE_RT_DEBUG_NCONN_INIT_CBK                           (0)
#define GPIO_DEBUG_NCONN_INIT_CBK                             {GPIOA, LL_PWR_GPIO_PIN_0}

#define USE_RT_DEBUG_ANT_RADIO_CMPLT_EVNT_CBK                 (0)
#define GPIO_DEBUG_ANT_RADIO_CMPLT_EVNT_CBK                   {GPIOA, LL_PWR_GPIO_PIN_0}

#define USE_RT_DEBUG_ANT_STACK_EVNT_CBK                       (0)
#define GPIO_DEBUG_ANT_STACK_EVNT_CBK                         {GPIOA, LL_PWR_GPIO_PIN_0}

#define USE_RT_DEBUG_ADV_EXT_PROCESS_TMOUT_EVNT_CBK           (0)
#define GPIO_DEBUG_ADV_EXT_PROCESS_TMOUT_EVNT_CBK             {GPIOA, LL_PWR_GPIO_PIN_0}

#define USE_RT_DEBUG_ADV_EXT_MNGR_SCN_DUR_EVNT                (0)
#define GPIO_DEBUG_ADV_EXT_MNGR_SCN_DUR_EVNT                  {GPIOA, LL_PWR_GPIO_PIN_0}

#define USE_RT_DEBUG_ADV_EXT_MNGR_SCN_PERIODIC_EVNT           (0)
#define GPIO_DEBUG_ADV_EXT_MNGR_SCN_PERIODIC_EVNT             {GPIOA, LL_PWR_GPIO_PIN_0}

#define USE_RT_DEBUG_ADV_EXT_MNGR_PRDC_SCN_TMOUT_EVNT         (0)
#define GPIO_DEBUG_ADV_EXT_MNGR_PRDC_SCN_TMOUT_EVNT           {GPIOA, LL_PWR_GPIO_PIN_0}

#define USE_RT_DEBUG_ADV_EXT_MNGR_PRDC_SCN_CNCEL_EVNT         (0)
#define GPIO_DEBUG_ADV_EXT_MNGR_PRDC_SCN_CNCEL_EVNT           {GPIOA, LL_PWR_GPIO_PIN_0}

#define USE_RT_DEBUG_BIS_MNGR_BIG_TERM_CBK                    (0)
#define GPIO_DEBUG_BIS_MNGR_BIG_TERM_CBK                      {GPIOA, LL_PWR_GPIO_PIN_0}

#define USE_RT_DEBUG_BIS_MNGR_SYNC_TMOUT_CBK                  (0)
#define GPIO_DEBUG_BIS_MNGR_SYNC_TMOUT_CBK                    {GPIOA, LL_PWR_GPIO_PIN_0}

#define USE_RT_DEBUG_ISOAL_MNGR_SDU_GEN                       (0)
#define GPIO_DEBUG_ISOAL_MNGR_SDU_GEN                         {GPIOA, LL_PWR_GPIO_PIN_0}

#define USE_RT_DEBUG_ISO_MNGR_CIS_PROCESS_EVNT_CBK            (0)
#define GPIO_DEBUG_ISO_MNGR_CIS_PROCESS_EVNT_CBK              {GPIOA, LL_PWR_GPIO_PIN_0}

#define USE_RT_DEBUG_CONN_MNGR_PROCESS_EVNT_CLBK              (0)
#define GPIO_DEBUG_CONN_MNGR_PROCESS_EVNT_CLBK                {GPIOA, LL_PWR_GPIO_PIN_0}

#define USE_RT_DEBUG_CONN_MNGR_UPDT_CONN_PARAM_CBK            (0)
#define GPIO_DEBUG_CONN_MNGR_UPDT_CONN_PARAM_CBK              {GPIOA, LL_PWR_GPIO_PIN_0}

#define USE_RT_DEBUG_EVNT_SCHDLR_HW_EVNT_CMPLT                (0)
#define GPIO_DEBUG_EVNT_SCHDLR_HW_EVNT_CMPLT                  {GPIOA, LL_PWR_GPIO_PIN_0}

#define USE_RT_DEBUG_HCI_EVENT_HNDLR                          (0)
#define GPIO_DEBUG_HCI_EVENT_HNDLR                            {GPIOA, LL_PWR_GPIO_PIN_0}

#define USE_RT_DEBUG_MLME_TMRS_CBK                            (0)
#define GPIO_DEBUG_MLME_TMRS_CBK                              {GPIOA, LL_PWR_GPIO_PIN_0}

#define USE_RT_DEBUG_DIRECT_TX_EVNT_CBK                       (0)
#define GPIO_DEBUG_DIRECT_TX_EVNT_CBK                         {GPIOA, LL_PWR_GPIO_PIN_0}

#define USE_RT_DEBUG_INDIRECT_PKT_TOUR_CBK                    (0)
#define GPIO_DEBUG_INDIRECT_PKT_TOUR_CBK                      {GPIOA, LL_PWR_GPIO_PIN_0}

#define USE_RT_DEBUG_RADIO_CSMA_TMR                           (0)
#define GPIO_DEBUG_RADIO_CSMA_TMR                             {GPIOA, LL_PWR_GPIO_PIN_0}

#define USE_RT_DEBUG_RAL_SM_DONE_EVNT_CBK                     (0)
#define GPIO_DEBUG_RAL_SM_DONE_EVNT_CBK                       {GPIOB, LL_PWR_GPIO_PIN_4}

#define USE_RT_DEBUG_ED_TMR_HNDL                              (0)
#define GPIO_DEBUG_ED_TMR_HNDL                                {GPIOA, LL_PWR_GPIO_PIN_0}

#define USE_RT_DEBUG_OS_TMR_EVNT_CBK                          (0)
#define GPIO_DEBUG_OS_TMR_EVNT_CBK                            {GPIOA, LL_PWR_GPIO_PIN_0}

#define USE_RT_DEBUG_PROFILE_MARKER_PHY_WAKEUP_TIME           (0)
#define GPIO_DEBUG_PROFILE_MARKER_PHY_WAKEUP_TIME             {GPIOA, LL_PWR_GPIO_PIN_0}

#define USE_RT_DEBUG_PROFILE_END_DRIFT_TIME                   (0)
#define GPIO_DEBUG_PROFILE_END_DRIFT_TIME                     {GPIOA, LL_PWR_GPIO_PIN_0}

#define USE_RT_DEBUG_PROC_RADIO_RCV                           (0)
#define GPIO_DEBUG_PROC_RADIO_RCV                             {GPIOA, LL_PWR_GPIO_PIN_0}

#define USE_RT_DEBUG_EVNT_TIME_UPDT                           (0)
#define GPIO_DEBUG_EVNT_TIME_UPDT                             {GPIOA, LL_PWR_GPIO_PIN_0}

#define USE_RT_DEBUG_MAC_RECEIVE_DONE                         (0)
#define GPIO_DEBUG_MAC_RECEIVE_DONE                           {GPIOA, LL_PWR_GPIO_PIN_0}

#define USE_RT_DEBUG_MAC_TX_DONE                              (0)
#define GPIO_DEBUG_MAC_TX_DONE                                {GPIOA, LL_PWR_GPIO_PIN_0}

#define USE_RT_DEBUG_RADIO_APPLY_CSMA                         (0)
#define GPIO_DEBUG_RADIO_APPLY_CSMA                           {GPIOA, LL_PWR_GPIO_PIN_0}

#define USE_RT_DEBUG_RADIO_TRANSMIT                           (0)
#define GPIO_DEBUG_RADIO_TRANSMIT                             {GPIOA, LL_PWR_GPIO_PIN_0}

#define USE_RT_DEBUG_PROC_RADIO_TX                            (0)
#define GPIO_DEBUG_PROC_RADIO_TX                              {GPIOA, LL_PWR_GPIO_PIN_0}

#define USE_RT_DEBUG_RAL_TX_DONE                              (0)
#define GPIO_DEBUG_RAL_TX_DONE                                {GPIOA, LL_PWR_GPIO_PIN_0}

#define USE_RT_DEBUG_RAL_TX_DONE_INCREMENT_BACKOFF_COUNT      (0)
#define GPIO_DEBUG_RAL_TX_DONE_INCREMENT_BACKOFF_COUNT        {GPIOA, LL_PWR_GPIO_PIN_0}

#define USE_RT_DEBUG_RAL_TX_DONE_RST_BACKOFF_COUNT            (0)
#define GPIO_DEBUG_RAL_TX_DONE_RST_BACKOFF_COUNT              {GPIOA, LL_PWR_GPIO_PIN_0}

#define USE_RT_DEBUG_RAL_CONTINUE_RX                          (0)
#define GPIO_DEBUG_RAL_CONTINUE_RX                            {GPIOA, LL_PWR_GPIO_PIN_0}

#define USE_RT_DEBUG_RAL_PERFORM_CCA                          (0)
#define GPIO_DEBUG_RAL_PERFORM_CCA                            {GPIOA, LL_PWR_GPIO_PIN_0}

#define USE_RT_DEBUG_RAL_ENABLE_TRANSMITTER                   (0)
#define GPIO_DEBUG_RAL_ENABLE_TRANSMITTER                     {GPIOA, LL_PWR_GPIO_PIN_0}

#define USE_RT_DEBUG_LLHWC_GET_CH_IDX_ALGO_2                  (0)
#define GPIO_DEBUG_LLHWC_GET_CH_IDX_ALGO_2                    {GPIOA, LL_PWR_GPIO_PIN_0}

#define USE_RT_DEBUG_BACK_FROM_DEEP_SLEEP                     (0)
#define GPIO_DEBUG_BACK_FROM_DEEP_SLEEP                       {GPIOA, LL_PWR_GPIO_PIN_0}

/* Application signal selection and GPIO assignment.
   CAN BE MODIFIED BY USER */

#define USE_RT_DEBUG_APP_APPE_INIT                            (0)
#define GPIO_DEBUG_APP_APPE_INIT                              {GPIOA, LL_PWR_GPIO_PIN_0}

/********************************/
/** Debug configuration setup **/
/*******************************/

/*
 *
 * Debug configuration for System purpose
 *
 */
#if (USE_RT_DEBUG_CONFIGURATION_SYSTEM == 1U)
/* SCM_SETUP activation */
#undef USE_RT_DEBUG_SCM_SETUP
#define USE_RT_DEBUG_SCM_SETUP                                (1U)

/* SCM_SYSTEM_CLOCK_CONFIG activation */
#undef USE_RT_DEBUG_SCM_SYSTEM_CLOCK_CONFIG
#define USE_RT_DEBUG_SCM_SYSTEM_CLOCK_CONFIG                  (1U)

/* SCM_HSERDY_ISR activation */
#undef USE_RT_DEBUG_SCM_HSERDY_ISR
#define USE_RT_DEBUG_SCM_HSERDY_ISR                           (1U)

/* LOW_POWER_STOP_MODE_ACTIVE activation */
#undef USE_RT_DEBUG_LOW_POWER_STOP_MODE_ACTIVE
#define USE_RT_DEBUG_LOW_POWER_STOP_MODE_ACTIVE               (1U)

/* ADC_ACTIVATION activation */
#undef USE_RT_DEBUG_ADC_ACTIVATION
#define USE_RT_DEBUG_ADC_ACTIVATION                           (1U)

/* ADC_TEMPERATURE_ACQUISITION activation */
#undef USE_RT_DEBUG_ADC_TEMPERATURE_ACQUISITION
#define USE_RT_DEBUG_ADC_TEMPERATURE_ACQUISITION              (1U)

/* RNG_GEN_RAND_NUM activation */
#undef USE_RT_DEBUG_RNG_GEN_RAND_NUM
#define USE_RT_DEBUG_RNG_GEN_RAND_NUM                         (1U)

/* LOW_POWER_STANDBY_MODE_ACTIVE activation */
#undef USE_RT_DEBUG_LOW_POWER_STANDBY_MODE_ACTIVE
#define USE_RT_DEBUG_LOW_POWER_STANDBY_MODE_ACTIVE            (1U)

/*
 *
 * Debug configuration for BLE purpose
 *
 */
#elif (USE_RT_DEBUG_CONFIGURATION_BLE == 1U)

/* LLHWC_CMN_LW_ISR activation */
#undef USE_RT_DEBUG_LLHWC_CMN_LW_ISR
#define USE_RT_DEBUG_LLHWC_CMN_LW_ISR                         (1U)

/* LLHWC_CMN_HG_ISR activation */
#undef USE_RT_DEBUG_LLWCC_CMN_HG_ISR
#define USE_RT_DEBUG_LLWCC_CMN_HG_ISR                         (1U)

/* PHY_CLBR_EXEC activation */
#undef USE_RT_DEBUG_PHY_CLBR_EXEC
#define USE_RT_DEBUG_PHY_CLBR_EXEC                            (1U)

/* SCHDLR_EVNT_RGSTR activation */
#undef USE_RT_DEBUG_SCHDLR_EVNT_RGSTR
#define USE_RT_DEBUG_SCHDLR_EVNT_RGSTR                        (1U)

/* SCHDLR_HNDL_MISSED_EVNT activation */
#undef USE_RT_DEBUG_SCHDLR_HNDL_MISSED_EVNT
#define USE_RT_DEBUG_SCHDLR_HNDL_MISSED_EVNT                  (1U)

/* SCHDLR_HNDL_NXT_TRACE activation */
#undef USE_RT_DEBUG_SCHDLR_HNDL_NXT_TRACE
#define USE_RT_DEBUG_SCHDLR_HNDL_NXT_TRACE                    (1U)

/* SCHDLR_EXEC_EVNT_TRACE activation */
#undef USE_RT_DEBUG_SCHDLR_EXEC_EVNT_TRACE
#define USE_RT_DEBUG_SCHDLR_EXEC_EVNT_TRACE                   (1U)

/* PHY_CLBR_ISR activation */
#undef USE_RT_DEBUG_PHY_CLBR_ISR
#define USE_RT_DEBUG_PHY_CLBR_ISR                             (1U)

/*
 *
 * Debug configuration for MAC purpose
 *
 */
#elif (USE_RT_DEBUG_CONFIGURATION_MAC == 1U)

/* LLHWC_CMN_LW_ISR activation */
#undef USE_RT_DEBUG_LLHWC_CMN_LW_ISR
#define USE_RT_DEBUG_LLHWC_CMN_LW_ISR                         (1U)

/* LLHWC_CMN_HG_ISR activation */
#undef USE_RT_DEBUG_LLWCC_CMN_HG_ISR
#define USE_RT_DEBUG_LLWCC_CMN_HG_ISR                         (1U)

/* RAL_ISR_TRACE activation */
#undef USE_RT_DEBUG_RAL_ISR_TRACE
#define USE_RT_DEBUG_RAL_ISR_TRACE                            (1U)

/* RAL_SM_DONE_EVNT_CBK activation */
#undef USE_RT_DEBUG_RAL_SM_DONE_EVNT_CBK
#define USE_RT_DEBUG_RAL_SM_DONE_EVNT_CBK                     (1U)

/* RAL_STOP_OPRTN activation */
#undef USE_RT_DEBUG_RAL_STOP_OPRTN
#define USE_RT_DEBUG_RAL_STOP_OPRTN                           (1U)

/* RAL_STRT_RX activation */
#undef USE_RT_DEBUG_RAL_STRT_RX
#define USE_RT_DEBUG_RAL_STRT_RX                              (1U)

/* RAL_STRT_TX activation */
#undef USE_RT_DEBUG_RAL_STRT_TX
#define USE_RT_DEBUG_RAL_STRT_TX                              (1U)

/*
 *
 * Debug configuration for COEX purpose
 *
 */
#elif (USE_RT_DEBUG_CONFIGURATION_COEX == 1U)

/* COEX_ASK_FOR_AIR activation */
#undef USE_RT_DEBUG_COEX_ASK_FOR_AIR
#define USE_RT_DEBUG_COEX_ASK_FOR_AIR                         (1U)

/* COEX_FORCE_STOP_RX activation */
#undef USE_RT_DEBUG_COEX_FORCE_STOP_RX
#define USE_RT_DEBUG_COEX_FORCE_STOP_RX                       (1U)

/* COEX_STRT_ON_IDLE activation */
#undef USE_RT_DEBUG_COEX_STRT_ON_IDLE
#define USE_RT_DEBUG_COEX_STRT_ON_IDLE                        (1U)

/* COEX_STRT_ONE_SHOT activation */
#undef USE_RT_DEBUG_COEX_STRT_ONE_SHOT
#define USE_RT_DEBUG_COEX_STRT_ONE_SHOT                       (1U)

/* SCHDLR_HNDL_NXT_TRACE activation */
#undef USE_RT_DEBUG_SCHDLR_HNDL_NXT_TRACE
#define USE_RT_DEBUG_SCHDLR_HNDL_NXT_TRACE                    (1U)

/* SCHDLR_EXEC_EVNT_TRACE activation */
#undef USE_RT_DEBUG_SCHDLR_EXEC_EVNT_TRACE
#define USE_RT_DEBUG_SCHDLR_EXEC_EVNT_TRACE                   (1U)

/* RAL_SM_DONE_EVNT_CBK activation */
#undef USE_RT_DEBUG_RAL_SM_DONE_EVNT_CBK
#define USE_RT_DEBUG_RAL_SM_DONE_EVNT_CBK                     (1U)

/* RAL_STOP_OPRTN activation */
#undef USE_RT_DEBUG_RAL_STOP_OPRTN
#define USE_RT_DEBUG_RAL_STOP_OPRTN                           (1U)

#else
/* Nothing to do */
#endif /* (USE_RT_DEBUG_CONFIGURATION_COEX == 1U) */

#endif /* CFG_RT_DEBUG_GPIO_MODULE */

/******************************************************************/
/** Association table between general debug signal and used gpio **/
/******************************************************************/

#include "debug_signals.h"

#if (CFG_RT_DEBUG_GPIO_MODULE == 1)

#include "stm32wbaxx_hal.h"

typedef struct {
  GPIO_TypeDef* GPIO_port;
  uint16_t GPIO_pin;
} st_gpio_debug_t;

extern const st_gpio_debug_t general_debug_table[RT_DEBUG_SIGNALS_TOTAL_NUM];

#endif /* CFG_RT_DEBUG_GPIO_MODULE */

#ifdef __cplusplus
}
#endif

#endif /* DEBUG_CONFIG_H */
