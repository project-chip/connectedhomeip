#include "ProvisionManager.h"
#include "ProvisionCommands.h"
#include "ProvisionEncoder.h"

#include <FreeRTOS.h>
#include <task.h>

#include <lib/support/logging/CHIPLogging.h>
#include <lib/support/CHIPPlatformMemory.h>
#include <lib/support/CHIPMem.h>
#include <mbedtls/platform.h>

namespace chip {
namespace DeviceLayer {
namespace Silabs {
namespace Provision {

static uint8_t buffer[1024] = { 0xff };
static InitCommand _init_command;
static CsrCommand _csr_command;
static ImportCommand _import_command;
static SetupCommand _setup_command;


#define MAIN_TASK_STACK_SIZE    (1024)
#define MAIN_TASK_PRIORITY      (configMAX_PRIORITIES - 1)
TaskHandle_t main_Task;


void taskMain(void * pvParameter)
{
    Manager *man = static_cast<Manager*>(pvParameter);
    if(man)
    {
        man->Run();
    }
}


CHIP_ERROR Manager::Start()
{
#if !defined(MBEDTLS_PLATFORM_CALLOC_MACRO) ||  !defined(MBEDTLS_PLATFORM_FREE_MACRO)
    mbedtls_platform_set_calloc_free(CHIPPlatformMemoryCalloc, CHIPPlatformMemoryFree);
    ReturnErrorOnFailure(chip::Platform::MemoryInit());
#endif
    xTaskCreate(taskMain, "Provision Task", MAIN_TASK_STACK_SIZE, this, MAIN_TASK_PRIORITY, &main_Task);
    return CHIP_NO_ERROR;
}

void Manager::Run()
{
    bool done = !ProvisionRequired();
    while (!done)
    {
        size_t bytes_read = 0;
        size_t offset = 0;

        while(CHIP_NO_ERROR == mChannel.Read(&buffer[offset], sizeof(buffer) - offset, bytes_read))
        {
            offset += bytes_read;
            bytes_read = 0;
        }
        if(offset > 0)
        {
            Encoder input(buffer, offset);
            Encoder output(buffer, sizeof(buffer));
            done = ProcessCommand(input, output);
            mChannel.Write(output.data(), output.offset());
        }
    }
    // Mark as provisioned
    mStore.SetProvisionRequest(false);

    // Reset
    vTaskDelay(pdMS_TO_TICKS(500));
    NVIC_SystemReset();
}


bool Manager::ProvisionRequired()
{
    bool requested = false;
    if(CHIP_NO_ERROR == mStore.GetProvisionRequest(requested))
    {
        return requested;
    }
#if defined(PROVISION_CHANNEL_ENABLED)
    return (PROVISION_CHANNEL_ENABLED > 0);
#else
    return false;
#endif
}


CHIP_ERROR Manager::RequestProvision()
{
    return mStore.SetProvisionRequest(true);
}


bool Manager::ProcessCommand(Encoder & input, Encoder & output)
{
    // Decode header
    Command *cmd = DecodeCommand(input);
    if(!cmd)
    {
        EncodeHeader(output, 0, CHIP_ERROR_INVALID_ARGUMENT);
        return true;
    }

    // Decode command
    CHIP_ERROR err = cmd->DecodeRequest(input);
    if(CHIP_NO_ERROR == err)
    {
        // Execute command
        err = cmd->Execute(mStore);
    }

    // Encode response
    EncodeHeader(output, cmd->id(), err);
    if(CHIP_NO_ERROR == err)
    {
        cmd->EncodeResponse(output);
        return (kCommand_Setup == cmd->id());
    }
    return false;
}

Command *Manager::DecodeCommand(Encoder & input)
{
    uint8_t command_id = kCommand_None;
    VerifyOrReturnError(CHIP_NO_ERROR == input.getUint8(command_id), nullptr);

    switch(command_id)
    {
    case kCommand_Init:
        return &_init_command;
    case kCommand_CSR:
        return &_csr_command;
    case kCommand_Import:
        return &_import_command;
    case kCommand_Setup:
        return &_setup_command;
    default:
        return nullptr;
    }
}

void Manager::EncodeHeader(Encoder & output, uint8_t cid, CHIP_ERROR err)
{
    output.addUint8(cid);
    output.addInt32(static_cast<uint32_t>(err.AsInteger()));
}

} // namespace Provision
} // namespace Silabs
} // namespace DeviceLayer
} // namespace chip
