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
#include "platform.h"
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include <stdio.h>


#define BUFFER_SIZE          2048

static uint8_t _buffer[BUFFER_SIZE] = { 0 };


static VoidCommand _void_command;
static InitCommand _init_command;
static CsrCommand _csr_command;
static ImportCommand _import_command;
static SetupCommand _setup_command;


Command & select_command(uint8_t id)
{
    switch(id)
    {
    case kCommand_Init:
        return _init_command;
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

    platform_write(output.data(), output.offset());
}

void reject_command(uint8_t id, int err)
{
    Encoder out(_buffer, sizeof(_buffer));
    out.addUint8(id);
    out.addInt32(err);
    platform_write(out.data(), out.offset());
}

/*******************************************************************************
 * Initialize application.
 ******************************************************************************/

void app_init(void)
{
}

/*******************************************************************************
 * App ticking function.
 ******************************************************************************/

void app_process_action(void)
{
    uint8_t command_id = kCommand_None;
    size_t in_size = 0;
    int status = 0;

    // Read input
    status = platform_read((void *)_buffer, sizeof(_buffer), &in_size);
    ASSERT((0 == status) && (in_size > 0), return, "RX error");

    // Decode header
    Encoder input(_buffer, in_size);
    int err = input.getUint8(command_id);
    ASSERT(!err, reject_command(command_id, err); return, "Decode error");

    // Execute command
    execute_command(command_id, input);
}
