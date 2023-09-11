/**
 * SPDX-FileCopyrightText: 2023 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

/**
 * Declarations of function that can be used to query the endpoint configuration
 * for the device.
 */

#include <app/util/af-types.h>

/**
 * Returns the total number of possible endpoints (dynamic and pre-compiled).
 * Not all those endpoints might be enabled, and the dynamic ones might not even
 * have an EmberAfEndpointType defined.
 *
 * Typically only used for endpoint index iteration.
 */
uint16_t emberAfEndpointCount(void);

/**
 * @brief Enable/disable endpoints
 */
bool emberAfEndpointEnableDisable(chip::EndpointId endpoint, bool enable);

/**
 * Returns whether the endpoint at the specified index (which must be less than
 * emberAfEndpointCount() is enabled.  If an endpoint is disabled, it is not
 * guaranteed to have an EmberAfEndpointType.
 */
bool emberAfEndpointIndexIsEnabled(uint16_t index);

/**
 * Returns the endpoint id of the endpoint at the given index.  Will return
 * kInvalidEndpointId for endpoints that are not actually configured.
 */
chip::EndpointId emberAfEndpointFromIndex(uint16_t index);

/**
 * Returns the endpoint descriptor for the given endpoint id if there is an
 * enabled endpoint with that endpoint id.  Otherwise returns null.
 */
const EmberAfEndpointType * emberAfFindEndpointType(chip::EndpointId endpointId);

/**
 * Returns the cluster descriptor for the given cluster on the given endpoint.
 *
 * If the given endpoint does not exist or is disabled, returns null.
 *
 * If the given endpoint does not have the given cluster, returns null.
 */
const EmberAfCluster * emberAfFindServerCluster(chip::EndpointId endpoint, chip::ClusterId clusterId);

/**
 * Returns true if the given endpoint exists, is enabled, has the given cluster,
 * and that cluster has the given attribute.
 */
bool emberAfContainsAttribute(chip::EndpointId endpoint, chip::ClusterId clusterId, chip::AttributeId attributeId);
