/* USER CODE BEGIN Header */
/*
 *
 *    Copyright (c) 2026 Project CHIP Authors
 *    All rights reserved.
 *
 *    Licensed under the Apache License, Version 2.0 (the "License");
 *    you may not use this file except in compliance with the License.
 *    You may obtain a copy of the License at
 *
 *        http://www.apache.org/licenses/LICENSE-2.0
 *
 *    Unless required by applicable law or agreed to in writing, software
 *    distributed under the License is distributed on an "AS IS" BASIS,
 *    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *    See the License for the specific language governing permissions and
 *    limitations under the License.
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
