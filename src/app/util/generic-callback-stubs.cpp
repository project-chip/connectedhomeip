/*
 *
 * SPDX-FileCopyrightText: 2022 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <app-common/zap-generated/callback.h>
#include <app/util/basic-types.h>
#include <lib/support/Span.h>
#include <protocols/interaction_model/Constants.h>

using namespace chip;

// This file contains overridable callbacks that are not cluster specific.

EmberAfAttributeWritePermission __attribute__((weak))
emberAfAllowNetworkWriteAttributeCallback(EndpointId endpoint, ClusterId clusterId, AttributeId attributeId, uint8_t * value,
                                          uint8_t type)
{
    return EmberAfAttributeWritePermission::AllowWriteNormal; // Default
}

bool __attribute__((weak)) emberAfAttributeReadAccessCallback(EndpointId endpoint, ClusterId clusterId, AttributeId attributeId)
{
    return true;
}

bool __attribute__((weak)) emberAfAttributeWriteAccessCallback(EndpointId endpoint, ClusterId clusterId, AttributeId attributeId)
{
    return true;
}

EmberAfStatus __attribute__((weak))
emberAfExternalAttributeReadCallback(EndpointId endpoint, ClusterId clusterId, const EmberAfAttributeMetadata * attributeMetadata,
                                     uint8_t * buffer, uint16_t maxReadLength)
{
    return EMBER_ZCL_STATUS_FAILURE;
}

EmberAfStatus __attribute__((weak))
emberAfExternalAttributeWriteCallback(EndpointId endpoint, ClusterId clusterId, const EmberAfAttributeMetadata * attributeMetadata,
                                      uint8_t * buffer)
{
    return EMBER_ZCL_STATUS_FAILURE;
}

chip::Protocols::InteractionModel::Status __attribute__((weak))
MatterPreAttributeChangeCallback(const chip::app::ConcreteAttributePath & attributePath, uint8_t type, uint16_t size,
                                 uint8_t * value)
{
    return chip::Protocols::InteractionModel::Status::Success;
}

void __attribute__((weak)) MatterPostAttributeChangeCallback(const chip::app::ConcreteAttributePath & attributePath, uint8_t type,
                                                             uint16_t size, uint8_t * value)
{}
