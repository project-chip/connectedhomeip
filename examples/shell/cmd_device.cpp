/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
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

#include <core/CHIPCore.h>

#if CONFIG_DEVICE_LAYER

#include <platform/CHIPDeviceLayer.h>
#include <shell/shell.h>
#include <support/CHIPArgParser.hpp>
#include <support/CHIPMem.h>
#include <support/CodeUtils.h>

using namespace chip;
using namespace chip::Shell;
using namespace chip::Platform;
using namespace chip::DeviceLayer;
using namespace chip::Logging;
using namespace chip::ArgParser;

static chip::Shell::Shell sShellDeviceSubcommands;

int cmd_device_help_iterator(shell_command_t * command, void * arg)
{
    streamer_printf(streamer_get(), "  %-15s %s\n\r", command->cmd_name, command->cmd_help);
    return 0;
}

int cmd_device_help(int argc, char ** argv)
{
    sShellDeviceSubcommands.ForEachCommand(cmd_device_help_iterator, NULL);
    return 0;
}

int cmd_device_start(int argc, char ** argv)
{
    CHIP_ERROR error  = CHIP_NO_ERROR;
    streamer_t * sout = streamer_get();

    VerifyOrExit(argc == 0, error = CHIP_ERROR_INVALID_ARGUMENT);

    streamer_printf(sout, "Init CHIP Stack\r\n");
    error = PlatformMgr().InitChipStack();
    SuccessOrExit(error);

    streamer_printf(sout, "Starting Platform Manager Event Loop\r\n");
    error = PlatformMgr().StartEventLoopTask();
    SuccessOrExit(error);

exit:
    return error;
}

static CHIP_ERROR ConfigGetDone(streamer_t * sout, CHIP_ERROR error)
{
    if (error)
    {
        streamer_printf(sout, "<None>");
    }
    streamer_printf(sout, "\r\n");
    return error;
}

static CHIP_ERROR ConfigGetVendorId(bool printHeader)
{
    CHIP_ERROR error  = CHIP_NO_ERROR;
    streamer_t * sout = streamer_get();
    uint16_t value16;

    if (printHeader)
    {
        streamer_printf(sout, "VendorId:        ");
    }
    SuccessOrExit(error = ConfigurationMgr().GetVendorId(value16));
    streamer_printf(sout, "%04x", value16);

exit:
    return ConfigGetDone(sout, error);
}

static CHIP_ERROR ConfigGetProductId(bool printHeader)
{
    CHIP_ERROR error  = CHIP_NO_ERROR;
    streamer_t * sout = streamer_get();
    uint16_t value16;

    if (printHeader)
    {
        streamer_printf(sout, "ProductId:       ");
    }
    SuccessOrExit(error = ConfigurationMgr().GetProductId(value16));
    streamer_printf(sout, "%04x", value16);

exit:
    return ConfigGetDone(sout, error);
}

static CHIP_ERROR ConfigGetProductRevision(bool printHeader)
{
    CHIP_ERROR error  = CHIP_NO_ERROR;
    streamer_t * sout = streamer_get();
    uint16_t value16;

    if (printHeader)
    {
        streamer_printf(sout, "ProductRevision: ");
    }
    SuccessOrExit(error = ConfigurationMgr().GetProductRevision(value16));
    streamer_printf(sout, "%04x", value16);

exit:
    return ConfigGetDone(sout, error);
}

static CHIP_ERROR ConfigGetSerialNumber(bool printHeader)
{
    CHIP_ERROR error  = CHIP_NO_ERROR;
    streamer_t * sout = streamer_get();
    char buf[ConfigurationManager::kMaxSerialNumberLength];
    size_t bufSize;

    if (printHeader)
    {
        streamer_printf(sout, "SerialNumber:    ");
    }
    SuccessOrExit(error = ConfigurationMgr().GetSerialNumber(buf, sizeof(buf), bufSize));
    streamer_print_hex(sout, reinterpret_cast<const uint8_t *>(buf), bufSize);

exit:
    return ConfigGetDone(sout, error);
}

