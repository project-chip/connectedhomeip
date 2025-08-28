/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    app_debug_signal_def.h
  * @author  MCD Application Team
  * @brief   Real Time Debug module application signal definition
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2023 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* This header is part of the RTDebug module mechanisim.
 * In this file, application global debug signals can be
 * define by the user without modyfing files in the module core.
 *
 * There is no need to add this header file as include file
 * in the application files. This is handled by the module itself.
 */

#if (USE_RT_DEBUG_APP_APPE_INIT == 1)
  RT_DEBUG_APP_APPE_INIT,
#endif /* USE_RT_DEBUG_APP_APPE_INIT */
