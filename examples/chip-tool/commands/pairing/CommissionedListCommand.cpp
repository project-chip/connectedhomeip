/*
 * SPDX-FileCopyrightText: (c) 2021 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "CommissionedListCommand.h"

#include <lib/core/CHIPEncoding.h>
#include <lib/support/PersistentStorageMacros.h>
#include <lib/support/SafeInt.h>
#include <lib/support/Span.h>

CHIP_ERROR CommissionedListCommand::Run()
{
    ReturnLogErrorOnFailure(mStorage.Init());
    return PrintInformation();
}

CHIP_ERROR CommissionedListCommand::PrintInformation()
{
    uint64_t pairedNodesIds[chip::Controller::kNumMaxPairedDevices];
    uint16_t pairedNodesIdsSize = sizeof(pairedNodesIds);
    memset(pairedNodesIds, 0, pairedNodesIdsSize);

    // TODO: Get the list of paired node IDs.  chip-tool needs to store that as
    // devices get paired.
    uint16_t pairedDevicesCount = 0;
    while (pairedNodesIds[pairedDevicesCount] != 0x0 && pairedDevicesCount < chip::Controller::kNumMaxPairedDevices)
    {
        pairedDevicesCount++;
    }

    if (pairedDevicesCount == 0)
    {
        ChipLogProgress(chipTool, "No paired devices.");
    }
    else
    {
        fprintf(stdout, "NOTES: Only the devices locally commissioned with chip-tool are displayed.\n");
        fprintf(stdout, "+---------------------------------------------------------------------------------------------+\n");
        fprintf(stdout, "|       NodeId       |                   Address                     | Port |    Interface    |\n");
        fprintf(stdout, "+---------------------------------------------------------------------------------------------+\n");
        for (uint16_t i = 0; i < pairedDevicesCount; i++)
        {
            ReturnLogErrorOnFailure(PrintDeviceInformation(pairedNodesIds[i]));
        }
        fprintf(stdout, "+---------------------------------------------------------------------------------------------+\n");
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR CommissionedListCommand::PrintDeviceInformation(chip::NodeId deviceId)
{
    // TODO: Controller::SerializedDevice and Controller::SerializableDevice are
    // gone.  Need to figure out what chip-tool should actually store/retrieve
    // here.
    chip::Controller::SerializedDevice deviceInfo;
    uint16_t size = sizeof(deviceInfo.inner);

    chip::Controller::SerializableDevice serializable;
    constexpr size_t maxlen = BASE64_ENCODED_LEN(sizeof(serializable));
    const size_t len        = strnlen(chip::Uint8::to_const_char(&deviceInfo.inner[0]), maxlen);

    VerifyOrReturnError(len < sizeof(chip::Controller::SerializedDevice), CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(chip::CanCastTo<uint16_t>(len), CHIP_ERROR_INVALID_ARGUMENT);

    CHIP_ZERO_AT(serializable);
    const uint16_t deserializedLen = chip::Base64Decode(chip::Uint8::to_const_char(deviceInfo.inner), static_cast<uint16_t>(len),
                                                        chip::Uint8::to_uchar(reinterpret_cast<uint8_t *>(&serializable)));

    VerifyOrReturnError(deserializedLen > 0, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(deserializedLen <= sizeof(serializable), CHIP_ERROR_INVALID_ARGUMENT);

    const uint16_t port = chip::Encoding::LittleEndian::HostSwap16(serializable.mDevicePort);
    fprintf(stderr, "| 0x%-16" PRIx64 " | %-45s | %-5u| %-15s |\n", deviceId, serializable.mDeviceAddr, port,
            serializable.mInterfaceName);

    return CHIP_NO_ERROR;
}
