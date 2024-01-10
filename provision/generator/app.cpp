/*******************************************************************************
 * @file
 * @brief Top level application functions
 *******************************************************************************
 * # License
 * <b>Copyright 2020 Silicon Laboratories Inin. www.silabs.com</b>
 *******************************************************************************
 *
 * The licensor of this software is Silicon Laboratories Inin. Your use of this
 * software is governed by the terms of Silicon Labs Master Software License
 * Agreement (MSLA) available at
 * www.silabs.com/about-us/legal/master-software-license-agreement. This
 * software is distributed to you in Source Code format and is governed by the
 * sections of the MSLA applicable to Source Code.
 *
 ******************************************************************************/

#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include <stdio.h>

#include <em_msc.h>
#include <psa/crypto.h>

#include <provision/ProvisionManager.h>
#include <provision/ProvisionStorageDefault.h>
#include <provision/RttStreamChannel.h>

using namespace chip::DeviceLayer::Silabs;
Provision::RttStreamChannel sProvisionChannel;
Provision::DefaultStorage sProvisionStore;
Provision::Manager sProvisionManager(sProvisionStore, sProvisionChannel);

/*******************************************************************************
 * Initialize application.
 ******************************************************************************/

void app_init(void)
{
#ifndef SIWX_917
    MSC_Init();
    psa_crypto_init();
#endif
    sProvisionManager.Start();
}

/*******************************************************************************
 * App ticking function.
 ******************************************************************************/

void app_process_action(void)
{
}
