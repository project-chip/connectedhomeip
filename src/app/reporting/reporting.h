/**
 *
 * SPDX-FileCopyrightText: 2020 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <app/ConcreteAttributePath.h>

/** @brief Reporting Attribute Change
 *
 * This function is called by the framework when an attribute managed by the
 * framework changes.  The application should call this function when an
 * externally-managed attribute changes.  This function triggers attribute
 * reports for subscriptions as needed.
 */
void MatterReportingAttributeChangeCallback(chip::EndpointId endpoint, chip::ClusterId clusterId, chip::AttributeId attributeId);

/*
 * Same but with a nicer attribute path.
 */
void MatterReportingAttributeChangeCallback(const chip::app::ConcreteAttributePath & aPath);

/*
 * Same but only with an EndpointId, this is used when adding / enabling an endpoint during runtime.
 */
void MatterReportingAttributeChangeCallback(chip::EndpointId endpoint);
