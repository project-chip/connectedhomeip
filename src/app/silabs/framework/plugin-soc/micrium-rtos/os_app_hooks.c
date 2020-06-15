/**
 *
 *    Copyright (c) 2020 Silicon Labs
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
/***************************************************************************//**
 * @file
 * @brief Hooks for interfacing with the Micrium OS.
 *******************************************************************************
   ******************************************************************************/

#define   MICRIUM_SOURCE
#ifdef EMBER_AF_NCP
#include "app/framework/include/af-ncp.h"
#else // !EMBER_AF_NCP
#include "app/framework/include/af.h"
#endif // EMBER_AF_NCP
#include <kernel/include/os.h>
#include <em_core.h>

#if (OS_CFG_TASK_STK_REDZONE_EN == DEF_ENABLED)
void  App_OS_RedzoneHitHook (OS_TCB  *p_tcb);
#endif
void  App_OS_StatTaskHook (void);
void  App_OS_TaskCreateHook (OS_TCB  *p_tcb);
void  App_OS_TaskDelHook (OS_TCB  *p_tcb);
void  App_OS_TaskReturnHook (OS_TCB  *p_tcb);
void  App_OS_TaskSwHook (void);

/*
 ************************************************************************************************************************
 *                                              SET ALL APPLICATION HOOKS
 *
 * Description: Set ALL application hooks.
 *
 * Arguments  : none.
 *
 * Note(s)    : none
 ************************************************************************************************************************
 */
void  App_OS_SetAllHooks(void)
{
#if OS_CFG_APP_HOOKS_EN > 0u
  CORE_DECLARE_IRQ_STATE;

  CORE_ENTER_ATOMIC();
#if (OS_CFG_TASK_STK_REDZONE_EN == DEF_ENABLED)
  OS_AppRedzoneHitHookPtr = App_OS_RedzoneHitHook;
#endif

  OS_AppStatTaskHookPtr   = App_OS_StatTaskHook;

  OS_AppTaskCreateHookPtr = App_OS_TaskCreateHook;

  OS_AppTaskDelHookPtr    = App_OS_TaskDelHook;

  OS_AppTaskReturnHookPtr = App_OS_TaskReturnHook;

  OS_AppTaskSwHookPtr     = App_OS_TaskSwHook;
  CORE_EXIT_ATOMIC();
#endif
}

/*
 ************************************************************************************************************************
 *                                             CLEAR ALL APPLICATION HOOKS
 *
 * Description: Clear ALL application hooks.
 *
 * Arguments  : none.
 *
 * Note(s)    : none
 ************************************************************************************************************************
 */
void  App_OS_ClrAllHooks(void)
{
#if OS_CFG_APP_HOOKS_EN > 0u
  CORE_DECLARE_IRQ_STATE;

  CORE_ENTER_ATOMIC();
#if (OS_CFG_TASK_STK_REDZONE_EN == DEF_ENABLED)
  OS_AppRedzoneHitHookPtr = (OS_APP_HOOK_TCB)0;
#endif

  OS_AppStatTaskHookPtr   = (OS_APP_HOOK_VOID)0;

  OS_AppTaskCreateHookPtr = (OS_APP_HOOK_TCB)0;

  OS_AppTaskDelHookPtr    = (OS_APP_HOOK_TCB)0;

  OS_AppTaskReturnHookPtr = (OS_APP_HOOK_TCB)0;

  OS_AppTaskSwHookPtr     = (OS_APP_HOOK_VOID)0;
  CORE_EXIT_ATOMIC();
#endif
}

/*
 ************************************************************************************************************************
 *                                              APPLICATION IDLE TASK HOOK
 *
 * Description: This function is called by the idle task.  This hook has been added to allow you to do such things as
 *              STOP the CPU to conserve power.
 *
 * Arguments  : none
 *
 * Note(s)    : none
 ************************************************************************************************************************
 */

void emAfPluginMicriumRtosSleepHandler(void);

