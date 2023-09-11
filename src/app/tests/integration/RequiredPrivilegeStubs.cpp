/*
 *
 * SPDX-FileCopyrightText: 2022 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <app/util/privilege-storage.h>

int MatterGetAccessPrivilegeForReadAttribute(chip::ClusterId cluster, chip::AttributeId attribute)
{
    return kMatterAccessPrivilegeAdminister;
}

int MatterGetAccessPrivilegeForWriteAttribute(chip::ClusterId cluster, chip::AttributeId attribute)
{
    return kMatterAccessPrivilegeAdminister;
}

int MatterGetAccessPrivilegeForInvokeCommand(chip::ClusterId cluster, chip::CommandId command)
{
    return kMatterAccessPrivilegeAdminister;
}

int MatterGetAccessPrivilegeForReadEvent(chip::ClusterId cluster, chip::EventId event)
{
    return kMatterAccessPrivilegeAdminister;
}
