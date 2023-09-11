/*
 *
 * SPDX-FileCopyrightText: 2022 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

std::vector<EndpointListInfo> GetEndpointListInfo(chip::EndpointId parentId);

std::vector<Action *> GetActionListInfo(chip::EndpointId parentId);