static CHIP_ERROR ConfigGetDeviceId(bool printHeader)
{
    CHIP_ERROR error  = CHIP_NO_ERROR;
    streamer_t * sout = streamer_get();
    uint64_t value64;

    if (printHeader)
    {
        streamer_printf(sout, "DeviceId:        ");
    }
    SuccessOrExit(error = ConfigurationMgr().GetDeviceId(value64));
    streamer_print_hex(sout, reinterpret_cast<const uint8_t *>(&value64), sizeof(value64));

exit:
    return ConfigGetDone(sout, error);
}

static CHIP_ERROR ConfigGetDeviceCert(bool printHeader)
{
    CHIP_ERROR error  = CHIP_NO_ERROR;
    streamer_t * sout = streamer_get();
    uint8_t * certBuf = NULL;
    size_t certLen;

    if (printHeader)
    {
        streamer_printf(sout, "DeviceCert:      ");
    }
    // Determine the length of the device certificate.
    error = ConfigurationMgr().GetDeviceCertificate((uint8_t *) NULL, 0, certLen);
    SuccessOrExit(error);

    // Fail if no certificate has been configured.
    VerifyOrExit(certLen != 0, error = CHIP_ERROR_CERT_NOT_FOUND);

    // Create a temporary buffer to hold the certificate.
    certBuf = (uint8_t *) MemoryAlloc(certLen);
    VerifyOrExit(certBuf != NULL, error = CHIP_ERROR_NO_MEMORY);

    // Read the certificate
    error = ConfigurationMgr().GetDeviceCertificate(certBuf, certLen, certLen);
    SuccessOrExit(error);

    streamer_print_hex(sout, const_cast<const uint8_t *>(certBuf), certLen);

exit:
    if (certBuf != NULL)
    {
        MemoryFree(certBuf);
    }
    return ConfigGetDone(sout, error);
}

static CHIP_ERROR ConfigGetDeviceCaCerts(bool printHeader)
{
    CHIP_ERROR error  = CHIP_NO_ERROR;
    streamer_t * sout = streamer_get();
    uint8_t * certBuf = NULL;
    size_t certLen;

    if (printHeader)
    {
        streamer_printf(sout, "DeviceCaCerts:   ");
    }
    // Determine the length of the device certificate.
    error = ConfigurationMgr().GetDeviceIntermediateCACerts((uint8_t *) NULL, 0, certLen);
    SuccessOrExit(error);

    // Fail if no certificate has been configured.
    VerifyOrExit(certLen != 0, error = CHIP_ERROR_CERT_NOT_FOUND);

    // Create a temporary buffer to hold the certificate.
    certBuf = (uint8_t *) MemoryAlloc(certLen);
    VerifyOrExit(certBuf != NULL, error = CHIP_ERROR_NO_MEMORY);

    // Read the certificate
    error = ConfigurationMgr().GetDeviceIntermediateCACerts(certBuf, certLen, certLen);
    SuccessOrExit(error);

    streamer_print_hex(sout, const_cast<const uint8_t *>(certBuf), certLen);

exit:
    if (certBuf != NULL)
    {
        MemoryFree(certBuf);
    }
    return ConfigGetDone(sout, error);
}

static CHIP_ERROR ConfigGetManufacturerDeviceId(bool printHeader)
{
    CHIP_ERROR error  = CHIP_NO_ERROR;
    streamer_t * sout = streamer_get();
    uint64_t value64;

    if (printHeader)
    {
        streamer_printf(sout, "MfrDeviceId:     ");
    }
    SuccessOrExit(error = ConfigurationMgr().GetManufacturerDeviceId(value64));
    streamer_print_hex(sout, reinterpret_cast<const uint8_t *>(&value64), sizeof(value64));

exit:
    return ConfigGetDone(sout, error);
}

static CHIP_ERROR ConfigGetManufacturerDeviceCert(bool printHeader)
{
    CHIP_ERROR error  = CHIP_NO_ERROR;
    streamer_t * sout = streamer_get();
    uint8_t * certBuf = NULL;
    size_t certLen;

    if (printHeader)
    {
        streamer_printf(sout, "MfrDeviceCert:   ");
    }
    // Determine the length of the device certificate.
    error = ConfigurationMgr().GetManufacturerDeviceCertificate((uint8_t *) NULL, 0, certLen);
    SuccessOrExit(error);

    // Fail if no certificate has been configured.
    VerifyOrExit(certLen != 0, error = CHIP_ERROR_CERT_NOT_FOUND);

    // Create a temporary buffer to hold the certificate.
    certBuf = (uint8_t *) MemoryAlloc(certLen);
    VerifyOrExit(certBuf != NULL, error = CHIP_ERROR_NO_MEMORY);

    // Read the certificate
    error = ConfigurationMgr().GetManufacturerDeviceCertificate(certBuf, certLen, certLen);
    SuccessOrExit(error);

    streamer_print_hex(sout, const_cast<const uint8_t *>(certBuf), certLen);

exit:
    if (certBuf != NULL)
    {
        MemoryFree(certBuf);
    }
    return ConfigGetDone(sout, error);
}

