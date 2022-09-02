/***************************************************************************/ /**
* @file main.c
* @brief main() function.
*******************************************************************************
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

#include "host_creds.h"
#include "sl_component_catalog.h"
#include "sl_system_init.h"
#include "sl_system_process_action.h"
#include "af.h"

#ifdef EMBER_TEST
int nodeMain(void)
#else
int main(int argc, char *argv[])
#endif
{
#ifndef EMBER_TEST
    {
        // Initialize ezspProcessCommandOptions and gatewayBackchannelStart
        // for host apps running on hardware.
        int returnCode;
        if (emberAfMainStartCallback(&returnCode, argc, argv))
        {
            return returnCode;
        }
    }
#endif // EMBER_TEST

    // Initialize Silicon Labs device, system, service(s) and protocol stack(s).
    // Note that if the kernel is present, processing task(s) will be created by
    // this call.
    sl_system_init();

    // Do not remove this call: Silicon Labs components process action routine
    // must be called from the super loop.
    sl_system_process_action();

    // Application process.
    return host_creds_csr();
}
