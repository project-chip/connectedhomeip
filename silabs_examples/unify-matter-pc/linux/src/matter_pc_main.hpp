/******************************************************************************
 * # License
 * <b>Copyright 2023 Silicon Laboratories Inc. www.silabs.com</b>
 ******************************************************************************
 * The licensor of this software is Silicon Laboratories Inc. Your use of this
 * software is governed by the terms of Silicon Labs Master Software License
 * Agreement (MSLA) available at
 * www.silabs.com/about-us/legal/master-software-license-agreement. This
 * software is distributed to you in Source Code format and is governed by the
 * sections of the MSLA applicable to Source Code.
 *
 *****************************************************************************/

 #ifndef MATTER_PC_MAIN_HPP
 #define MATTER_PC_MAIN_HPP

/**
 * @brief Schedule the contili main loop to be run
 * 
 * This will schedule the contiki main loop to be run. This is needed 
 * when we are posting contiki events ouside the normal contiki processes.
 * Ie, after calling contiki code from a Matter Callback
 * 
 */
void mpc_schedule_contiki();

 #endif