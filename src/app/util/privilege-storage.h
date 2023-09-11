/**
 *
 * SPDX-FileCopyrightText: 2022 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#pragma once

#include <lib/core/DataModelTypes.h>

constexpr int kMatterAccessPrivilegeView       = 0;
constexpr int kMatterAccessPrivilegeOperate    = 1;
constexpr int kMatterAccessPrivilegeManage     = 2;
constexpr int kMatterAccessPrivilegeAdminister = 3;
constexpr int kMatterAccessPrivilegeMaxValue   = kMatterAccessPrivilegeAdminister;

int MatterGetAccessPrivilegeForReadAttribute(chip::ClusterId cluster, chip::AttributeId attribute);
int MatterGetAccessPrivilegeForWriteAttribute(chip::ClusterId cluster, chip::AttributeId attribute);
int MatterGetAccessPrivilegeForInvokeCommand(chip::ClusterId cluster, chip::CommandId command);
int MatterGetAccessPrivilegeForReadEvent(chip::ClusterId cluster, chip::EventId event);