static CHIP_ERROR ConfigGetManufacturerDeviceCaCerts(bool printHeader)
{
    CHIP_ERROR error  = CHIP_NO_ERROR;
    streamer_t * sout = streamer_get();
    uint8_t * certBuf = NULL;
    size_t certLen;

    if (printHeader)
    {
        streamer_printf(sout, "MfgDeviceCaCerts:");
    }
    // Determine the length of the device certificate.
    error = ConfigurationMgr().GetManufacturerDeviceIntermediateCACerts((uint8_t *) NULL, 0, certLen);
    SuccessOrExit(error);

    // Fail if no certificate has been configured.
    VerifyOrExit(certLen != 0, error = CHIP_ERROR_CERT_NOT_FOUND);

    // Create a temporary buffer to hold the certificate.
    certBuf = (uint8_t *) MemoryAlloc(certLen);
    VerifyOrExit(certBuf != NULL, error = CHIP_ERROR_NO_MEMORY);

    // Read the certificate
    error = ConfigurationMgr().GetManufacturerDeviceIntermediateCACerts(certBuf, certLen, certLen);
    SuccessOrExit(error);

    streamer_print_hex(sout, const_cast<const uint8_t *>(certBuf), certLen);

exit:
    if (certBuf != NULL)
    {
        MemoryFree(certBuf);
    }
    return ConfigGetDone(sout, error);
}

static CHIP_ERROR ConfigGetPairingCode(bool printHeader)
{
    CHIP_ERROR error  = CHIP_NO_ERROR;
    streamer_t * sout = streamer_get();
    char buf[ConfigurationManager::kMaxPairingCodeLength];
    size_t bufSize;

    if (printHeader)
    {
        streamer_printf(sout, "PairingCode:     ");
    }
    SuccessOrExit(error = ConfigurationMgr().GetPairingCode(buf, sizeof(buf), bufSize));
    streamer_print_hex(sout, reinterpret_cast<const uint8_t *>(buf), bufSize);

exit:
    return ConfigGetDone(sout, error);
}

static CHIP_ERROR ConfigGetServiceId(bool printHeader)
{
    CHIP_ERROR error  = CHIP_NO_ERROR;
    streamer_t * sout = streamer_get();
    uint64_t value64;

    if (printHeader)
    {
        streamer_printf(sout, "ServiceId:       ");
    }
    SuccessOrExit(error = ConfigurationMgr().GetServiceId(value64));
    streamer_print_hex(sout, reinterpret_cast<const uint8_t *>(&value64), sizeof(value64));

exit:
    return ConfigGetDone(sout, error);
}

static CHIP_ERROR ConfigGetFabricId(bool printHeader)
{
    CHIP_ERROR error  = CHIP_NO_ERROR;
    streamer_t * sout = streamer_get();
    uint64_t value64;

    if (printHeader)
    {
        streamer_printf(sout, "FabricId:        ");
    }
    SuccessOrExit(error = ConfigurationMgr().GetFabricId(value64));
    streamer_print_hex(sout, reinterpret_cast<const uint8_t *>(&value64), sizeof(value64));

exit:
    return ConfigGetDone(sout, error);
}

