/*******************************************************************************
 * (c) 2020, Cypress Semiconductor Corporation. All rights reserved.
 *******************************************************************************
 * This software, including source code, documentation and related materials
 * ("Software"), is owned by Cypress Semiconductor Corporation or one of its
 * subsidiaries ("Cypress") and is protected by and subject to worldwide patent
 * protection (United States and foreign), United States copyright laws and
 * international treaty provisions. Therefore, you may use this Software only
 * as provided in the license agreement accompanying the software package from
 * which you obtained this Software ("EULA").
 *
 * If no EULA applies, Cypress hereby grants you a personal, non-exclusive,
 * non-transferable license to copy, modify, and compile the Software source
 * code solely for use in connection with Cypress's integrated circuit products.
 * Any reproduction, modification, translation, compilation, or representation
 * of this Software except as specified above is prohibited without the express
 * written permission of Cypress.
 *
 * Disclaimer: THIS SOFTWARE IS PROVIDED AS-IS, WITH NO WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING, BUT NOT LIMITED TO, NONINFRINGEMENT, IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE. Cypress
 * reserves the right to make changes to the Software without notice. Cypress
 * does not assume any liability arising out of the application or use of the
 * Software or any product or circuit described in the Software. Cypress does
 * not authorize its products for use in any products where a malfunction or
 * failure of the Cypress product may reasonably be expected to result in
 * significant property damage, injury or death ("High Risk Product"). By
 * including Cypress's product in a High Risk Product, the manufacturer of such
 * system or application assumes all risk of such use and in doing so agrees to
 * indemnify Cypress against all liability.
 *******************************************************************************/

/*******************************************************************************
 * File Name: app_platform_cfg.c
 * Version: 1.0
 *
 * Description:
 *   Runtime Bluetooth stack configuration parameters
 *
 *******************************************************************************/
#include "app_platform_cfg.h"
#include "wiced_bt_dev.h"

const cybt_platform_config_t bt_platform_cfg_settings =
{
    .hci_config =
    {
        .hci_transport = CYBT_HCI_UART,

        .hci =
        {
            .hci_uart =
            {
                .uart_tx_pin = CYBSP_BT_UART_TX,
                .uart_rx_pin = CYBSP_BT_UART_RX,
                .uart_rts_pin = CYBSP_BT_UART_RTS,
                .uart_cts_pin = CYBSP_BT_UART_CTS,

                .baud_rate_for_fw_download = 3000000,
                .baud_rate_for_feature     = 115200,

                .data_bits = 8,
                .stop_bits = 1,
                .parity = CYHAL_UART_PARITY_NONE,
                .flow_control = WICED_TRUE
            }
        }
    },

    .controller_config =
    {
        .bt_power_pin      = CYBSP_BT_POWER,
        .sleep_mode =
        {
#if (bt_0_power_0_ENABLED == 1)
#if (CYCFG_BT_LP_ENABLED == 1)
                 .sleep_mode_enabled   = CYCFG_BT_LP_ENABLED,
                 .device_wakeup_pin    = CYCFG_BT_DEV_WAKE_GPIO,
                 .host_wakeup_pin      = CYCFG_BT_HOST_WAKE_GPIO,
                 .device_wake_polarity = CYCFG_BT_DEV_WAKE_POLARITY,
                 .host_wake_polarity   = CYCFG_BT_HOST_WAKE_IRQ_EVENT

#else
                 .sleep_mode_enabled   = WICED_FALSE
#endif
#else
                .sleep_mode_enabled   = WICED_TRUE,
                .device_wakeup_pin    = CYBSP_BT_DEVICE_WAKE,
                .host_wakeup_pin      = CYBSP_BT_HOST_WAKE,
                .device_wake_polarity = CYBT_WAKE_ACTIVE_LOW,
                .host_wake_polarity   = CYBT_WAKE_ACTIVE_LOW
#endif
        }
    },

    .task_mem_pool_size    = 2048
};

/* [] END OF FILE */
