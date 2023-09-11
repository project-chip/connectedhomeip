/**
 *
 * SPDX-FileCopyrightText: 2020 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <app/util/basic-types.h>
#include <lib/support/Span.h>

/** @brief Groups Cluster Endpoint In Group
 *
 * This function is called by the framework when it needs to determine if an
 * endpoint is a member of a group.  The application should return true if the
 * endpoint is a member of the group and false otherwise.
 *
 * @param endpoint The endpoint.  Ver.: always
 * @param groupId The group identifier.  Ver.: always
 */
bool emberAfGroupsClusterEndpointInGroupCallback(chip::FabricIndex fabricIndex, chip::EndpointId endpoint, chip::GroupId groupId);
