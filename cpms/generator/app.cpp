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

#include "assert.h"
#include "commands.h"
#include "encoding.h"
#include "credentials.h"
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include <stdio.h>
#include <em_msc.h>
#include <nvm3.h>
#include <sl_iostream_rtt.h>

#define BUFFER_SIZE          2048

static uint8_t _buffer[BUFFER_SIZE] = { 0 };

static VoidCommand _void_command;
static CsrCommand _csr_command;
static ImportCommand _import_command;
static SetupCommand _setup_command;


Command & select_command(uint8_t id)
{
    switch(id)
    {
    case kCommand_CSR:
        return _csr_command; 
    case kCommand_Import:
        return _import_command; 
    case kCommand_Setup:
        return _setup_command; 
    default:
        return _void_command; 
    }
}

void execute_command(uint8_t id, Encoder & input)
{
    Command &cmd = select_command(id);
    int err = cmd.decode(input);
    if(!err)
    {
        err = cmd.execute();
    }

    Encoder output(_buffer, sizeof(_buffer));
    output.addUint8(id);
    output.addInt32(err);
    if(! err)
    {
        cmd.encode(output);
    }

    sl_iostream_write(sl_iostream_rtt_handle, output.data(), output.offset());
}

void reject_command(uint8_t id, int err)
{
    Encoder out(_buffer, sizeof(_buffer));
    out.addUint8(id);
    out.addInt32(err);
    sl_iostream_write(sl_iostream_rtt_handle, out.data(), out.offset());
}

/*******************************************************************************
 * Initialize application.
 ******************************************************************************/

void app_init(void)
{
    MSC_Init();

    // Initialize NVM3
    nvm3_open(nvm3_defaultHandle, nvm3_defaultInit);

    // Initialize PSA Crypto
    psa_crypto_init();
}

/*******************************************************************************
 * App ticking function.
 ******************************************************************************/

void app_process_action(void)
{
    uint8_t command_id = kCommand_None;
    size_t in_size = 0;
    sl_status_t status = SL_STATUS_OK;

    // Read input
    status = sl_iostream_read(sl_iostream_rtt_handle, (void *)_buffer, sizeof(_buffer), &in_size);
    ASSERT((SL_STATUS_OK == status) && (in_size > 0), return, "RX error");

    // Decode header
    Encoder input(_buffer, in_size);
    int err = input.getUint8(command_id);
    ASSERT(!err, reject_command(command_id, err); return, "Decode error");

    // Execute command
    execute_command(command_id, input);
}