int cmd_device_config(int argc, char ** argv)
{
    CHIP_ERROR error = CHIP_NO_ERROR;

    VerifyOrExit(argc == 0, error = CHIP_ERROR_INVALID_ARGUMENT);

    error |= ConfigGetVendorId(true);
    error |= ConfigGetProductId(true);
    error |= ConfigGetProductRevision(true);
    error |= ConfigGetSerialNumber(true);

    error |= ConfigGetServiceId(true);
    error |= ConfigGetFabricId(true);
    error |= ConfigGetPairingCode(true);

    error |= ConfigGetDeviceId(true);
    error |= ConfigGetDeviceCert(true);
    error |= ConfigGetDeviceCaCerts(true);

    error |= ConfigGetManufacturerDeviceId(true);
    error |= ConfigGetManufacturerDeviceCert(true);
    error |= ConfigGetManufacturerDeviceCaCerts(true);

exit:
    return (error) ? CHIP_DEVICE_ERROR_CONFIG_NOT_FOUND : CHIP_NO_ERROR;
}

int cmd_device_get(int argc, char ** argv)
{
    CHIP_ERROR error  = CHIP_NO_ERROR;
    streamer_t * sout = streamer_get();

    if (argc == 0)
    {
        return cmd_device_config(argc, argv);
    }

    if (strcmp(argv[0], "vendorid") == 0)
    {
        SuccessOrExit(error = ConfigGetVendorId(false));
    }
    else if (strcmp(argv[0], "productid") == 0)
    {
        SuccessOrExit(error = ConfigGetProductId(false));
    }
    else if (strcmp(argv[0], "productrev") == 0)
    {
        SuccessOrExit(error = ConfigGetProductRevision(false));
    }
    else if (strcmp(argv[0], "serial") == 0)
    {
        SuccessOrExit(error = ConfigGetSerialNumber(false));
    }
    else if (strcmp(argv[0], "deviceid") == 0)
    {
        SuccessOrExit(error = ConfigGetDeviceId(false));
    }
    else if (strcmp(argv[0], "cert") == 0)
    {
        SuccessOrExit(error = ConfigGetDeviceCert(false));
    }
    else if (strcmp(argv[0], "cacerts") == 0)
    {
        SuccessOrExit(error = ConfigGetDeviceCaCerts(false));
    }
    else if (strcmp(argv[0], "mfrdeviceid") == 0)
    {
        SuccessOrExit(error = ConfigGetManufacturerDeviceId(false));
    }
    else if (strcmp(argv[0], "mfrcert") == 0)
    {
        SuccessOrExit(error = ConfigGetManufacturerDeviceCert(false));
    }
    else if (strcmp(argv[0], "mfrcacerts") == 0)
    {
        SuccessOrExit(error = ConfigGetManufacturerDeviceCaCerts(false));
    }
    else if (strcmp(argv[0], "pairingcode") == 0)
    {
        SuccessOrExit(error = ConfigGetPairingCode(false));
    }
    else if (strcmp(argv[0], "serviceid") == 0)
    {
        SuccessOrExit(error = ConfigGetServiceId(false));
    }
    else if (strcmp(argv[0], "fabricid") == 0)
    {
        SuccessOrExit(error = ConfigGetFabricId(false));
    }
    else
    {
        ExitNow(error = CHIP_ERROR_INVALID_ARGUMENT);
    }

exit:
    return error;
}

int cmd_device_dispatch(int argc, char ** argv)
{
    CHIP_ERROR error = CHIP_NO_ERROR;

    VerifyOrExit(argc > 0, error = CHIP_ERROR_INVALID_ARGUMENT);

    error = sShellDeviceSubcommands.ExecCommand(argc, argv);

exit:
    return error;
}

static const shell_command_t cmds_base64_root = { &cmd_device_dispatch, "device", "Device Layer commands" };

/// Subcommands for root command: `base64 <subcommand>`
static const shell_command_t cmds_device[] = {
    { &cmd_device_help, "help", "Usage: device <subcommand>" },
    { &cmd_device_start, "start", "Start the device layer. Usage: device start" },
    { &cmd_device_get, "get", "Get configuration value. Usage: device get <param_name>" },
    { &cmd_device_config, "config", "Dump entire configuration of device. Usage: device dump" },
};

#endif // CONFIG_DEVICE_LAYER

void cmd_device_init(void)
{
#if CONFIG_DEVICE_LAYER
    // Register `device` subcommands with the local shell dispatcher.
    sShellDeviceSubcommands.RegisterCommands(cmds_device, ARRAY_SIZE(cmds_device));

    // Register the root `base64` command with the top-level shell.
    shell_register(&cmds_base64_root, 1);
#endif // CONFIG_DEVICE_LAYER
}
