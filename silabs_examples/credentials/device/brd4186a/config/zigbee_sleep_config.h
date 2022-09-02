/***************************************************************************//**
 * @brief Zigbee Application Framework common component configuration header.
 *\n*******************************************************************************
 * # License
 * <b>Copyright 2021 Silicon Laboratories Inc. www.silabs.com</b>
 *******************************************************************************
 *
 * The licensor of this software is Silicon Laboratories Inc. Your use of this
 * software is governed by the terms of Silicon Labs Master Software License
 * Agreement (MSLA) available at
 * www.silabs.com/about-us/legal/master-software-license-agreement. This
 * software is distributed to you in Source Code format and is governed by the
 * sections of the MSLA applicable to Source Code.
 *
 ******************************************************************************/

// <<< Use Configuration Wizard in Context Menu >>>

// <h>Zigbee Sleep configuration

// <o SL_ZIGBEE_APP_FRAMEWORK_MINIMUM_SLEEP_DURATION_MS> Minimum Sleep Duration <5-1000>
// <i> Default: 5
// <i> The minimum duration in milliseconds that the application will attempt to sleep for. If the sleep duration would be less than this amount, the application will not sleep.
#define SL_ZIGBEE_APP_FRAMEWORK_MINIMUM_SLEEP_DURATION_MS   5

// <o SL_ZIGBEE_APP_FRAMEWORK_BACKOFF_SLEEP_MS> Sleep Backoff time <0-10000>
// <i> Default: 0
// <i> This setting will keep a device from going back to sleep immediately upon waking up
#define SL_ZIGBEE_APP_FRAMEWORK_BACKOFF_SLEEP_MS   0

// <q SL_ZIGBEE_APP_FRAMEWORK_STAY_AWAKE_WHEN_NOT_JOINED> Stay awake when NOT joined
// <i> Default: TRUE
// <i> This will force a device to stay awake even when not joined to the network. This is often used for debugging and is not recommended for production devices since it cause the device to consume battery power even when not joined to a ZigBee network.
#define SL_ZIGBEE_APP_FRAMEWORK_STAY_AWAKE_WHEN_NOT_JOINED   1

// <q SL_ZIGBEE_APP_FRAMEWORK_USE_BUTTON_TO_STAY_AWAKE> Use button to force wakeup or allow sleep
// <i> Default: FALSE
// <i> This will setup the hardware buttons to wake-up or allow the device to go to sleep.  Button 0 will force the device to wake up and stay awake.  Button 1 will turn off this behavior to allow the device to sleep normally. Please note that in order for this option to be fully functional, button 0 and button 1 have to be configured to wake the device from sleep.
#define SL_ZIGBEE_APP_FRAMEWORK_USE_BUTTON_TO_STAY_AWAKE   0
// </h>

// <<< end of configuration section >>>