void  OSIdleEnterHook(void)
{
#if (HAL_WDOG_ENABLE)
  halResetWatchdog();
#endif

#if defined(EMBER_AF_PLUGIN_MICRIUM_RTOS_SLEEP_ENABLE) || defined(EMBER_AF_NCP)
  // On NCP this handler only idle the processor.
  emAfPluginMicriumRtosSleepHandler();
#endif
}

/*
 ************************************************************************************************************************
 *                                             APPLICATION REDZONE HIT HOOK
 *
 * Description: This function is called when a task's stack overflowed.
 *
 * Arguments  : p_tcb   is a pointer to the task control block of the offending task. NULL if ISR.
 *
 * Note(s)    : None.
 ************************************************************************************************************************
 */
#if (OS_CFG_TASK_STK_REDZONE_EN == DEF_ENABLED)
void  App_OS_RedzoneHitHook(OS_TCB  *p_tcb)
{
  while (DEF_ON) {
#if (HAL_WDOG_ENABLE)
    halResetWatchdog();
#endif

#if (OS_CFG_DBG_EN == DEF_ENABLED)
    emberSerialGuaranteedPrintf(APP_SERIAL, "stack overflow: %s\n",
                                ((p_tcb) ? p_tcb->NamePtr : "isr"));
#else
    emberSerialGuaranteedPrintf(APP_SERIAL, "stack overflow: p_tcb=%4x\n", p_tcb);
#endif
  }
}
#endif

/*
 ************************************************************************************************************************
 *                                           APPLICATION STATISTIC TASK HOOK
 *
 * Description: This function is called every second by uC/OS-III's statistics task.  This allows your application to add
 *              functionality to the statistics task.
 *
 * Arguments  : none
 *
 * Note(s)    : none
 ************************************************************************************************************************
 */
void  App_OS_StatTaskHook(void)
{
}

/*
 ************************************************************************************************************************
 *                                            APPLICATION TASK CREATION HOOK
 *
 * Description: This function is called when a task is created.
 *
 * Arguments  : p_tcb   is a pointer to the task control block of the task being created.
 *
 * Note(s)    : none
 ************************************************************************************************************************
 */
void  App_OS_TaskCreateHook(OS_TCB  *p_tcb)
{
  (void)&p_tcb;
}

/*
 ************************************************************************************************************************
 *                                            APPLICATION TASK DELETION HOOK
 *
 * Description: This function is called when a task is deleted.
 *
 * Arguments  : p_tcb   is a pointer to the task control block of the task being deleted.
 *
 * Note(s)    : none
 ************************************************************************************************************************
 */
void  App_OS_TaskDelHook(OS_TCB  *p_tcb)
{
  (void)&p_tcb;
}

/*
 ************************************************************************************************************************
 *                                             APPLICATION TASK RETURN HOOK
 *
 * Description: This function is called if a task accidentally returns.  In other words, a task should either be an
 *              infinite loop or delete itself when done.
 *
 * Arguments  : p_tcb     is a pointer to the OS_TCB of the task that is returning.
 *
 * Note(s)    : none
 ************************************************************************************************************************
 */
void  App_OS_TaskReturnHook(OS_TCB  *p_tcb)
{
  (void)&p_tcb;
}

/*
 ************************************************************************************************************************
 *                                             APPLICATION TASK SWITCH HOOK
 *
 * Description: This function is called when a task switch is performed.  This allows you to perform other operations
 *              during a context switch.
 *
 * Arguments  : none
 *
 * Note(s)    : 1) Interrupts are disabled during this call.
 *              2) It is assumed that the global pointer 'OSTCBHighRdyPtr' points to the TCB of the task that will be
 *                 'switched in' (i.e. the highest priority task) and, 'OSTCBCurPtr' points to the task being switched out
 *                 (i.e. the preempted task).
 ************************************************************************************************************************
 */
void  App_OS_TaskSwHook(void)
{
}
