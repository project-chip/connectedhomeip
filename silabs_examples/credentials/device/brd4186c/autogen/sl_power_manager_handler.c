#include <stdbool.h>
#include "em_core.h"
#include "sl_power_manager.h"
#include "sl_sleeptimer.h"
#include "sl_iostream_init_usart_instances.h"
#include "sl_legacy_hal_integration_hooks.h"
#include "zigbee_app_framework_common.h"
#include "serial-interface.h"

/***************************************************************************//**
 * Check if the MCU can sleep at that time. This function is called when the system
 * is about to go sleeping, with the interrupts disabled. It allows the software to
 * cancel going to sleep in case of a last-minute event occurred (window between the
 * function call and interrupt disable).
 *
 * @return  True, if the system can go to sleep.
 *          False, otherwise.
 *
 * @note  This function is called with the interrupt disabled and it MUST NOT be
 *        re-enabled.
 ******************************************************************************/
__WEAK bool app_is_ok_to_sleep(void)
{
  return true;
}

/***************************************************************************//**
 * Check if the MCU can sleep after an interrupt. This function is called after an
 * interrupt occured and was processed. It allows the power manger to know if it must
 * go back to sleep or wakeup.
 *
 * @return  SL_POWER_MANAGER_IGNORE, if the module did not trigger an ISR and it
 *          won't to contribute to the decision.
 *
 *          SL_POWER_MANAGER_SLEEP, The module was the one that caused the system
 *          wakeup and the system SHOULD go back to sleep.
 *
 *          SL_POWER_MANAGER_WAKEUP, The module was the one that caused the system
 *          wakeup and the system MUST NOT go back to sleep.
 *
 * @note  This function must not have any side effects. It is not guaranteed to be
 *        called for every ISR. If a prior hook function requires to wakeup, such
 *        as a wireless stack, the application hook function won't be called.
 ******************************************************************************/
__WEAK sl_power_manager_on_isr_exit_t app_sleep_on_isr_exit(void)
{
  return SL_POWER_MANAGER_IGNORE;
}

/***************************************************************************//**
 * Mandatory callback that allows to cancel sleeping action. This function is called
 * when the system is about to go sleeping, with the interrupts disabled. It allows
 * the software to cancel going to sleep in case of a last-minute event occurred
 * (window between the function call and interrupt disable).
 ******************************************************************************/
bool sl_power_manager_is_ok_to_sleep(void)
{
  bool ok_to_sleep = true;
  if (sl_legacy_hal_is_ok_to_sleep() == false) {
    ok_to_sleep = false;
  }
  if (sli_zigbee_app_framework_is_ok_to_sleep() == false) {
    ok_to_sleep = false;
  }
  if (serialOkToSleep() == false) {
    ok_to_sleep = false;
  }
  // Application hook
  if (app_is_ok_to_sleep() == false) {
    ok_to_sleep = false;
  }

  return ok_to_sleep;
}

/***************************************************************************//**
 * Mandatory callback that must validate if the MCU can sleep after having
 * processed an interrupt when the system was sleeping.
 ******************************************************************************/
bool sl_power_manager_sleep_on_isr_exit(void)
{
  sl_power_manager_on_isr_exit_t answer;
  bool sleep = false;
  bool force_wakeup = false;
   
  // This function allow the power manager to return into sleep, if the latest timer 
  // to expire was power manager's internal one used to restore HFXO on time or 
  // the HFXO interrupt. 
  // Most of the time we want to get back to sleep until the next event occurs.
  sleep = sl_power_manager_is_latest_wakeup_internal();

  answer = sl_iostream_usart_vcom_sleep_on_isr_exit();
  if (answer == SL_POWER_MANAGER_WAKEUP) {
    force_wakeup = true;
  } else if (answer == SL_POWER_MANAGER_SLEEP) {
    sleep = true;
  }

  answer = sl_legacy_hal_sleep_on_isr_exit();
  if (answer == SL_POWER_MANAGER_WAKEUP) {
    force_wakeup = true;
  } else if (answer == SL_POWER_MANAGER_SLEEP) {
    sleep = true;
  }

  answer = sli_zigbee_sleep_on_isr_exit();
  if (answer == SL_POWER_MANAGER_WAKEUP) {
    force_wakeup = true;
  } else if (answer == SL_POWER_MANAGER_SLEEP) {
    sleep = true;
  }

  // Application hook
  answer = app_sleep_on_isr_exit();
  if (answer == SL_POWER_MANAGER_WAKEUP) {
    force_wakeup = true;
  } else if (answer == SL_POWER_MANAGER_SLEEP) {
    sleep = true;
  }
  
  if (force_wakeup) {
    sleep = false;
  }

  return sleep;
}